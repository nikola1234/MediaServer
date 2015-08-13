
#ifndef _NET_SERVER_H_
#define _NET_SERVER_H_

#include <list>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "MyLog.h"
#include "Common.h"
#include "NetClientSession.h"
#include "CameraManage.h"

class ManageCamera;
class NetClientSession;
class NetServer
{
public:
  typedef boost::shared_ptr<NetClientSession> NetClientPtr;
  typedef boost::shared_lock<boost::shared_mutex> readLock;
  typedef boost::unique_lock<boost::shared_mutex> writeLock;
  
public:
  	NetServer();
  	~NetServer();
	  void AddManaCamera(ManageCamera *cam){ ManCam = cam;}
public:

	boost::asio::io_service& Get_io_service(){ return m_io_service_; }
	int InitNetServer(uint32 port);
	int Start();
	int StartAccpet();
	NetClientPtr CreateNetClientSession();
	void RemoveNetClientBySessionID(unsigned int sessionid);

	void HandleAccept(NetClientPtr clientptr,const boost::system::error_code& error);
	void Run();

	unsigned int  GetClientListSize(){return m_ClientList.size() ;}
private:

	// The io_service used to perform asynchronous operations.
	boost::asio::io_service m_io_service_;
	boost::asio::io_service::work m_work_;

	// Acceptor used to listen for incoming connections.
	boost::asio::ip::tcp::acceptor m_acceptor_;

	// client listen port
	unsigned short m_clientListenPort;

	std::list<NetClientPtr> m_ClientList;

	// client_list mutex
	boost::shared_mutex m_clientListMutex_;

	//NetServer pthread
	boost::shared_ptr<boost::thread>  m_pNetServerThread;


public:
	CMyLog m_log;
	unsigned int m_Num;
	char log_file[40];
	boost::posix_time::ptime  ptime_startup;
	ManageCamera *ManCam;
};

#endif
