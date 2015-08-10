#pragma once

#ifndef RTSPSERVER_H
#define RTSPSERVER_H

#include <list>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "MyLog.h"
#include "DevInfo.h"

namespace MediaSave
{
class CPreAllocateDisk;
}

namespace live555RTSP
{

class RTSPClientSession;
class ServerMediasession;
class RTSPServer
{
public:
	typedef boost::shared_ptr<RTSPClientSession> RTSPClientPtr;
	typedef boost::shared_ptr<ServerMediasession> ServerMediaPtr;

	typedef boost::shared_lock<boost::shared_mutex> readLock;
	typedef boost::unique_lock<boost::shared_mutex> writeLock;

#define MONITORSERVERMEDIA_TIME_SEC  60//300 
public:
	RTSPServer(void);
	~RTSPServer(void);

public:
	int InitRTSPServer(ICamera* pICamera, const char* ms_dbfile);
	int Start( unsigned short rtsp_port = 554, std::size_t thread_pool_size = 10 );
	void Run();
	int Stop();

	boost::asio::io_service& Get_io_service(){ return m_io_service_; }
	void SetIDeviceInRTSPServer(IDevice* pIDevice){ m_pDevice = pIDevice; if( m_pDevice )m_pDevice->SetIData( m_devinfo.GetDataInfoPtr() ); }
	//// RTSPClient method
	RTSPClientPtr FindRTSPClientBySessionID(unsigned int sessionid);
	void RemoveRTSPClientSession(RTSPClientPtr rtspclientptr);
	void RemoveRTSPClientBySessionID(unsigned int sessionid);

	///ServerMediaSession method
	ServerMediaPtr lookupServerMediaSession(const char* urlSuffix, std::list<int>& cameralist, std::vector<int>& channleLevels, int& curLevel);
	ServerMediaPtr FindServerMediaSession(int mediaid);
	bool IsExistMediaSessionByCameraID(int cameraid);
	void TryRemoveServerMediaSession(ServerMediaPtr mediaptr);
	void RemoveServerMediaSession(ServerMediaPtr mediaptr);

	unsigned short GetRTSPListenPort(){ return m_rtspListenPort; };
	std::string getRTSPAddress();
	CDevInfo& GetDevInfo(){ return m_devinfo; };

	void SetPreAllocateDisk(MediaSave::CPreAllocateDisk* pre_allocate_disk){ m_pPreAllocateDisk = pre_allocate_disk; };
	MediaSave::CPreAllocateDisk* GetPreAllocateDisk(){ return m_pPreAllocateDisk; };

	void SendMSLoadBalanceInfo();
	void PrintClientSessionInfoList();

	void SendMSGUpdateCameraName(int mediaid, int iCameraID = 0, const char* chUnitName = NULL, const char* chCameraName = "");
	int SendUpdateSwitchCameraInfo(int oldCameraID, int iNewCameraID);

	void PrintHttpInfo(std::string &strHtml);
	void PrintClientNUM(std::string &strHtml);
	void GetDeviceInfoByHttp(std::string strParam, std::string &strXml);

private:
	int StartAccpet();
	RTSPClientPtr CreateRTSPClientSession();
	void HandleAccept(RTSPClientPtr clientptr, const boost::system::error_code& error);
	void MonitorServerMediaThread();
	static bool CompareCoderID(ServerMediaPtr mediaptr1, ServerMediaPtr mediaptr2);

public:
	CMyLog m_log;
	std::vector<std::string> m_saveVideoUsers;
private:
	/// sign RTSPServer stop
	bool m_bStoped;

	/// The number of threads that will call io_service::run().
	std::size_t m_threadPoolSize;

	/// The io_service used to perform asynchronous operations.
	boost::asio::io_service m_io_service_;
	boost::asio::io_service::work m_work_;

	/// Acceptor used to listen for incoming connections.
	boost::asio::ip::tcp::acceptor m_acceptor_;

	/// rtsp listen port
	unsigned short m_rtspListenPort;

	/// RTSPClient list
	std::list<RTSPClientPtr> m_rtspClientList;
	/// rtsp_client_list mutex
	boost::shared_mutex m_clientListMutex_;

	/// ServerMediasession list
	std::list<ServerMediaPtr> m_serverMediaList;
	/// ServerMediasession mutex
	boost::shared_mutex m_serverMediaMutex_;

	CDevInfo m_devinfo;

	std::vector< boost::shared_ptr<boost::thread> > m_threadsVector;
	MediaSave::CPreAllocateDisk* m_pPreAllocateDisk;

	boost::shared_ptr<boost::thread> m_pMonitorMediaThread;

	bool m_IsRecordUserLog;

	boost::posix_time::ptime m_ptime_startup;

	IDevice* m_pDevice;

	int m_iSubSystemID;
};

}// end namespace

#endif // RTSPSERVER_H