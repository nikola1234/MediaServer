
#include "NetClientSession.h"
#include "Xmlparser.h"

extern T_ServerParam SerParam;


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

NetClientSession::~NetClientSession()
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

int NetClientSession::client_register_ack()
{
  T_PacketHead						     t_PackHeadRegAck;
	T_ANAY_VDCS_REGISTER_ACK     t_RegAck;
	char  RegAckBuff[28 + 20] ={0};
	int 	iRet = -1;

	t_PackHeadRegAck.magic			  =  T_PACKETHEAD_MAGIC;
	t_PackHeadRegAck.cmd			    =  SM_VDCS_ANAY_REGISTER_ACK;
	t_PackHeadRegAck.UnEncryptLen	=  sizeof(T_ANAY_VDCS_REGISTER_ACK);

	memcpy(RegAckBuff,&t_PackHeadRegAck,sizeof(T_PacketHead));
  t_RegAck.ServerID = SerParam.ServerID;
	memcpy(t_RegAck.Serverip ,SerParam.Serverip,IP_LEN_16);
	memcpy(RegAckBuff+sizeof(T_PacketHead),&t_RegAck,sizeof(T_ANAY_VDCS_REGISTER_ACK));
  SendMessage(RegAckBuff,sizeof(RegAckBuff));
  return 0;
}

int NetClientSession::push_camera_data(char* buffer ,int size)
{
    ST_VDCS_VIDEO_PUSH_CAM t_camera;
    return 0;
}

int NetClientSession::ReciveData_GetParam(char* buffer ,int size)
{
  uint16 cmd;
	T_PacketHead   t_packet_head;
  if(size  < PACKET_HEAD_LEN )
  {
    printf("client recive buffer err!\n");
    return -1;
  }
  memcpy(&t_packet_head,buffer,PACKET_HEAD_LEN);
  cmd = t_packet_head.cmd;

	switch (cmd ){
		case SM_ANAY_VDCS_REGISTER:
         client_register_ack();
			   break;
    case SM_VDCS_ANAY_PUSH_CAMERA:
         push_camera_data(buffer+PACKET_HEAD_LEN,size-PACKET_HEAD_LEN);
			   break;
		default: break;
	}
	dbgprint("client cmd = %x\n", cmd);
	return 0;
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
