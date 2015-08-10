#include "stdafx.h"
#include "ConnectMC.h"
#include "OpenCamera.h"
#include "PlaySave.h"

boost::atomic<int> CConnectMC::s_loginHandle(1);

CConnectMC::CConnectMC(void)
	:m_mcport(0),
	m_bMonitorRun(true),
	m_clientType(0)
{
	m_loginHandle = s_loginHandle++;

	memset(m_mcip,0, 256);
	memset(m_username, 0, 256);
	memset(m_password, 0, 256);

	m_pMonitorThread = 
		boost::shared_ptr< boost::thread > (new boost::thread( boost::bind( &CConnectMC::MonitorOpenCameraThread, this ) ) );
}

CConnectMC::~CConnectMC(void)
{
	StopAllCamera();
}

CConnectMC::OpenCameraPtr CConnectMC::CreateOpenCamera()
{
	OpenCameraPtr open_cameraptr_(new COpenCamera());
	writeLock wrlock(m_openCameraMutex);
	m_openCameraList.push_front(open_cameraptr_);
	return open_cameraptr_;
}

CConnectMC::OpenCameraPtr CConnectMC::CreatePlaySave()
{
	OpenCameraPtr open_cameraptr_(new CPlaySave());
	writeLock wrlock(m_openCameraMutex);
	m_openCameraList.push_front(open_cameraptr_);
	return open_cameraptr_;
}

CConnectMC::OpenCameraPtr CConnectMC::FindOpenCameraByMediaID(int mediaid)
{
	OpenCameraPtr open_cameraptr_;

	readLock rdlock(m_openCameraMutex);
	std::list<OpenCameraPtr>::iterator it = m_openCameraList.begin();
	for (; it != m_openCameraList.end();it++){
		if((*it)->GetMediaID() == mediaid)
			return (*it);
	}

	return open_cameraptr_;
}

CConnectMC::OpenCameraPtr CConnectMC::FindOpenCameraByCameraID(int cameraid)
{
	OpenCameraPtr open_cameraptr_;

	readLock rdlock(m_openCameraMutex);
	std::list<OpenCameraPtr>::iterator it = m_openCameraList.begin();
	for (; it != m_openCameraList.end();it++){
		if((*it)->GetCameraID() == cameraid)
			return (*it);
	}

	return open_cameraptr_;
}

CConnectMC::OpenCameraPtr CConnectMC::FindOpenCameraByUser(int user)
{
	OpenCameraPtr open_cameraptr_;

	readLock rdlock(m_openCameraMutex);
	std::list<OpenCameraPtr>::iterator it = m_openCameraList.begin();
	for (; it != m_openCameraList.end();it++){
		if((*it)->GetUser() == user)
			return (*it);
	}

	return open_cameraptr_;
}

int CConnectMC::FindNoOpenCameraID(std::list<int>& cameraidList, int& cameraidNO)
{
	int cameraID = -1;
	cameraidNO = 0;
	std::list<int>::iterator cameraID_IT = cameraidList.begin();
	for(; cameraID_IT != cameraidList.end(); cameraID_IT++)
	{
		cameraidNO++;
		readLock rdlock(m_openCameraMutex);
		std::list<OpenCameraPtr>::iterator it = m_openCameraList.begin();
		for (; it != m_openCameraList.end();it++){
			if( (*it)->GetCameraID() == *cameraID_IT )
				break;
		}
		if( it == m_openCameraList.end() ){
			cameraID = *cameraID_IT;
			break;
		}
	}
	return cameraID;
}

void CConnectMC::RemoveOpenCameraByMediaID(int mediaid)
{
	writeLock wrlock(m_openCameraMutex);
	std::list<OpenCameraPtr>::iterator it = m_openCameraList.begin();
	for (; it != m_openCameraList.end();it++){
		if((*it)->GetMediaID() == mediaid){
			it = m_openCameraList.erase(it);
			return;
		}
	}
}

void CConnectMC::StopAllCamera()
{
	m_bMonitorRun = false;
	if(m_pMonitorThread != NULL){
		m_pMonitorThread->timed_join(boost::posix_time::millisec(1500));
		//m_pMonitorThread->join();
		boost::shared_ptr< boost::thread > pPtrNULL;
		m_pMonitorThread = pPtrNULL;
	}
	writeLock wrlock(m_openCameraMutex);
	std::list<OpenCameraPtr>::iterator it = m_openCameraList.begin();
	for (; it != m_openCameraList.end(); ){
		(*it)->StopCamera();
		it = m_openCameraList.erase(it);
	}
}

bool CConnectMC::IsExistConnectMC(char *ip, int port, char *username)
{
	if(strcmp(ip, m_mcip) == 0 &&
		port == m_mcport &&
		strcmp(username, m_username) == 0)
	{
		return true;
	}
	return false;
}

void CConnectMC::MonitorOpenCameraThread()
{
	while( m_bMonitorRun )
	{
		OpenCameraPtr open_cameraptr_;
		{
			readLock rdlock(m_openCameraMutex);
			std::list<OpenCameraPtr>::iterator it = m_openCameraList.begin();
			for (; it != m_openCameraList.end();it++){
				int bt_time = (int) difftime(time(NULL), (*it)->GetCurRecvDataTime());
				if( bt_time > MONITOROPENCAMERA_TIME_SEC && !(*it)->IsPlaySave() ){
					open_cameraptr_ = (*it);
					break;
				}
			}
		}
		if(open_cameraptr_ != NULL){

			int iRet = open_cameraptr_->ReStartCamera();
			g_MSNetSDKlog.Add("COpenCamera::ReStartCamera  nret = %d",  iRet);
			if(  iRet == ERROR_CODEC_FULL )
			{
				open_cameraptr_->StopCamera();
				RemoveOpenCameraByMediaID(open_cameraptr_->GetMediaID());
			}
			else if( iRet < 0 )
			{
				open_cameraptr_->PutCurRecvDataTime();
			}
			else
			{
				open_cameraptr_->ResetCurRecvDataTime();
			}

			continue;
		}

		int iexit_second = 0;
		while(iexit_second++ < MONITOROPENCAMERA_TIME_SEC){
			boost::this_thread::sleep( boost::posix_time::seconds( 1 ) );
			if( !m_bMonitorRun )return;
		}
	}
}
