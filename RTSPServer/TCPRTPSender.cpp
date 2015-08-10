#include "TCPRTPSender.h"
#include "RTSPClientSession.h"

namespace live555RTSP
{

CTCPRTPSender::CTCPRTPSender(live555RTSP::RTSPServer* ourserver)
	: CRTPSender(ourserver)
{
}

CTCPRTPSender::~CTCPRTPSender(void)
{
	Stop();
	fourserver->m_log.Add("CTCPRTPSender::~CTCPRTPSender foursessionid = %d", foursessionid);
}

int CTCPRTPSender::InitRTPPackSender(
	unsigned int oursessionid, char* chip,
	unsigned short clientRTPPort,
	unsigned short clientRTCPPort,
	unsigned short& serverRTPPort, // out
	unsigned short& serverRTCPPort)
{
	strcpy(clientipaddress, chip);
	foursessionid = oursessionid;

	rtspport = clientRTPPort;
	rtsp_client_ptr = fourserver->FindRTSPClientBySessionID(oursessionid);

	fourserver->m_log.Add("CTCPRTPSender::CTCPRTPSender foursessionid = %d", foursessionid);

	rtpsenderstate = INITED;
	return 0;
}

bool CTCPRTPSender::SendRTP(unsigned char *data, size_t len)
{
	if (stop_)return false;
	if(rtsp_client_ptr == NULL)return false;
	if(!rtsp_client_ptr->GetrtspSocket().is_open())return false;
	if(m_sendbufsize >= DATABUF_MAXSTZE)
	{
		boost::system::error_code err;
		rtsp_client_ptr->GetrtspSocket().close(err);
#ifdef WIN32
		//if( m_sendbufsize%50 == 0)
		fourserver->m_log.Add( "CRAWSender::RealSendData NetWork Congestion not send bufsize = %d", m_sendbufsize );
#endif
		return false;
	}

	unsigned char* sendbuf = new unsigned char[len + 4];
	memcpy(sendbuf + 4,data,len);
	memcpy(sendbuf,"$",1);
	sendbuf[1] = 0x00;
	sendbuf[2] = len/256;
	sendbuf[3] = len%256;

	try{
		rtsp_client_ptr->GetrtspSocket().async_send(
			boost::asio::buffer(sendbuf,len + 4),
			boost::bind(&CTCPRTPSender::HandleSendRTPData,shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred,
			sendbuf));
	}catch(...){ };

	m_sendbufsize += len + 4;
	return true;
}

bool CTCPRTPSender::SendRTCP(unsigned char *data, size_t len)
{
	if (stop_)return false;
	if(rtsp_client_ptr == NULL)return false;
	if(!rtsp_client_ptr->GetrtspSocket().is_open())return false;

	unsigned char* sendbuf = new unsigned char[len + 4];
	memcpy(sendbuf + 4,data,len);
	memcpy(sendbuf,"$",1);
	sendbuf[1] = 0x01;
	sendbuf[2] = len/256;
	sendbuf[3] = len%256;

	try{
	rtsp_client_ptr->GetrtspSocket().async_send(
		boost::asio::buffer(sendbuf,len + 4),
		boost::bind(&CTCPRTPSender::HandleSendRTCPData,shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred,
		sendbuf));
	}catch(...){ };

	m_sendbufsize += len + 4;
	return true;
}

void CTCPRTPSender::HandleSendRTPData(const boost::system::error_code& error, int sendsize, unsigned char* data)
{
	m_sendbufsize -= sendsize;
	delete[] data;
	if (stop_)return;
	if (!error){ }
	else{
		Stop();
		std::cerr << "CTCPRTPSender::HandleSendRTPData" << error.message() << std::endl;
	}
}

void CTCPRTPSender::HandleSendRTCPData(const boost::system::error_code& error, int sendsize, unsigned char* data)
{
	m_sendbufsize -= sendsize;
	delete[] data;
	if (stop_)return;
	if (!error){ }
	else{
		Stop();
		std::cerr << error.message() << std::endl;
	}
}

void CTCPRTPSender::StartRecvRTCP()
{
}

int CTCPRTPSender::RecvRTCPData(unsigned char *data, unsigned long len)
{
	if(!(rtpsenderstate == WAITFOR_SENDHAED || rtpsenderstate == WORKING))return -1;

	jrtplib::RTPIPv4Address ipv4addr;
	ipv4addr.SetIP(boost::asio::ip::address_v4::from_string(clientipaddress).to_ulong());
	ipv4addr.SetPort(rtspport);
	boost::recursive_mutex::scoped_lock lock_( rtpsession_mutex_ );
	rtpexternaltransinfo->GetPacketInjector()->InjectRTCP(data, len, ipv4addr);
	lock_.unlock();
	return 0;
}

void CTCPRTPSender::Stop()
{
	if (stop_)return;
	stop_ = true;

	rtpsenderstate = STOP;
}

}// end namespace