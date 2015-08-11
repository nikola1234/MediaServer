
#include "NetClientSession.h"

NetClientSession::NetClientSession(NetServer* ourServer,unsigned int sessionId)
:fOurServer(ourServer),fOurSessionId(sessionId),
fSessionIsActive(true), client_socket_(ourServer->Get_io_service()),
stop_(true)
{
  memset(fAcceptBuffer, 0 ,BUFFER_SIZE);
  memset(fBuffer, 0 ,BUFFER_SIZE_MIN);
  PKGNUM = 0;
  totalRecive = 0;
}

NetClientSession::~NetClientSession(void)
{
  boost::system::error_code ec;
	client_socket_.shutdown(boost::asio::socket_base::shutdown_both,ec);
	client_socket_.close(ec);
  fOurServer->m_log.Add("NetClientSession::~NetClientSession fOurSessionId = %d", fOurSessionId);
}

int NetClientSession::Start()
{
	stop_ = false;
	int iret = -1;
	incomingAcceptHandler();
	iret = 0;

	fOurServer->m_log.Add("NetClientSession::Start fOurSessionId = %d", fOurSessionId);
	return iret;
}

void NetClientSession::incomingAcceptHandler()
{
	try {
		  client_socket_.async_read_some(
      //boost::asio::read(client_socket_,
			boost::asio::buffer(fBuffer,1024),
			boost::bind(&NetClientSession::HandleRecvReponse,
			shared_from_this(),boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
	catch (...){  }

}

int ReciveData_GetParam(char* buffer int size)
{

}

void NetClientSession::HandleRecvReponse(const boost::system::error_code& error, int recvsizes)
{
  if(stop_)return;
	if (!error)
	{
    //std::cout<<recvsizes<<std::endl;
    totalRecive += recvsizes;
    if(recvsizes == BUFFER_SIZE_MIN)
    {
      memcpy(fAcceptBuffer+BUFFER_SIZE_MIN*PKGNUM,fBuffer,BUFFER_SIZE_MIN);
      PKGNUM++;
      memset(fBuffer, 0 ,BUFFER_SIZE_MIN);
      incomingAcceptHandler();
    }
    else if(recvsizes < BUFFER_SIZE_MIN)
    {
        memcpy(fAcceptBuffer+BUFFER_SIZE_MIN*PKGNUM,fBuffer,recvsizes);
        std::cout<<fAcceptBuffer<<std::endl;
        std::cout<<totalRecive<<std::endl;
        ReciveData_GetParam(fAcceptBuffer,totalRecive);
        memset(fBuffer, 0 ,BUFFER_SIZE_MIN);
        PKGNUM = 0;
        totalRecive = 0;
        incomingAcceptHandler();
    }
  }
  else{
		std::cerr<< "NetClientSession::HandleRecvReponse : " << error.message() << std::endl;
	}

}
int NetClientSession::SendMessage(char *buff,int size)
{
  try {
		  client_socket_.async_send(
			boost::asio::buffer(buff,size),
			boost::bind(&NetClientSession::HandleSendResponse,shared_from_this(),
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));

		return 0;

	}catch (...){  }

	return 0;
}

void NetClientSession::HandleSendResponse(const boost::system::error_code& error, int sendsizes)
{
	if (!error){
    std::cout<<"sendsizes:"<<sendsizes<<std::endl;
  }
	else{

	}
}
