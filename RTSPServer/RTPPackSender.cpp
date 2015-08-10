#include "RTPPackSender.h"

namespace live555RTSP
{

UDPPortVector CUDPRTPSender::s_udpportvector;

CUDPRTPSender::CUDPRTPSender(live555RTSP::RTSPServer* ourserver)
	: CRTPSender(ourserver),
	rtp_socket_(ourserver->Get_io_service()),
	rtcp_socket_(ourserver->Get_io_service()),
	rtcpserverport(0),
    rtpserverport(0),
	rtcpclientport(0),
	rtpclientport(0)
{

}

CUDPRTPSender::~CUDPRTPSender(void)
{
	Stop();

	if(rtcpserverport)
		s_udpportvector.FreeUDPPort(rtcpserverport);

	fourserver->m_log.Add("CRTPPackSender::~CRTPPackSender foursessionid = %d",foursessionid);
}

int CUDPRTPSender::InitRTPPackSender(
	unsigned int oursessionid, char* chip,
	unsigned short clientRTPPort,
	unsigned short clientRTCPPort,
	unsigned short& serverRTPPort, // out
	unsigned short& serverRTCPPort)
{
	rtcpclientport = clientRTCPPort;
	rtpclientport = clientRTPPort;

	rtcpserverport = s_udpportvector.GetFreeUDPPort();
	rtpserverport = rtcpserverport - 1;

	if(rtcpserverport == 0)
		return -1;

	serverRTCPPort = rtcpserverport;
	serverRTPPort = rtpserverport;

	strcpy(clientipaddress, chip);
	foursessionid = oursessionid;

	boost::asio::ip::udp::endpoint rtcpendpoint(
		boost::asio::ip::udp::v4(), rtcpserverport);
	boost::asio::ip::udp::endpoint rtpendpoint(
		boost::asio::ip::udp::v4(), rtpserverport);

	fourserver->m_log.Add("CRTPPackSender::CRTPPackSender foursessionid = %d",foursessionid);

	try
	{
		rtcp_socket_.open(boost::asio::ip::udp::v4());
		rtp_socket_.open(boost::asio::ip::udp::v4());
		rtcp_socket_.bind(rtcpendpoint);
		rtp_socket_.bind(rtpendpoint);

		boost::asio::socket_base::send_buffer_size sendbufsize(65536*64);
		rtp_socket_.set_option(sendbufsize);
	}
	catch (std::exception& e)
	{
		fourserver->m_log.Add("CRTPPackSender::CRTPPackSender foursessionid = %d error = %s",e.what());
		return -1;
	}

	rtpsenderstate = INITED;
	return 0;
}

bool CUDPRTPSender::SendRTP(unsigned char *data, size_t len)
{
	if (stop_) return false;
	if (!rtp_socket_.is_open())return false;
	if(m_sendbufsize >= DATABUF_MAXSTZE)return false;

	unsigned char* sendbuf = new unsigned char[len];
	if( sendbuf == NULL ){
		//fourserver->m_log.Add("CUDPRTPSender::SendRTP new = null");
		return false;
	}
	memcpy(sendbuf,data,len);

	boost::asio::ip::udp::endpoint rtpendpoint(
		boost::asio::ip::address_v4::from_string(clientipaddress), rtpclientport);
	//rtp_socket_.async_send_to(
	//	boost::asio::buffer(sendbuf,len),rtpendpoint,
	//	boost::bind(&CUDPRTPSender::HandleSendRTPData,shared_from_this(),
	//	boost::asio::placeholders::error,
	//	boost::asio::placeholders::bytes_transferred,
	//	sendbuf));

	boost::unique_lock<boost::mutex> lock_(m_queque_mutex);
	if( m_willSendBufFifo.empty() )
	{
		lock_.unlock();
		try{
			rtp_socket_.async_send_to(
				boost::asio::buffer(sendbuf,len),rtpendpoint,
				boost::bind(&CUDPRTPSender::HandleSendRTPData, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred,
				sendbuf));
		}catch(...){};
	}
	else
	{
		m_willSendBufFifo.push( boost::asio::buffer(sendbuf, len) );
		lock_.unlock();
	}

	m_sendbufsize += len;
	return true;
}

bool CUDPRTPSender::SendRTCP(unsigned char *data, size_t len)
{
	if (stop_)return false;
	if (!rtcp_socket_.is_open())return false;

	unsigned char* sendbuf = new unsigned char[len];
	memcpy(sendbuf,data,len);

	boost::asio::ip::udp::endpoint rtcpendpoint(
		boost::asio::ip::address_v4::from_string(clientipaddress), rtcpclientport);
	rtcp_socket_.async_send_to(
		boost::asio::buffer(sendbuf,len),rtcpendpoint,
		boost::bind(&CUDPRTPSender::HandleSendRTCPData,shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred,
		sendbuf));

	m_sendbufsize += len;
	return true;
}

void CUDPRTPSender::StartRecvRTCP()
{
	boost::asio::ip::udp::endpoint rtcpclientendpoint(
		boost::asio::ip::address_v4::from_string(clientipaddress), rtcpclientport);

	rtcp_socket_.async_receive_from(
		boost::asio::buffer(rtcprecvdata,RTCP_DATASTZE),rtcpclientendpoint,
		boost::bind(&CUDPRTPSender::HandleRecvRTCPData, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void CUDPRTPSender::HandleSendRTPData(const boost::system::error_code& error, int sendsize, unsigned char* data)
{
	m_sendbufsize -= sendsize;
	delete[] data;

	if ( stop_ )return;
	if ( !error ){
		boost::unique_lock<boost::mutex> lock_(m_queque_mutex);
		if( !m_willSendBufFifo.empty() )
		{
			boost::asio::mutable_buffers_1 boost_buf = m_willSendBufFifo.front();
			m_willSendBufFifo.pop();
			lock_.unlock();
			try{
				boost::asio::ip::udp::endpoint rtpendpoint(
					boost::asio::ip::address_v4::from_string(clientipaddress), rtpclientport);
				rtp_socket_.async_send_to(
					boost_buf,rtpendpoint,
					boost::bind(&CUDPRTPSender::HandleSendRTPData,shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					boost::asio::buffer_cast<unsigned char*>(boost_buf)));
			}catch(...){};
		}
	}
	else{
		Stop();
		//std::cerr << "CRTPPackSender::HandleSendRTPData" << error.message() << std::endl;
	}
}

void CUDPRTPSender::HandleSendRTCPData(const boost::system::error_code& error, int sendsize,unsigned char* data)
{
	m_sendbufsize -= sendsize;
	delete[] data;

	if (stop_)return;
	if (!error){ }
	else{
		Stop();
		//std::cerr << error.message() << std::endl;
	}
}

void CUDPRTPSender::HandleRecvRTCPData(const boost::system::error_code& error, int recvsizes)
{
	if (stop_)return;
	if (!error)
	{
		jrtplib::RTPIPv4Address ipv4addr;
		ipv4addr.SetIP(boost::asio::ip::address_v4::from_string(clientipaddress).to_ulong());
		ipv4addr.SetPort(rtcpclientport);

		boost::recursive_mutex::scoped_lock lock_(rtpsession_mutex_);
		rtpexternaltransinfo->GetPacketInjector()->InjectRTCP(rtcprecvdata,recvsizes,ipv4addr);
		lock_.unlock();

		StartRecvRTCP();
	}
	else{
		Stop();
	}
}

int CUDPRTPSender::RecvRTCPData(unsigned char *data, unsigned long len)
{
	return 0;
}

void CUDPRTPSender::Stop()
{
	if (stop_)return;
	stop_ = true;

	boost::system::error_code err;
	rtp_socket_.close(err);
	rtcp_socket_.close(err);
	rtpsenderstate = STOP;

	boost::unique_lock<boost::mutex> lock_(m_queque_mutex);
	while( !m_willSendBufFifo.empty() ) {
		boost::asio::mutable_buffers_1 boost_buf = m_willSendBufFifo.front();
		m_willSendBufFifo.pop();
		unsigned char* p = boost::asio::buffer_cast<unsigned char*>(boost_buf);
		delete[] p;
	}
	lock_.unlock();
}

}// end namespace