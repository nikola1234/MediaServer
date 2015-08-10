#include "CameraRecord.h"
#include "RAW_RTSPClient.h"
#include "ManageRecord.h"
#include "PreAllocateDisk.h"

#define _SCL_SECURE_NO_WARNINGS

#include <fstream>
#include <boost/filesystem.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
namespace fs = boost::filesystem;

namespace MediaSave
{

CCameraRecord::CCameraRecord( CManageRecord* pmanagerecord, int recordType )
	:m_prtsp_client(NULL),
	m_cameraid(0),
	m_cur_record_time(time(NULL)),
	m_manage_record(pmanagerecord),
	m_file_write(pmanagerecord->GetPreAllocateDisk()),
	m_filestate(false),
	m_port(0),
	m_isopen_camera(false),
	m_lastPtzAutoBackTime(time(NULL)),
	m_CameraRecordType( recordType ),
	m_startRecordTime(time(NULL)),
	m_isBadSocket(false),
	m_startManualFileTime(0),
	m_curFileIndexTime_t(0),
	m_isHaveHeadData(false),
	m_headLen(0),
	m_EventRecordID(0/*-1*/)
{
	memset(m_ip, 0, 256);
	memset(m_url, 0, 256);
	memset(m_userName, 0, 256);
	memset(m_passWord, 0, 256);

	memset(m_chMRURL, 0, 256);
	memset(m_headData, 0, HEAD_SIZE);
	if( CRTSPClient::s_UpdateCameraNameCallBack == NULL ) {
		CRTSPClient::s_UpdateCameraNameCallBack = CCameraRecord::SVideoSwitchCameraUpdateCallback;
		CRTSPClient::s_UpdateCameraCallBackUser = (unsigned long)m_manage_record;
	}

	int iRet = m_manage_record->m_pDataInfo->GetNewEventID(m_EventRecordID);

#ifndef OLD_MANUAL_SAVA
	memset(m_manualSavePath, 0, 256);
	memset(m_manualFileIndexPath, 0, 256);
#endif
}

int CCameraRecord::SwitchCameraByCameraID(int iNewCamera)
{
	m_cameraid = iNewCamera;
	sprintf( m_url, "rtsp://%s:%d/%d", m_ip, m_port, m_cameraid );
	m_file_write.SwitchCamera( iNewCamera );

	CreateNewManualSaveFilePath();

	return 0;
}

int CCameraRecord::InitCameraRecordByURL(char* url, bool bRealInit)
{
	char* username;
	char* password;
	char* address;
	char* urlSuffix;
	int portNum;

	int iret = 0;
	if(CRTSPClient::parseRTSPURL( url, username, password, address, portNum, urlSuffix ))
	{
		if(address != NULL && urlSuffix!= NULL &&
			strlen(address) < 256 && atoi( urlSuffix ) > 0 )
		{
			iret = 0;
			if(address != NULL)
				strcpy(m_ip, address);
			if(username != NULL)
				strcpy(m_userName, username);
			if(password != NULL)
				strcpy(m_passWord, password);

			m_port = portNum;
			m_cameraid = atoi( urlSuffix );

			sprintf( m_url, "rtsp://%s:%d/%s", m_ip, m_port, urlSuffix );
		} else {  iret = -1; }
	} else  {  iret = -1; }

	delete[] username; username = NULL;
	delete[] password; password = NULL;
	delete[] address; address = NULL;
	delete[] urlSuffix; urlSuffix = NULL;

	char ip[64]; memset(ip, 0, 64);
	m_manage_record->m_pDataInfo->GetConfig( "MSIP", ip );
	char port[256]; memset(port, 0, 256);
	m_manage_record->m_pDataInfo->GetConfig("PORT_TOClient", port);
	memset(this->m_chMRURL, 0, 256);
	sprintf( m_chMRURL, "rtsp://%s:%s/", ip, port );

	if ( iret >= 0 && bRealInit){
		m_filestate = m_file_write.InitFileWrite( m_cameraid );
		CreateNewManualSaveFilePath();
	}
	return iret;
}

CCameraRecord::~CCameraRecord(void)
{
	StopRecord();
}

int CCameraRecord::StartRecord()
{
	int nret = -1;

	StopRecord();
	int tokenType = 2;
	if( m_CameraRecordType == RECORD_USER )
		tokenType = 4;
	m_prtsp_client = new CRAW_RTSPClient(this, tokenType);
	m_prtsp_client->setUsernameAndPassword(m_userName, m_passWord);
	m_prtsp_client->setUserAgentByToken();
	nret = m_prtsp_client->startRTSPRequest(m_ip, m_port, m_url);
	m_manage_record->GetPreAllocateDisk()->m_log.Add("CCameraRecord::StartRecord IP=%s Port=%d url=%s nret = %d",
		m_ip, m_port, m_url, nret);
	if ( nret < 0 )
		m_prtsp_client->stopRTSPRequest();
	else
		m_isopen_camera = true;

	return nret;
}

void CCameraRecord::StopRecord()
{
	if (m_prtsp_client != NULL)
	{
		m_prtsp_client->stopRTSPRequest();
		delete m_prtsp_client;
		m_prtsp_client = NULL;
	}
	m_isopen_camera = false;
	m_file_write.CloseSaveFile();
}

int CCameraRecord::RecordData(unsigned char *data, int len, unsigned int dwdatatype)
{
	m_cur_record_time = time(NULL);

#ifndef OLD_MANUAL_SAVA
	//if ( m_CameraRecordType == RECORD_MANUAL && strlen(m_manualSavePath) > 0 )
	if ( m_CameraRecordType >= RECORD_MANUAL && strlen(m_manualSavePath) > 0 )
	{
		time_t time_Now = time(NULL);
		tm *local = localtime(&time_Now);
		tm *oldLocal = localtime(&m_startManualFileTime);
		int bt_time = (int) difftime(time_Now, m_startManualFileTime);
		if( (bt_time >= MANUAL_RECORD_TIME*6) || (local->tm_mday != oldLocal->tm_mday) )
			CreateNewManualSaveFilePath();
		try
		{
			if(!m_isHaveHeadData)
			{
				unsigned int isign = 0;
				memcpy(&isign, data, sizeof(int));
				if (isign == 0xAAAB)
				{
					m_isHaveHeadData = true;
					memcpy(m_headData, data, len);
					m_headLen = len;
				}else
				{
					return 0;
				}
			}

			std::fstream m_videoStream;
			m_videoStream.open(m_manualSavePath, std::ios_base::out|std::ios_base::app|std::ios_base::binary);
			if(!m_videoStream.good() || m_videoStream.fail())return -1;
			m_videoStream.write( (const char*)data, len );
			len = (int)m_videoStream.gcount();
			int iCurlen = (int)m_videoStream.tellp();
			m_videoStream.close();

			if( m_curFileIndexTime_t != time(NULL) )
			{
				m_curFileIndexTime_t = time(NULL);

				boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());
				int totalSecs = t.time_of_day().total_seconds();
				std::fstream fstream_;
				fstream_.open(m_manualFileIndexPath, std::ios_base::out|std::ios_base::in|std::ios_base::binary);
				if(!fstream_.good() || fstream_.fail())return false;

				fstream_.seekp( (sizeof(time_t) + sizeof(int)) * totalSecs, std::ios_base::beg );
				fstream_.write((const char*)&m_curFileIndexTime_t, sizeof(time_t));
				fstream_.write((const char*)&iCurlen, sizeof(int));

				fstream_.close();

				int _seconds = (int)m_curFileIndexTime_t;
				if( _seconds%5 == 0 )
				{
					char chStartTime[256], chEndTime[256];
					memset(chStartTime, 0, 256);
					memset(chEndTime, 0, 256);
					tm *tmStartTime = localtime( &m_startManualFileTime );
					sprintf(chStartTime,"%04d%02d%02dT%02d%02d%02dZ",
						tmStartTime->tm_year + 1900, tmStartTime->tm_mon + 1, tmStartTime->tm_mday,
						tmStartTime->tm_hour, tmStartTime->tm_min, tmStartTime->tm_sec);

					time_t nowTime = time(NULL);
					tm *tmEndTime = localtime( &nowTime );
					sprintf(chEndTime,"%04d%02d%02dT%02d%02d%02dZ",
						tmEndTime->tm_year + 1900, tmEndTime->tm_mon + 1, tmEndTime->tm_mday,
						tmEndTime->tm_hour, tmEndTime->tm_min, tmEndTime->tm_sec);

					char chSQL[1024];memset(chSQL, 0, 1024);
					sprintf(chSQL,"update %s set EndTime = '%s' where CameraID = %d and StartTime = '%s';",
						m_strEventTableName.c_str(), chEndTime, m_cameraid, chStartTime);

					std::string strSQL = chSQL;
					m_manage_record->m_pDataInfo->ExecuteSQL(strSQL);
				}
			}

			return len;
		} catch (...) {
			return -1;
		}
		return len;
	}
#endif

