#ifndef _NET_CLIENT_SESSION_H_
#define _NET_CLIENT_SESSION_H_

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/system/error_code.hpp>
#include "NetServer.h"
#include "CmdDefine.h"

#define  BUFFER_SIZE  (10*1024)
#define  BUFFER_SIZE_MIN  (512)
class NetServer;
class NetClientSession
: public boost::enable_shared_from_this<NetClientSession>
{
public:
  	NetClientSession(NetServer * ourServer, unsigned int sessionId);
  	~NetClientSession(void);

public:
    boost::asio::ip::tcp::socket& GetClientSocket(){ return client_socket_; }
    int Start();
    unsigned int GetOurSessionID(){ return fOurSessionId; }

private:

  	void HandleRecvReponse(const boost::system::error_code& error, int recvsizes);
  	void incomingAcceptHandler();

    int SendMessage(char *buff,int size);
    void HandleSendResponse(const boost::system::error_code& error, int sendsizes);

private:
    	NetServer* fOurServer;
      unsigned int fOurSessionId;
      unsigned char fAcceptBuffer[BUFFER_SIZE];
      unsigned char fBuffer[BUFFER_SIZE_MIN];
      unsigned int PKGNUM;
      unsigned int totalRecive;

      boost::asio::ip::tcp::socket client_socket_;
      int m_clientid;
      bool fSessionIsActive;
      bool stop_;
};

#endif
