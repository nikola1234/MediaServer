#include "NetClient.h"
#include "boost/filesystem.hpp"
#include "FileOpr.h"

/*
	bug: 连接服务器超时后重连异常，思路:改成异步连接
*/
extern T_ServerParam SerParam;

NetClient::NetClient(void)
	:tcp_socket_(io_service_),timer1_(io_service_, boost::posix_time::minutes(10))
{
	m_isConnected = false;
	m_checkConnect = true;

	memset(m_hostname, 0, IP_LEN_16);
	memset(m_port, 0 ,PORT_LEN_10);
	
	timer1_.async_wait(boost::bind(&NetClient::RegularEvent, this)); 
}

NetClient::~NetClient(void)
{
	m_checkConnect = false;

	try
	{
		boost::system::error_code err;
		tcp_socket_.close(err);
		io_service_.stop();
	} catch (...){ }

}

void NetClient::CloseSocket(void)
{
	try
	{
		boost::system::error_code err;
		tcp_socket_.close(err);

	} catch (...){ }
}

void NetClient::start(char *ip ,uint32 port)
{
	memcpy(m_hostname , ip , strlen(ip));
    sprintf(m_port,"%d",port); 

	boost::filesystem::create_directory("log");
	m_log.InitLog("./log/NC-");
	m_log.Add("NetClient Start !");
	ConnectingtoSvr();
}

bool NetClient::IsConnected(void)
{
	bool conn;
	readLock readlock_(m_ConnStatusMutex_);
	conn =  m_isConnected;
	readlock_.unlock();
	return conn;
}

void NetClient::ChangeConnect(bool staus)
{
	writeLock writelock_(m_ConnStatusMutex_);
	m_isConnected = staus;
	writelock_.unlock();
}

int NetClient::ConnectingtoSvr(void)
{
/*
	boost::asio::ip::tcp::resolver resolver(io_service_);	 
	boost::asio::ip::tcp::resolver::query query(m_hostname, m_port);
	boost::asio::ip::tcp::resolver::iterator itr_endpoint = resolver.resolve(query);
	boost::asio::async_connect(tcp_socket_, itr_endpoint, boost::bind(&NetClient::HandleConnect,this,boost::asio::placeholders::error) );
*/

	boost::asio::ip::tcp::resolver resolver(io_service_);	
	boost::asio::ip::tcp::resolver::query query(m_hostname, m_port);
	boost::asio::ip::tcp::resolver::iterator itr_endpoint = resolver.resolve(query);

	boost::asio::ip::tcp::resolver::iterator itr_end; //无参数构造生成end迭代器
	boost::system::error_code ec =  boost::asio::error::host_not_found;
	for(;ec && itr_endpoint!=itr_end;++itr_endpoint)
	{
        tcp_socket_.close();
        tcp_socket_.connect(*itr_endpoint, ec);
	}
	
	if(ec)
	{
        m_log.Add( boost::system::system_error(ec).what());
		m_isConnected = false;	
		CloseSocket();
		ConnectingtoSvr();
		printf("ConnectingtoSvr again!\n");
        return -1;
	}
	ChangeConnect(true);
	m_log.Add("connect sucess!");
	return 0;
}
/*
void NetClient::HandleConnect(const boost::system::error_code& error) 
{ 
	printf("HandleConnect \n");
	if(!error) 
	{ 
		m_isConnected = true;
	  	SendToServer("hello!",7);
		printf("send\n");
	}
	else 
	{ 
		printf("connect err\n");
	} 
} 
*/
int NetClient::Send(char *buf, int len)
{
	int nSumLen = len;
	int nRet = 0,nCur = 0;

	try
	{
		while(nCur < nSumLen)
		{
			nRet = tcp_socket_.send( boost::asio::buffer( (char *)buf + nCur, nSumLen - nCur ) );
			printf("have send is %d\n",nRet);
			if(nRet <= 0)return -1;
			nCur += nRet;
		}
		return len;
	}
	catch (std::exception& e)
	{
		m_log.Add(e.what());
		ChangeConnect(false);
		return -1;
	}
	return 0;
}

