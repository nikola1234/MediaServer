#include "RAWSender.h"
#include "RTSPClientSession.h"
#include "RTSPClient.h"

namespace live555RTSP
{

CRAWSender::CRAWSender(live555RTSP::RTSPServer* ourserver)
	: fourserver(ourserver),
	rtpsenderstate(CREATE),
	foursessionid(0),
	stop_(false)
{
}

CRAWSender::~CRAWSender(void)
{
	fourserver->m_log.Add("CRAWSender::~CRAWSender foursessionid = %d",foursessionid);
}

int CRAWSender::InitRTPPackSender(
	unsigned int oursessionid, char* chip,
	unsigned short clientRTPPort,
	unsigned short clientRTCPPort,
	unsigned short& serverRTPPort, // out
	unsigned short& serverRTCPPort) // out
{
	foursessionid = oursessionid;
	
	rtsp_client_ptr = fourserver->FindRTSPClientBySessionID(oursessionid);
	fourserver->m_log.Add("CRAWSender::CRAWSender foursessionid = %d", foursessionid);

	rtpsenderstate = INITED;

	m_iH264BufCurSize = 0;
	return 0;
}

int CRAWSender::Start()
{
	rtpsenderstate = START;

	boost::asio::socket_base::send_buffer_size sendbufsize(65536);
	boost::asio::socket_base::keep_alive keepalive(true);
	if(rtsp_client_ptr == NULL)return -1;
	rtsp_client_ptr->GetrtspSocket().set_option(sendbufsize);
	rtsp_client_ptr->GetrtspSocket().set_option(keepalive);
	return 0;
}

int CRAWSender::RealSendData(unsigned long dwDataType, unsigned char *data, unsigned long len, int timestampinc)
{
	if (stop_)return -1;
	if(rtsp_client_ptr == NULL)return -1;
	if(!rtsp_client_ptr->GetrtspSocket().is_open())return -1;
	if(!(rtpsenderstate == WAITFOR_SENDHAED || rtpsenderstate == WORKING))return -2;
	if(m_sendbufsize >= DATABUF_MAXSTZE ){
//		boost::system::error_code err;
//		rtsp_client_ptr->GetrtspSocket().close(err);
		//boost::this_thread::sleep( boost::posix_time::milliseconds( 100 ) );会堵塞视频数据回调函数
#ifdef WIN32
		if( ( !rtsp_client_ptr->GetIsDownLoad() ) && ( m_sendbufsize%50 == 0 ))
			fourserver->m_log.Add( "CRAWSender::RealSendData NetWork Congestion not send bufsize = %d", m_sendbufsize );
#endif
		if( !rtsp_client_ptr->GetIsDownLoad() )
		{
			return false;
		} else{
			boost::this_thread::sleep( boost::posix_time::milliseconds( 100 ) ); //阻塞下载数据
		}
	}

	unsigned int head = 0xAAAA;
	if( ( dwDataType >> 16 )  == 0x0001 ){
		head = 0xAAAB;
		rtpsenderstate = WORKING;
	}

	unsigned char* sendbuf = NULL;
	int sendlen = 0;
	int devtype = dwDataType & 0xffff;
	int reallen = len;

	if(devtype <= H264_MAX_DEVTYPE)
	{
		//dwDataType = 2563;
		//sendbuf = new unsigned char[len + 12 + 14];

		//int wh = 3520288;
		//memcpy(sendbuf + 12,"ITSSKY",6);
		//memcpy(sendbuf + 12 + 6,&wh,sizeof(int));
		//memcpy(sendbuf + 12 + 10,"H264",4);
		//memcpy(sendbuf + 12 + 14, data, len);

		//sendlen = len + 12 + 14;
		//reallen = len + 14;

		devtype = 20483;
	}

	unsigned int type = devtype; // dwDataType & 0xffff + timestampinc << 16;

	sendbuf = new unsigned char[len + 12];
	memcpy(sendbuf + 12, data, len);

	sendlen = len + 12;
	reallen = len;

	if( timestampinc != -1 && head != 0xAAAB ) {
		type = timestampinc;
	}

	memcpy(sendbuf, &head, sizeof(int));
	memcpy(sendbuf + 4, &reallen, sizeof(int));
	memcpy(sendbuf + 8, &type, sizeof(int));

#ifdef WIN32
	try{
	rtsp_client_ptr->GetrtspSocket().async_send(
		boost::asio::buffer(sendbuf, sendlen),
		boost::bind(&CRAWSender::HandleSendData, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred,
		sendbuf));
	}catch(...){};
#else
	boost::unique_lock<boost::mutex> lock_(m_queque_mutex);
	if( m_willSendBufFifo.empty() )
	{
		lock_.unlock();
		try{
			boost::asio::async_write(
				rtsp_client_ptr->GetrtspSocket(),
				boost::asio::buffer(sendbuf, sendlen),
				boost::bind(&CRAWSender::HandleSendData, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred,
				sendbuf));
		}catch(...){};
	}
	else
	{
		m_willSendBufFifo.push( boost::asio::buffer(sendbuf, sendlen) );
		lock_.unlock();
	}
#endif

	m_sendbufsize += sendlen;
	return 0;
}

int CRAWSender::SendData(unsigned long dwDataType, unsigned char *s_data, unsigned long s_len, int timestampinc)
{
	int devtype = dwDataType & 0xffff;
	if( devtype <= H264_MAX_DEVTYPE )
	{
		unsigned char *data = s_data + 12;
		unsigned long len = s_len - 12;

		if( m_pH264PackBuf == NULL )
			m_pH264PackBuf = boost::shared_ptr<char>( new char[RAWDATALEN] );
		unsigned char * pBuf = (unsigned char *)m_pH264PackBuf.get();

		//char debuginfo[1024];
		//sprintf(debuginfo, "CRAWSender::SendData type = %d s_bit = %d e_bit = %d \n", data[0] & 0x1f, data[1]&0x80 ,data[1]&0x40 );
		//OutputDebugString(debuginfo);

		if( (data[0]&0x1f) != 28 )
		{
			pBuf[0] = 0x00; pBuf[1] = 0x00;
			pBuf[2] = 0x00; pBuf[3] = 0x01;
			memcpy(pBuf + 4, data, len );
			return RealSendData( dwDataType, pBuf, len + 4, timestampinc );
		}
		else
		{
			if( ( (data[1]&0x80) > 0 ) && ( (data[1]&0x40) == 0 ) )
			{
				m_iH264BufCurSize = 0;

				pBuf[0] = 0x00; pBuf[1] = 0x00;
				pBuf[2] = 0x00; pBuf[3] = 0x01;
				pBuf[4] = (data[0] & 0xe0) | (data[1] & 0x1f);
				memcpy(pBuf + 5, data + 2, len - 2);
				m_iH264BufCurSize += len + 3;
			}
			else if( ( (data[1]&0x80) == 0 ) && ( (data[1]&0x40) == 0 ) && m_iH264BufCurSize > 0 )
			{
				if( m_iH264BufCurSize + len - 2 > RAWDATALEN ){
					int nRet = RealSendData( dwDataType, pBuf, m_iH264BufCurSize, timestampinc );
					m_iH264BufCurSize = 0;
				}
				memcpy( pBuf + m_iH264BufCurSize, data + 2, len - 2 );
				m_iH264BufCurSize += len - 2;
			}
			else if( ( (data[1]&0x80) == 0 ) && ( (data[1]&0x40) > 1 ) && m_iH264BufCurSize > 0 )
			{
				if( m_iH264BufCurSize + len - 2 > RAWDATALEN ){
					int nRet = RealSendData( dwDataType, pBuf, m_iH264BufCurSize, timestampinc );
					m_iH264BufCurSize = 0;
				}
				memcpy( pBuf + m_iH264BufCurSize, data + 2, len - 2 );
				m_iH264BufCurSize += len - 2;
				int nRet = RealSendData( dwDataType, pBuf, m_iH264BufCurSize, timestampinc );
				m_iH264BufCurSize = 0;
				return nRet;
			}
		}
	}
	else
	{
		return RealSendData( dwDataType, s_data, s_len, timestampinc );
	}
	return 0;
}

void CRAWSender::HandleSendData(const boost::system::error_code& error, int sendsize, unsigned char *data)
{
	m_sendbufsize -= sendsize;
	delete[] data;

	if ( stop_ )return;
	if (!error){
#ifndef WIN32
		boost::unique_lock<boost::mutex> lock_(m_queque_mutex);
		if( !m_willSendBufFifo.empty() )
		{
			boost::asio::mutable_buffers_1 boost_buf = m_willSendBufFifo.front();
			m_willSendBufFifo.pop();
			lock_.unlock();
			try{
				boost::asio::async_write(
					rtsp_client_ptr->GetrtspSocket(),
					boost_buf,
					boost::bind(&CRAWSender::HandleSendData, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred,
					boost::asio::buffer_cast<unsigned char*>(boost_buf)));
			}catch(...){};
		}
#endif
	}
	else{
		Stop();
		std::cerr << "CRAWSender::HandleSendData" << error.message() << std::endl;
	}
}

void CRAWSender::Stop()
{
	if (stop_)return;
	stop_ = true;
	rtpsenderstate = STOP;
}

}//end namespace