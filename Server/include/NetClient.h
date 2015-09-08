#ifndef _NET_CLIENT_H_
#define _NET_CLIENT_H_

#include "Common.h"
#include "CmdDefine.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "CameraManage.h"

#include "MyLog.h"

#define PORT_LEN_10 10

class ManageCamera;
class NetClient
{
public:

  typedef boost::shared_lock<boost::shared_mutex> readLock;
  typedef boost::unique_lock<boost::shared_mutex> writeLock;

public:
	NetClient(void);
	~NetClient(void);
	
public:	

	void AddManaCamera(ManageCamera *cam){ m_camMana = cam;}
	
	void start(char *ip ,uint32 port);
	int SendToServer(char *buf, int len);
	void Run(void);	


private:
	CMyLog m_log;
	ManageCamera *m_camMana; 
		
	bool m_isConnected;
	bool m_checkConnect;
	boost::shared_mutex m_ConnStatusMutex_;
	
	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::socket tcp_socket_;
	
	char m_hostname[IP_LEN_16];
	char m_port[PORT_LEN_10];

	boost::shared_ptr<boost::thread> m_clientThread_;
	boost::shared_ptr<boost::thread> m_connectThread_;

 	boost::asio::deadline_timer timer1_; 
 
	void CloseSocket(void);

	int Send(char *buf, int len);
	
	bool IsConnected(void);
	void ChangeConnect(bool staus);	
	void CheckConnect(void);
	int ConnectingtoSvr(void);
	//void HandleConnect(const boost::system::error_code& error) ;
	void SendPicToServer(void);
	void RegularEvent(void);
	
};

#endif
