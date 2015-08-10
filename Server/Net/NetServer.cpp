#include "NetServer.h"
#include "boost/filesystem.hpp"

NetServer::NetServer()
  :	m_acceptor_(m_io_service_),
	  m_clientListenPort(9098),
	  m_work_(m_io_service_)
{
  m_Num =0;
  memset(log_file, 0 ,40);
}
NetServer::~NetServer(void)
{
	m_ClientList.clear();
}

int NetServer::InitNetServer()
{

	boost::filesystem::create_directory("log");

	m_log.InitLog("./log/NS-");
  m_log.Add("NetServer Start !");
	return 0;
}

int NetServer::Start()
{
	int iret = -1;
	try
	{
		boost::asio::ip::tcp::endpoint endpoint(
			boost::asio::ip::tcp::v4(), m_clientListenPort);
		m_acceptor_.open(endpoint.protocol());
		m_acceptor_.bind(endpoint);
		m_acceptor_.listen();

		StartAccpet();
		iret = 0;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return iret;
}

int NetServer::StartAccpet()
{
	int iret = -1;
	try
	{
    NetClientPtr clientptr = CreateNetClientSession();
		m_acceptor_.async_accept(clientptr->GetClientSocket(),
			boost::bind(&NetServer::HandleAccept, this, clientptr,
			boost::asio::placeholders::error));
	}
	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return iret;
}

NetServer::NetClientPtr NetServer::CreateNetClientSession()
{
  m_Num++;
  if(m_Num  >= 16777216) m_Num = 1;

	NetClientPtr clientptr = NetClientPtr(new NetClientSession(this,m_Num));

	writeLock writelock(m_clientListMutex_);
	m_ClientList.push_front(clientptr);
	return clientptr;
}

void NetServer::RemoveNetClientBySessionID(unsigned int sessionid)
{
	writeLock writelock_(m_clientListMutex_);
	std::list<NetClientPtr>::iterator it = m_ClientList.begin();
	for ( ; it != m_ClientList.end() ; it++ )
	{
		if ((*it)->GetOurSessionID() == sessionid)
		{
			m_ClientList.erase(it);
			return;
		}
	}
}

void NetServer::HandleAccept(NetClientPtr clientptr,
	const boost::system::error_code& error)
{
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

void NetServer::Run()
{
  m_pNetServerThread =
  		(boost::shared_ptr<boost::thread>)new boost::thread(boost::bind(&boost::asio::io_service::run, &m_io_service_));
}
