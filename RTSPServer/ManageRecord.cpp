#include "ManageRecord.h"
#include "PreAllocateDisk.h"
#include "MediaServerDLL.h"
#include <boost/lexical_cast.hpp>

extern MediaSave::CManageRecord* g_pManageRecord;

namespace MediaSave
{

CManageRecord::CManageRecord(CPreAllocateDisk *pall_disk)
	:m_pre_allocate_disk(pall_disk),
	m_brun(false)
{
}

CManageRecord::~CManageRecord(void)
{
}

int CManageRecord::InitManageRecord(CDataInfo* pDataInfo)
{
	m_pDataInfo = pDataInfo;

	std::vector<std::string> CameraURLArray;
	int iRet = m_pDataInfo->GetRecordCameraURLList( CameraURLArray );
	//if( iRet < 0 )return iRet;
	//RECORD_MANUAL

	char configValue[256]; memset(configValue, 0, 256);
	int i = m_pDataInfo->GetConfig("MediaSaveByFile", configValue);
	if( i >= 0 && strcmp(configValue,"on") == 0 )
	{
		for ( std::vector<std::string>::size_type i = 0; i < CameraURLArray.size(); i++ )
		{
			AddManualCameraRecord((char*) CameraURLArray[i].c_str(), 2);
		}
	}
	else
	{
		for ( std::vector<std::string>::size_type i = 0; i < CameraURLArray.size(); i++ )
		{
			CameraRecordPtr camera_record_ptr( new CCameraRecord(this) );
			camera_record_ptr->InitCameraRecordByURL( (char*) CameraURLArray[i].c_str() );
			m_camera_recode_array.push_back( camera_record_ptr );
		}
	}

	return 0;
}

int CManageRecord::AddManualCameraRecord( char* url, int recordType )
{
	CameraRecordPtr camera_record_ptr(new CCameraRecord(this, recordType));
	int iRet = camera_record_ptr->InitCameraRecordByURL( url );
	if( iRet < 0 ) return iRet;

	CameraRecordPtr have_Camera_ptr = GetCameraRecordPtr( camera_record_ptr->GetCameraID() );
	if( have_Camera_ptr != NULL ){
		have_Camera_ptr->ResetStartRecordTime();
		if( !have_Camera_ptr->GetCameraRecordIsOpen() )
			return -1;
		return 1;
	} else {
		int iret = camera_record_ptr->StartRecord();
		//if( iret < 0 ) return -1;
		boost::unique_lock<boost::mutex> singleLock(m_ManualCamArrayMutex);
		m_ManualCamReclist.push_back( camera_record_ptr );
		if( iret >= 0 ) return 2;
		return iret;
	}
	return -1;
}

void myRequestHandleFun(std::string strRequstCmd, std::string& strReponse)
{
	//http://127.0.0.1:8080/StartRecordCamera?rtsp://127.0.0.1:554/110102001&ByManual
	//http://127.0.0.1:8080/StartRecordCamera?rtsp://127.0.0.1:554/110102001&ByUser

	strReponse = "SUCCESS";
	std::string strRealCmd = strRequstCmd.substr(0, strRequstCmd.find_first_of('?'));
	std::string strCameraURL = strRequstCmd.substr(strRequstCmd.find_first_of('?') + 1,
		strRequstCmd.find_first_of('&') - strRealCmd.length() - 1);
	std::string strParam1 = strRequstCmd.substr(strRequstCmd.find_first_of('&') + 1,
		strRequstCmd.length() - strRequstCmd.find_first_of('&'));

	if( strRealCmd.compare("StartRecordCamera") != 0  )
	{
		strReponse = "FAILED";
		if( strRealCmd.compare("StopRecordCamera") == 0  ){
			CManageRecord::CameraRecordPtr camera_record_ptr(new CCameraRecord(g_pManageRecord, CCameraRecord::RECORD_MANUAL));
			int iRet = camera_record_ptr->InitCameraRecordByURL( (char*)strCameraURL.c_str(), false );
			if( iRet < 0 ) return;
			CManageRecord::CameraRecordPtr have_Camera_ptr =
				g_pManageRecord->GetCameraRecordPtr( camera_record_ptr->GetCameraID() );
			if( have_Camera_ptr != NULL ){
				have_Camera_ptr->SetIsBadSocket( true );
				have_Camera_ptr->StopRecord();
				int iEventID = have_Camera_ptr->GetEventRecordID();
				strReponse = "SUCCESS#" + boost::lexical_cast<std::string>(iEventID);
				return;
			}
		}
		g_pManageRecord->GetPreAllocateDisk()->m_log.Add( "1RequestHTTP RequstCmd = %s  ;FAILED", strRequstCmd.c_str() );
		return;
	}

	int recordType = CCameraRecord::RECORD_MANUAL;
	if( strParam1.compare("ByUser") == 0 )
		recordType = CCameraRecord::RECORD_USER;
	if( g_pManageRecord->AddManualCameraRecord( (char*)strCameraURL.c_str(), recordType ) < 0 ){
		strReponse = "FAILED";
		g_pManageRecord->GetPreAllocateDisk()->m_log.Add( "2RequestHTTP RequstCmd = %s  ;FAILED", strRequstCmd.c_str() );
		return;
	}
	g_pManageRecord->GetPreAllocateDisk()->m_log.Add( "3RequestHTTP RequstCmd = %s  ;SUCCESS", strRequstCmd.c_str() );
}

int CManageRecord::StartRecordThread(int iManualPort)
{
	m_brun = true;
	for (std::vector< CameraRecordPtr >::size_type i = 0; i < m_camera_recode_array.size(); i += CAMERA_COUNT_PRE_THREAD)
	{
		boost::shared_ptr<boost::thread> thread_ptr(new boost::thread(
			boost::bind(&CManageRecord::ManageRecordThread, this, i)));
		m_threadptr_array.push_back(thread_ptr);
		boost::this_thread::sleep( boost::posix_time::milliseconds( 500 ) ); //错开 线程打开摄像机
	}

	if( iManualPort > 0 ) {
		m_httpServerPtr = (boost::shared_ptr<http_wl::server3::server>) 
			new http_wl::server3::server( boost::lexical_cast<std::string>(iManualPort), myRequestHandleFun );
		m_httpServerPtr->Start();
		m_ManualThreadPtr = boost::shared_ptr<boost::thread>( new boost::thread(
			boost::bind(&CManageRecord::ManageManualRecordThread, this)) );
	}
	return 0;
}

void CManageRecord::ManageRecordThread(int ibegin)
{
	int icount = 0;
	for (std::vector< CameraRecordPtr >::size_type i = ibegin; i < m_camera_recode_array.size(); i++ )
	{
		if( IsNeedToSave( m_camera_recode_array[i]->GetCameraID() ) )
		{
			if( m_camera_recode_array[i]->StartRecord() < 0 )
			{
//				PutCameraStateUpdate(m_camera_recode_array[i]->GetCameraID(), 3);
			}
			else
			{
//				PutCameraStateUpdate(m_camera_recode_array[i]->GetCameraID(), 1);
			}
			boost::this_thread::sleep( boost::posix_time::seconds( 1 ) );////错开 打开摄像机 避免竞争
		}
		if( ++icount >= CAMERA_COUNT_PRE_THREAD )break;
	}

	while( m_brun )
	{
		icount = 0;
		for ( std::vector<CameraRecordPtr>::size_type i = ibegin; i < m_camera_recode_array.size(); i++ )
		{
			if( IsNeedToSave( m_camera_recode_array[i]->GetCameraID() ) )
			{
				int bt_time = (int) difftime(time(NULL), m_camera_recode_array[i]->GetCurRecordTime());
				if(!m_camera_recode_array[i]->GetCameraRecordIsOpen() || bt_time > RECORD_TIMEOUT_MAX_SECONDS )
				{
					if( m_camera_recode_array[i]->StartRecord() < 0)
					{
//						PutCameraStateUpdate(m_camera_recode_array[i]->GetCameraID(), 3);
					}
					else
					{
//						PutCameraStateUpdate(m_camera_recode_array[i]->GetCameraID(), 1);
					}
					boost::this_thread::sleep( boost::posix_time::seconds( 2 ) );
					m_pre_allocate_disk->m_log.Add( "CManageRecord::ManageRecordThread ReOpen CameraID = %d; bt_time = %d",
						m_camera_recode_array[i]->GetCameraID(), bt_time );
				}	
			}
			else
			{
				if( m_camera_recode_array[i]->GetCameraRecordIsOpen() )
					m_camera_recode_array[i]->StopRecord();
			}

			//云台自动恢复功能
			if(m_pre_allocate_disk->GetIsPTZAutoBack() && m_camera_recode_array[i]->GetCameraRecordIsOpen() )
			{
				int iMinute = 30, presetNum = 5;
				if( m_pDataInfo->GetPTZBackInfoByCameraID(m_camera_recode_array[i]->GetCameraID(), iMinute, presetNum) >= 0 )
				{
					int bt_time = (int) difftime(time(NULL), m_camera_recode_array[i]->GetLastPtzAutoBackTime());
					if( bt_time > iMinute * 60 ){
						m_camera_recode_array[i]->CameraPTZCtrl(16, presetNum, 0);
						m_camera_recode_array[i]->SetLastPtzAutoBackTime(time(NULL));
					}
				}
			}

			if( ++icount >= CAMERA_COUNT_PRE_THREAD || !m_brun )break;
		}

		//等待十秒后再次检测
		int iexit_second = 0;
		while( iexit_second++ < CAMERA_COUNT_PRE_THREAD /** CAMERA_COUNT_PRE_THREAD / 2*/ ){
			boost::this_thread::sleep( boost::posix_time::seconds( 1 ) );
			if( !m_brun )return;
		}
	}
}

void CManageRecord::ManageManualRecordThread()
{
	while( m_brun )
	{
		CameraRecordPtr cameraRecordPtr;
		{
			boost::unique_lock<boost::mutex> singleLock(m_ManualCamArrayMutex);
			std::list<CameraRecordPtr>::iterator it = m_ManualCamReclist.begin();
			for ( ; it != m_ManualCamReclist.end(); it++ )
			{
				int bt_time = (int) difftime(time(NULL), (*it)->GetCurRecordTime());
				int manual_record_bt_time = (int) difftime(time(NULL), (*it)->GetStartRecordTime());

				if( ( (*it)->GetRecordType() <= CCameraRecord::RECORD_USER && (*it)->GetIsBadSocket() )
					|| ( (*it)->GetRecordType() == CCameraRecord::RECORD_MANUAL && manual_record_bt_time > MANUAL_RECORD_TIME*10 ) )
				{
					if( (*it)->GetRecordType() == CCameraRecord::RECORD_MANUAL )
						g_pManageRecord->GetPreAllocateDisk()->m_log.Add( "RequestHTTP ByManual StopRecord CameraID = %d", (*it)->GetCameraID() );
					else
						g_pManageRecord->GetPreAllocateDisk()->m_log.Add( "RequestHTTP ByUser StopRecord CameraID = %d", (*it)->GetCameraID() );

					(*it)->StopRecord();
					m_ManualCamReclist.erase(it);
					break;
				}

				if( (*it)->GetRecordType() <= CCameraRecord::RECORD_USER && bt_time > RECORD_TIMEOUT_MAX_SECONDS )
					cameraRecordPtr = (*it);
			}
		}//lock end

		if( cameraRecordPtr != NULL )
		{
			if( cameraRecordPtr->StartRecord() < 0 ) 
			{
//				PutCameraStateUpdate(cameraRecordPtr->GetCameraID(), 3);
			}
			else
			{
//				PutCameraStateUpdate(cameraRecordPtr->GetCameraID(), 1);
			}
		}

		int iexit_second = 0;
		while( iexit_second++ < CAMERA_COUNT_PRE_THREAD /** CAMERA_COUNT_PRE_THREAD / 2*/ ){
			boost::this_thread::sleep( boost::posix_time::seconds( 1 ) );
			if( !m_brun )return;
		}
	}
}

void CManageRecord::StopRecordThread()
{
	m_brun = false;
	for ( std::vector<CameraRecordPtr>::size_type i = 0; i < m_threadptr_array.size(); i++ )
	{
		m_threadptr_array[i]->join();
	}
	for ( std::vector<CameraRecordPtr>::size_type i = 0; i < m_camera_recode_array.size(); i++)
	{
		m_camera_recode_array[i]->StopRecord();
	}
	if( m_httpServerPtr != NULL )
	{
		m_httpServerPtr->Stop();
		m_ManualThreadPtr->join();
		boost::unique_lock<boost::mutex> singleLock(m_ManualCamArrayMutex);
		std::list<CameraRecordPtr>::iterator it = m_ManualCamReclist.begin();
		for ( ; it != m_ManualCamReclist.end(); it++ ) {
			(*it)->StopRecord();
		}
	}
}

bool CManageRecord::IsNeedToSave(int CameraID)
{
	return m_pDataInfo->GetIsNeedToRecordCamera( CameraID );
	//return true;
}

CManageRecord::CameraRecordPtr CManageRecord::GetCameraRecordPtr(int CameraID)
{
	CameraRecordPtr camera_record_ptr;
	for ( std::vector<CameraRecordPtr>::size_type i = 0; i < m_camera_recode_array.size(); i++ )
	{
		if( m_camera_recode_array[i]->GetCameraID() == CameraID )
		{
			camera_record_ptr = m_camera_recode_array[i];
			return camera_record_ptr;
		}
	}

	boost::unique_lock<boost::mutex> singleLock( m_ManualCamArrayMutex );
	std::list<CameraRecordPtr>::iterator it = m_ManualCamReclist.begin();
	for ( ; it != m_ManualCamReclist.end(); it++ ) {
		if( (*it)->GetCameraID() == CameraID ) {
			camera_record_ptr = *it;
			return camera_record_ptr;
		}
	}
	return camera_record_ptr;
}

}// end namespace
