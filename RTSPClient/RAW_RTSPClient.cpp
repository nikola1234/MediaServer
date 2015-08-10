#include "stdafx.h"
#include "RAW_RTSPClient.h"
#include "OpenCamera.h"

CRAW_RTSPClient::CRAW_RTSPClient(COpenCamera *pcam)
		: m_popen_camera( pcam )
		,m_iStreamType(263)
		,m_iTimestampSeconds(0)
{
}

CRAW_RTSPClient::~CRAW_RTSPClient(void)
{
}

int CRAW_RTSPClient::startRTSPRequest(char* ip, int port, const char *url, const char* Range_clock, float scale)
{
	strcpy(this->m_url, url);

	int i = connectRtspSrv(ip,port);
	if( i < 0 )return i;

	i = sendSomeCommand(url);
	if(i < 0)return i;

	i = sendPlayCommand(url, scale, Range_clock);
	if( i < 0 )return i;

	i = RecvPlayResponse();
	if( i< 0 )return i;

	return i;
}

int CRAW_RTSPClient::sendSetupCommand(const char * url)
{
	char sendBuf[RTSPCOMMANDLEN];
	memset(sendBuf, 0, RTSPCOMMANDLEN);
	char * OptionsInfo = 
		"SETUP %s/track1 RTSP/1.0\r\n"
		"CSeq: %d\r\n"
		"Transport: RAW/RAW/TCP;unicast;interleaved=0\r\n"
		"User-Agent:%s\r\n"
		"\r\n";

	sprintf(sendBuf, OptionsInfo, url, m_icseq, m_user_agent);
	m_iCommandStatus = SETUPCOMMAND;
	return sendtoSvr(sendBuf,strlen(sendBuf));
	return 0;
}

void CRAW_RTSPClient::handleRecvVideoDataThread()
{
	boost::asio::socket_base::receive_buffer_size sendbufsize(65536*1024);
	rtsp_socket_.set_option(sendbufsize);

	try
	{	
		boost::shared_ptr<char> databuf = boost::shared_ptr<char>(new char[RAWDATALEN]);
		unsigned char* pdata = (unsigned char *)databuf.get();

		boost::system::error_code ec;
		int iSign = 0,iLen = 0,iStreamType = 0;

		while(m_brun){
			if( m_iCommandStatus == PLAYCOMMANDED )break;
			boost::this_thread::sleep( boost::posix_time::milliseconds(100) );
		}

		int ipos = 0;
		while(m_brun)
		{
			int icur = 0;
			while( icur < (4 - ipos) ) {
				int iread = rtsp_socket_.read_some( boost::asio::buffer(
					( char* )(pdata + ipos + icur), 4 - ipos - icur) , ec );
				if( ec )return;
				icur += iread;
			}

			ipos = 0;
			if ( ( pdata[0] == 0xAB || pdata[0] == 0xAA ) &&
				pdata[1] == 0xAA && pdata[2] == 0x00 && pdata[3] == 0x00 )
			{
				int len = 8;
				icur = 0;
				while( icur < len ){
					int ireed = rtsp_socket_.read_some( boost::asio::buffer(
						( char* )pdata + 4 + icur, len - icur ) , ec );
					if( ec )return;
					icur += ireed;
				}

				memcpy(&iSign,pdata,sizeof(int));
				memcpy(&iLen,pdata + 4,sizeof(int));
				memcpy(&iStreamType,pdata + 8,sizeof(int));

				int timestamp = iStreamType;

				if( iSign == 0xAAAB)
				{
					m_iStreamType = iStreamType&0x0000ffff;
					iStreamType = (m_iStreamType&0x0000ffff) + ( 1 << 16 );
					timestamp = m_iTimestampSeconds;
				}
				else
				{
					m_iTimestampSeconds = timestamp;
					iStreamType = m_iStreamType&0x0000ffff;
				}

				icur = 0;
				while( icur < iLen  ) {
					int iread = rtsp_socket_.read_some( boost::asio::buffer(
						( char* )(pdata + 12 + icur), iLen - icur ), ec );
					if( ec )return;
					icur += iread;
				}
				m_popen_camera->RecvData( timestamp, iStreamType, pdata + 12, iLen );
			}
			else
			{
				icur = 0;
				while(true)
				{
					if (pdata[0 + icur] == '\r' && pdata[1 + icur] == '\n' && 
						pdata[2 + icur] == '\r' && pdata[3 + icur] == '\n' )
					{
						//VNMP todo
						int newBytesRead = icur + 4;
						memcpy((char*)&fResponseBuffer[fResponseBytesAlreadySeen], pdata, newBytesRead);
						int i = handleResponseBytes(newBytesRead);
						if( i != NOEnoughData )
							break;
						break;
					}

					int ireed = 0;
					while(ireed < 1){
						ireed = rtsp_socket_.read_some( boost::asio::buffer(
							( char* )pdata + 4 + icur, 1) , ec);
						if( ec )return;
					}
					icur++;

					if( icur + 4 + 4 >= RAWDATALEN )break;
				}
			}
		}
	}
	catch (std::exception& e)
	{
		//g_MSNetSDKlog.Add("CRAW_RTSPClient::handleRecvVideoDataThread catch error = %s", e.what());
		//std::cerr << e.what() << std::endl;
	}
}