	return m_file_write.WriteData( (const char *)data, len );
}

int CCameraRecord::CameraPTZCtrl(int ptzCMD, int param1, int param2)
{
	return m_prtsp_client->CameraPTZCtrlInPlay(m_url, ptzCMD, param1, param2);
}

void __stdcall CCameraRecord::SVideoSwitchCameraUpdateCallback(unsigned long dwOldCameraID,
	unsigned long dwNewCameraID, unsigned char *NewCameraName, unsigned long user)
{
	if( dwOldCameraID != dwNewCameraID ) 
	{
		CManageRecord::CameraRecordPtr cameraRecordPtr
			= ((CManageRecord *)user)->GetCameraRecordPtr(dwOldCameraID);
		if(cameraRecordPtr == NULL)return;
		cameraRecordPtr->SwitchCameraByCameraID(dwNewCameraID);
	}
}

bool CCameraRecord::CreateNewManualSaveFilePath()
{
#ifndef OLD_MANUAL_SAVA
	//if ( m_CameraRecordType == RECORD_MANUAL )
	if ( m_CameraRecordType >= RECORD_MANUAL )
	{
		time_t oldStartTime = m_startManualFileTime;
		time_t nowTime = time( NULL );
		boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());

		char str_jpg_file_path[256] = {0};
		char chVideoFileDBPath[256] = {0};
		char chVideoFileDBDir[256] = {0};
		char chVideoFileDBURL[256] = {0};
		sprintf( str_jpg_file_path, "%s\\_MANUAL_RECORD_\\%u\\%04d\\%02d\\%02d\\", 
			m_manage_record->GetPreAllocateDisk()->GetStoragePath().c_str(), 
			m_cameraid, t.date().year(), t.date().month(), t.date().day() );

		sprintf( chVideoFileDBDir, "%u/%04d/%02d/%02d/", 
			m_cameraid, t.date().year(), t.date().month(), t.date().day() );

		memset(m_manualSavePath, 0, 256);
		memset(m_manualFileIndexPath, 0, 256);
		if (IsSaveDirectory(str_jpg_file_path) != false)
		{
			sprintf(m_manualSavePath,"%s%d_%04d%02d%02dT%02d%02d%02dZ.264",
				str_jpg_file_path, m_cameraid, t.date().year(), t.date().month(), t.date().day(),
				t.time_of_day().hours(), t.time_of_day().minutes(), t.time_of_day().seconds());

			sprintf(chVideoFileDBPath,"%s%d_%04d%02d%02dT%02d%02d%02dZ.264",
				chVideoFileDBDir,m_cameraid, t.date().year(), t.date().month(), t.date().day(),
				t.time_of_day().hours(), t.time_of_day().minutes(), t.time_of_day().seconds());
			sprintf(chVideoFileDBURL, "%s%s", m_chMRURL, chVideoFileDBPath);

			m_startManualFileTime = nowTime;
			/////////////M_FILEINDEX////////////////////////////////////////////////////
			int fileindexsize = ( 24*60*60 + 1)*( sizeof(time_t) + sizeof(int) );
			boost::shared_ptr<char> chBuf = boost::shared_ptr<char>(new char[fileindexsize]);
			memset( chBuf.get(), 0 ,fileindexsize );

			sprintf(m_manualFileIndexPath,"%s%d.index", str_jpg_file_path, m_cameraid);

			fs::path path_( m_manualFileIndexPath );
			if(fs::exists(path_)){}
			else{
				std::fstream fstream_;
				fstream_.open(path_.c_str(), std::ios_base::out|std::ios_base::binary);
				if(!fstream_.good() || fstream_.fail() )return false;
				fstream_.seekp( 0, std::ios_base::beg );
				fstream_.write((const char*)chBuf.get(), fileindexsize);
				fstream_.close();
			}
			////////////////M_FILEINDEX//////////////////////////////////////////////////

			if( oldStartTime != 0 )//分段存储时更新结束时间
			{
				char chStartTime[256], chEndTime[256];
				memset(chStartTime, 0, 256);
				memset(chEndTime, 0, 256);
				tm *tmStartTime = localtime( &oldStartTime );
				sprintf(chStartTime,"%04d%02d%02dT%02d%02d%02dZ",
					tmStartTime->tm_year + 1900, tmStartTime->tm_mon + 1, tmStartTime->tm_mday,
					tmStartTime->tm_hour, tmStartTime->tm_min, tmStartTime->tm_sec);

				tm *tmEndTime = localtime( &nowTime );
				sprintf(chEndTime,"%04d%02d%02dT%02d%02d%02dZ",
					tmEndTime->tm_year + 1900, tmEndTime->tm_mon + 1, tmEndTime->tm_mday,
					tmEndTime->tm_hour, tmEndTime->tm_min, tmEndTime->tm_sec);

				char chSQL[1024];memset(chSQL, 0, 1024);
				sprintf(chSQL,"update %s set EndTime = '%s' where CameraID = %d and StartTime = '%s';",
					m_strEventTableName.c_str(), chEndTime, m_cameraid, chStartTime);

				std::string strSQL = chSQL;
				m_manage_record->m_pDataInfo->ExecuteSQL(strSQL);

				std::string strEndTime = chEndTime;
				std::string strVideoFileDBPath = chVideoFileDBPath;
				std::string strVideoFileDBURL = chVideoFileDBURL;
				m_manage_record->m_pDataInfo->InsertEventRecordInfo(m_cameraid, strEndTime, strVideoFileDBPath,
					strVideoFileDBURL, m_CameraRecordType, m_EventRecordID, m_strEventTableName);

				if(m_isHaveHeadData)
					RecordData((unsigned char *)m_headData, m_headLen );
			}
			else //新文件存储
			{
				char chStartTime[256];
				memset(chStartTime, 0, 256);
                tm *tmStartTime = localtime( &nowTime );
				sprintf(chStartTime,"%04d%02d%02dT%02d%02d%02dZ",
					tmStartTime->tm_year + 1900, tmStartTime->tm_mon + 1, tmStartTime->tm_mday,
					tmStartTime->tm_hour, tmStartTime->tm_min, tmStartTime->tm_sec);

				std::string strStartTime = chStartTime;
				std::string strVideoFileDBPath = chVideoFileDBPath;
				std::string strVideoFileDBURL = chVideoFileDBURL;
				int iRet = m_manage_record->m_pDataInfo->InsertEventRecordInfo(m_cameraid, strStartTime, strVideoFileDBPath,
					strVideoFileDBURL, m_CameraRecordType, m_EventRecordID, m_strEventTableName);
			}
			return true;
		}
		return false;
	}
	return false;
#endif
	return false;
}

bool CCameraRecord::IsSaveDirectory(const char * str_save_path)
{
	try{
		char str_sub_path[256] = {0};
		std::string str_solution_path("");
		str_solution_path = str_save_path;

		size_t npos =0;
		do{
			npos = str_solution_path.find('\\',npos + 1);
			memset(str_sub_path,0x00,sizeof(char)*256);
			str_solution_path.copy(str_sub_path,npos,0);

			if (str_solution_path.compare(str_sub_path) == 0)
			{
				npos = 1;
				break;
			}
			fs::path path_(str_sub_path);
			if ( fs::exists(path_) )
			{
				if( fs::is_directory(path_) )
					continue;
				else
				{
					npos = 0;
					break;
				}
			}
			else
			{
				if (fs::create_directory(path_) == false)
				{
					npos = 0;
					break;
				}
			}
		} while(npos != 0);

		return npos!=0 ? true : false;
	} catch(...){
		return false;
	}
}

}// end namespace
