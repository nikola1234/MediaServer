#include "RTSPServer.h"
#include "RTSPClientSession.h"
#include "ServerMediasession.h"
#include "MRServerMediasession.h"
#include "AgentServerMediasession.h"
#include "PreAllocateDisk.h"

#include "boost/filesystem.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>


namespace live555RTSP
{

RTSPServer::RTSPServer(void)
	:m_threadPoolSize(5),
	m_acceptor_(m_io_service_),
	m_rtspListenPort(554),
	m_bStoped(true),
	m_work_(m_io_service_),
	m_pPreAllocateDisk(NULL),
	m_IsRecordUserLog(false),
	m_pDevice(NULL),
	m_iSubSystemID(-1)
{

}

int RTSPServer::InitRTSPServer(ICamera* pICamera, const char* ms_dbfile)
{
	m_ptime_startup = boost::posix_time::second_clock::local_time();
	boost::filesystem::create_directory("log");
	m_log.InitLog("./log/MS-");
	return m_devinfo.initDevInfo(pICamera, ms_dbfile);
}

RTSPServer::~RTSPServer(void)
{
	m_rtspClientList.clear();
	m_serverMediaList.clear();
}

int RTSPServer::Start(unsigned short rtsp_port, std::size_t thread_pool_size)
{

	m_rtspListenPort = rtsp_port;
	m_threadPoolSize = thread_pool_size;

	int iret = -1;
	try
	{
		boost::asio::ip::tcp::endpoint endpoint(
			boost::asio::ip::tcp::v4(), m_rtspListenPort);
		m_acceptor_.open(endpoint.protocol());
		m_acceptor_.bind(endpoint);
		m_acceptor_.listen();

		StartAccpet();
		m_bStoped = false;
		iret = 0;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return iret;
}

void RTSPServer::Run()
{
	// Create a pool of threads to run all of the io_services.
	for (std::size_t i = 0; i < m_threadPoolSize; ++i)
	{
		boost::shared_ptr<boost::thread> thread(new boost::thread(
			boost::bind(&boost::asio::io_service::run, &m_io_service_)));
		m_threadsVector.push_back(thread);
	}

	m_pMonitorMediaThread = 
		(boost::shared_ptr<boost::thread>)new boost::thread( boost::bind(&RTSPServer::MonitorServerMediaThread, this));
}

int RTSPServer::StartAccpet()
{
	int iret = -1;
	try
	{
		RTSPClientPtr clientptr = CreateRTSPClientSession();
		m_acceptor_.async_accept(clientptr->GetrtspSocket(),
			boost::bind(&RTSPServer::HandleAccept, this, clientptr,
			boost::asio::placeholders::error));
	}
	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return iret;
}

void RTSPServer::HandleAccept(RTSPClientPtr clientptr,
	const boost::system::error_code& error)
{
	if (m_bStoped)
		return;

	if (!error)
	{
		if (clientptr)
			clientptr->Start();
	}
	else
	{
		m_log.Add("RTSPServer::HandleAccept error = %s", error.message().c_str());
	}

	StartAccpet();
}

int RTSPServer::Stop()
{
	m_bStoped = true;

	{
		writeLock writelock_(m_clientListMutex_);
		std::list<RTSPClientPtr>::iterator it = m_rtspClientList.begin();
		for ( ; it != m_rtspClientList.end(); it++ )
			(*it)->Stop();
	}

	boost::system::error_code ignored_ec;
	m_acceptor_.close(ignored_ec);
	m_io_service_.stop();

	// Wait for all threads in the pool to exit.
	for (std::size_t i = 0; i < m_threadsVector.size(); ++i)
		m_threadsVector[i]->join();

	if(m_pMonitorMediaThread != NULL)
		m_pMonitorMediaThread->join();
	return 0;
}

RTSPServer::RTSPClientPtr RTSPServer::CreateRTSPClientSession()
{
    srand(unsigned(time(0)));
	RTSPClientPtr clientptr = RTSPClientPtr(new RTSPClientSession(this,
		((unsigned int)((rand()%128) + 1)) * 16777216 + RTSPClientSession::GetStaticFakeClientNUM()));
	writeLock writelock(m_clientListMutex_);
	m_rtspClientList.push_front(clientptr);
	return clientptr;
}

RTSPServer::RTSPClientPtr RTSPServer::FindRTSPClientBySessionID(unsigned int sessionid)
{
	RTSPClientPtr clientptr;
	readLock readlock_(m_clientListMutex_);
	std::list<RTSPClientPtr>::iterator it = m_rtspClientList.begin();
	for ( ; it != m_rtspClientList.end() ; it++ )
	{
		if ((*it)->GetOurSessionID() == sessionid)
		{
			clientptr = *it;
			return clientptr;
		}
	}
	return clientptr;
}

void RTSPServer::RemoveRTSPClientBySessionID(unsigned int sessionid)
{
	writeLock writelock_(m_clientListMutex_);
	std::list<RTSPClientPtr>::iterator it = m_rtspClientList.begin();
	for ( ; it != m_rtspClientList.end() ; it++ )
	{
		if ((*it)->GetOurSessionID() == sessionid)
		{
			m_rtspClientList.erase(it);
			return;
		}
	}
}

RTSPServer::ServerMediaPtr RTSPServer::lookupServerMediaSession(
	const char* urlSuffix, std::list<int>& cameralist, std::vector<int>& channleLevels, int& curLevel )
{
	int cameraid = atoi(urlSuffix);

	ServerMediaPtr mediaptr;
	std::string strSuffix = urlSuffix;
	if( strSuffix.length() > 4 )//url=rtsp://ip:port/url=onvif://user:pwd@ip:port/1
	{
		std::string strURLchar = strSuffix.substr(0, 3);
		std::string strURLSuffix = strSuffix.substr(4, strSuffix.length() - 4);
		if( strURLchar.compare("url") == 0 )
		{
			{
				//复制转发
				readLock readlock_(m_serverMediaMutex_);
				std::list<ServerMediaPtr>::iterator it = m_serverMediaList.begin();
				for ( ; it != m_serverMediaList.end() ; it++ ) {
					if (((*it)->GetUrlSuffix().compare(strURLSuffix) == 0) && (*it)->IsLive()) {
						mediaptr = *it;
						return mediaptr;
					}
				}
				readlock_.unlock();
			}

			static int s_cameraid = 100100001;
			CDataInfo::DBCAMERAINFO cameradata;
			cameradata.isMatrix = 0;
			cameradata.CameraID = s_cameraid++;
			mediaptr = ServerMediaPtr( new AgentServerMediasession (this, cameradata, ServerMediasession::GetFreeMediaID(), strURLSuffix) );
			writeLock writelock_(m_serverMediaMutex_);
			m_serverMediaList.push_back(mediaptr);
			return mediaptr;
		}
	}

	if(cameraid == 0) return mediaptr;

	{
		readLock readlock_(m_serverMediaMutex_);
		std::list<ServerMediaPtr>::iterator it = m_serverMediaList.begin();
		for ( ; it != m_serverMediaList.end() ; it++ ) {
			if (((*it)->GetCurCameraID() == cameraid) && (*it)->IsLive()) {
				mediaptr = *it;
				return mediaptr;
			}
		}
		readlock_.unlock();
	}

	CDataInfo::DBCAMERAINFO cameradata;
	int iRet = m_devinfo.GetDataInfoPtr()->getCameraInfo( cameraid, &cameradata );
	if( iRet < 0 )return mediaptr;
	if ( cameradata.isMatrix == 0 )//无矩阵通道
	{
		mediaptr = ServerMediaPtr(new ServerMediasession(this,cameradata));//mediaid = cameraid
		writeLock writelock_(m_serverMediaMutex_);
		m_serverMediaList.push_back(mediaptr);
		return mediaptr;
	}
	return mediaptr;
}

RTSPServer::ServerMediaPtr RTSPServer::FindServerMediaSession(int mediaid)
{
	ServerMediaPtr mediaptr;

	readLock readlock_(m_serverMediaMutex_);
	std::list<ServerMediaPtr>::iterator it = m_serverMediaList.begin();
	for ( ; it != m_serverMediaList.end() ; it++ )
	{
		if ((*it)->GetMediaID() == mediaid)
		{
			mediaptr = *it;
			return mediaptr;
		}
	}

	return mediaptr;
}

bool RTSPServer::IsExistMediaSessionByCameraID(int cameraid)
{
	readLock readlock_(m_serverMediaMutex_);
	std::list<ServerMediaPtr>::iterator it = m_serverMediaList.begin();
	for ( ; it != m_serverMediaList.end() ; it++ )
	{
		if (((*it)->GetCurCameraID() == cameraid) && (*it)->IsLive())
		{
			return true;
		}
	}
	return false;
}

void RTSPServer::RemoveRTSPClientSession(RTSPClientPtr rtspclientptr)
{
	writeLock writelock_(m_clientListMutex_); 
	m_rtspClientList.remove(rtspclientptr); 
}

void RTSPServer::RemoveServerMediaSession(ServerMediaPtr mediaptr)
{
	writeLock writelock_( m_serverMediaMutex_ );
	m_serverMediaList.remove(mediaptr);
}

void RTSPServer::TryRemoveServerMediaSession(ServerMediaPtr mediaptr)
{
	if( mediaptr->GetSenderCount() == 0 ){

		writeLock writelock_( m_serverMediaMutex_ );
		m_serverMediaList.remove(mediaptr);
		writelock_.unlock();
		
		mediaptr->StopMediaServer();
	}
}

void RTSPServer::SendMSGUpdateCameraName( int mediaid, int iCameraID, const char* chUnitName, const char* chCameraName )
{
	//RTSPClientPtr clientptr;
	readLock readlock_(m_clientListMutex_);
	std::list<RTSPClientPtr>::iterator it = m_rtspClientList.begin();
	for (; it != m_rtspClientList.end() ; it++){
		if( (*it)->GetMediaID() == mediaid )
			(*it)->SendANNOUNCEMessage(iCameraID, chUnitName, chCameraName);
	}
}

int RTSPServer::SendUpdateSwitchCameraInfo(int oldCameraID, int iNewCameraID)
{
	ServerMediaPtr mediaptr;
	{
		readLock readlock_(m_serverMediaMutex_);
		std::list<ServerMediaPtr>::iterator it = m_serverMediaList.begin();
		for ( ; it != m_serverMediaList.end() ; it++ )
		{
			if ((*it)->GetCurCameraID() == oldCameraID)
			{
				mediaptr = *it;
			}
		}
		if ( mediaptr == NULL ) return -1;
		readlock_.unlock();
	}
	m_devinfo.GetDataInfoPtr()->getCameraInfo(iNewCameraID, mediaptr->GetCameraDataPtr());
	SendMSGUpdateCameraName(mediaptr->GetMediaID());
	return 0;
}

std::string RTSPServer::getRTSPAddress()
{
	std::string ipv4str ;
	char ip[64] = { 0 };
	m_devinfo.GetDataInfoPtr()->GetConfig( "MSIP", ip );
	ipv4str = ip;
	return ipv4str;
}

void RTSPServer::MonitorServerMediaThread()
{
	while( !m_bStoped )
	{
		ServerMediaPtr mediaptr;
		{
			readLock readlock_(m_serverMediaMutex_);
			std::list<ServerMediaPtr>::iterator it = m_serverMediaList.begin();
			for ( ; it != m_serverMediaList.end() ; it++ ) {
				int bt_time = (int) difftime(time(NULL), (*it)->GetCurRecvDataTime());
				if( bt_time > MONITORSERVERMEDIA_TIME_SEC ){
					mediaptr = (*it);
					break;
				}
			}
			readlock_.unlock();
		}
		if(mediaptr != NULL){
			m_log.Add("RTSPServer::MonitorServerMediaThread RecvData TimeOut mediaid = %d cameraid = %d",
				mediaptr->GetMediaID(), mediaptr->GetCurCameraID());
			mediaptr->StopMediaServer();
			RemoveServerMediaSession(mediaptr);
			continue;
		}
		int iexit_second = 0;
		while(iexit_second++ < MONITORSERVERMEDIA_TIME_SEC){
			boost::this_thread::sleep( boost::posix_time::seconds( 1 ) );
			if( m_bStoped )return;
		}
	}
}

void RTSPServer::PrintHttpInfo(std::string &strHtml)
{
	strHtml = "<html><head><title>MS INFO</title>"
		"<meta http-equiv='content-type' content='text/html'>"
		"</head><body><center><h2>MS 内部运行信息</h2>"
		"正在调看的摄像机信息表"
		"<table width='700' border='1' bordercolor='black' cellspacing='1'>"
		"<tr><td width='70'><B>序号</B></td><td width='100'><B>媒体ID</B></td><td width='100'><B>摄像机ID</B></td>"
		"<td width='200'><B>摄像机名称</B></td><td><B>客户数</B></td></tr>";
	{
		readLock readlock_(m_serverMediaMutex_);
		std::list<ServerMediaPtr>::iterator it = m_serverMediaList.begin();
		for (int i = 1 ; it != m_serverMediaList.end() ; it++, i++ )
		{
			int sender_count = (*it)->GetSenderCountNoLock();

			strHtml += "<tr><td>" + boost::lexical_cast<std::string>(i) + 
				"</td><td>" +  boost::lexical_cast<std::string>((*it)->GetMediaID()) + 
				"</td><td>" +  boost::lexical_cast<std::string>((*it)->GetCurCameraID()) +
				"</td><td>" +  boost::lexical_cast<std::string>((*it)->GetCameraDataPtr()->CameraName) +
				"</td><td>" +  boost::lexical_cast<std::string>(sender_count) +
				"</td></tr>";
		}
		readlock_.unlock();
	}

	strHtml += "</table></br>正在调看的用户信息表"
		"<table width='850' border='1' bordercolor='black' cellspacing='1'>"
		"<tr><td width='50'><B>序号</B></td><td width='70'><B>媒体ID</B></td><td width='70'><B>摄像机ID</B></td>"
		"<td width='70'><B>会话ID</B></td><td width='70'><B>用户名</B></td><td width='100'><B>用户等级</B></td>"
		"<td width='100'><B>IP</B></td><td width='50'><B>PORT</B></td><td width='170'><B>OPEN TIME</B></td></tr>";
	{
		readLock readlock_(m_clientListMutex_);
		std::list<RTSPClientPtr>::iterator it = m_rtspClientList.begin();
		int count = 1; 
		for (; it != m_rtspClientList.end() ; it++)
		{
			if ( (*it)->GetMediaID() == 0 ) continue;

			std::string clientIP;
			int clientPort = 0;
			boost::system::error_code ec;
			boost::asio::ip::tcp::endpoint endpoint = (*it)->GetrtspSocket().remote_endpoint(ec);
			if( !ec ){
				clientIP = endpoint.address().to_string(ec);
				clientPort = endpoint.port();
			}
			strHtml += "<tr><td>" + boost::lexical_cast<std::string>(count) + 
				"</td><td>" +  boost::lexical_cast<std::string>((*it)->GetMediaID()) + 
				"</td><td>" +  boost::lexical_cast<std::string>((*it)->GetCameraID()) +
				"</td><td>" +  boost::lexical_cast<std::string>((*it)->GetOurSessionID()) +
				"</td><td>" +  boost::lexical_cast<std::string>((*it)->GetUserName()) +
				"</td><td>" +  boost::lexical_cast<std::string>((*it)->GetUserLevel()) +
				"</td><td>" +  clientIP +
				"</td><td>" +  boost::lexical_cast<std::string>(clientPort) +
				"</td><td>" +  boost::posix_time::to_iso_extended_string((*it)->GetOpenTime()) +
				"</td></tr>";
			count++;
		}
		readlock_.unlock();
	}
	strHtml += "</table></br>"
		"此MS启动时间：" + boost::posix_time::to_iso_extended_string(m_ptime_startup) + 
		"</center></body></html>";
}

void RTSPServer::PrintClientNUM(std::string &strHtml)
{
	int count = 0; 
	readLock readlock_(m_serverMediaMutex_);
	std::list<ServerMediaPtr>::iterator it = m_serverMediaList.begin();
	for (int i = 1 ; it != m_serverMediaList.end() ; it++, i++ )
	{
		//int sender_count = (*it)->GetSenderCountNoLock();
		int sender_count = (*it)->GetSenderCount();
		count += sender_count;
	}
	readlock_.unlock();

	strHtml = "<?xml version=\"1.0\"?><Response><QueryResponse><Variable>BandWidth</Variable><Result>0</Result>"
		"<Manufacturer>itssky</Manufacturer><All>256</All><Free>" + boost::lexical_cast<std::string> ( 256 - count ) +
		"</Free><MediaLink>" + boost::lexical_cast<std::string> ( count ) + "</MediaLink></QueryResponse></Response>";
}

void RTSPServer::GetDeviceInfoByHttp(std::string strParam, std::string &strXml)
{
	char* outResult = new char[65536];
	memset( outResult, 0, 65536 );
	if( m_pDevice != NULL )
		m_pDevice->getDeviceInfo( strParam.c_str(), outResult );
	if( strlen(outResult) != 0 )
		strXml = outResult;
	delete[] outResult;
}

bool RTSPServer::CompareCoderID(ServerMediaPtr mediaptr1, ServerMediaPtr mediaptr2)
{
	if( mediaptr1->GetCurCoderID() <= mediaptr2->GetCurCoderID() )
		return true;
	return false;
}

}// end namespace