int NetClient::SendToServer(char *buf, int len)
{
	int iRet = -1;
	if(IsConnected() ==  true)
	{
		iRet = Send(buf,len);
		if(iRet ==	-1)
		{	
			ChangeConnect(false);
			m_log.Add("disconnect");
			return -1;
		}
	}
	/*
	else
	{
		m_log.Add("no send to server because of disconnect");
		return -1;
	}*/
	
	return 0;
}

/*异步发送
void NetClient::Send(char* buffer, int length)
{
	tcp_socket_.async_send(boost::asio::buffer(buffer, length), boost::bind(&Connection::handler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}
void NetClient::handler(const boost::system::error_code& error, std::size_t bytes_transferred ){}
*/

void NetClient::CheckConnect(void)
{
	int flag = 1;
	while(m_checkConnect)
	{
		if((IsConnected()== false)&&(flag == 1))
		{
			flag =0;
			ConnectingtoSvr();
			flag =1;	
		}
		sleep(2);
	}
}

void NetClient::SendPicToServer(void)
{
	int iRet = -1;
	int i = 0;
	uint8 CamNum = 0;
	T_PacketHead  t_PicPacketHead;

	CamNum = m_camMana->Get_Rest_SingleCamlist_num();

	t_PicPacketHead.magic	   		 =  T_PACKETHEAD_MAGIC;
	t_PicPacketHead.cmd			  	 =  SM_ANAY_ALARM_HEATBEAT;
	t_PicPacketHead.UnEncryptLen	 =  sizeof(T_ANAY_ALARM_HEATBEAT)+ CamNum *sizeof(T_ANAY_ALARM_CAM_PIC);

	int msgSize = PACKET_HEAD_LEN +sizeof(T_ANAY_ALARM_HEATBEAT)+ CamNum *sizeof(T_ANAY_ALARM_CAM_PIC);
	char * buff = (char*)malloc( msgSize );

	memcpy(buff ,&t_PicPacketHead,PACKET_HEAD_LEN);
	
	T_ANAY_ALARM_HEATBEAT t_heart;
	
	t_heart.AnaServerID = SerParam.ServerID;
	memcpy(t_heart.AnaServerIp,SerParam.ServerIp ,16);
	t_heart.CamNum = CamNum;
	memcpy(buff+PACKET_HEAD_LEN ,&t_heart,sizeof(T_ANAY_ALARM_HEATBEAT));

	for(i = 0; i < CamNum; i++)
	{
		T_ANAY_ALARM_CAM_PIC t_camPic;
		m_camMana->Get_Pic_SingleCamlist_num(&t_camPic, i);
		memcpy(buff+PACKET_HEAD_LEN+sizeof(T_ANAY_ALARM_HEATBEAT)+i * sizeof(T_ANAY_ALARM_CAM_PIC),&t_camPic,sizeof(T_ANAY_ALARM_CAM_PIC) );
	}

	SendToServer(buff,msgSize);
	//SendToServer("hello!",7);
	free(buff);
}

void NetClient::RegularEvent(void)
{
	if(m_checkConnect)
	{
	   SendPicToServer();
	   timer1_.expires_at(timer1_.expires_at() + boost::posix_time::minutes(10));  //minutes
	   timer1_.async_wait(boost::bind(&NetClient::RegularEvent, this)); 
	   m_log.Add("RegularEvent");
	}
}

void NetClient::Run(void)
{

  m_clientThread_  =
  		(boost::shared_ptr<boost::thread>)new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_)); 

  m_connectThread_ = 
  		(boost::shared_ptr<boost::thread>)new boost::thread(boost::bind(&NetClient::CheckConnect,this)); 
}


