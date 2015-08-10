#include "stdafx.h"
#include "RAW_RTSPClient.h"
#include "CameraRecord.h"

namespace MediaSave
{

CRAW_RTSPClient::CRAW_RTSPClient(CCameraRecord *pcam, int tokenType)
		: m_camera_record( pcam ),
		m_tokenType(tokenType),
		m_iStreamType(263)
{
	//memset(m_user_agent, 0, 256);
	//strcpy(m_user_agent, "MediaSave Client");
	sprintf(m_user_agent, "VNMPNetSDK V1.0 %02d %d", tokenType, m_sClientID);
}

CRAW_RTSPClient::~CRAW_RTSPClient(void)
{
}

int CRAW_RTSPClient::startRTSPRequest(char* ip ,int port ,const char *url, const char* Range_clock, float scale)
{
	strcpy(this->m_url, url);

	int i = connectRtspSrv( ip, port );
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
	try
	{	
		boost::shared_ptr<char> databuf = boost::shared_ptr<char>(new char[RAWDATALEN]);
		unsigned char* pdata = (unsigned char *)databuf.get();

		boost::system::error_code ec;
		int iSign = 0, iLen = 0, iStreamType = 0;

		while(m_brun){
			if(m_iCommandStatus == PLAYCOMMANDED)break;
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		}

		int ipos = 0;
		while(true)
		{
			int icur = 0;
			while( icur < (4 - ipos) ) {
				int iread = rtsp_socket_.read_some( boost::asio::buffer(
					( char* )(pdata + ipos + icur), 4 - ipos - icur) , ec);
				if( ec ) { m_camera_record->SetIsBadSocket(true); return; }
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
						( char* )pdata + 4 + icur, len - icur) , ec);
					if( ec ) { m_camera_record->SetIsBadSocket(true); return; }
					icur += ireed;
				}

				memcpy(&iSign, pdata,sizeof(int));
				memcpy(&iLen, pdata + 4,sizeof(int));
				memcpy(&iStreamType, pdata + 8,sizeof(int));

				if( iSign == 0xAAAB )
					m_iStreamType = iStreamType&0x0000ffff;

				unsigned int _clock = clock();
				_clock = (_clock << 16)&0xFFFF0000;
				int iType = (m_iStreamType&0x0000ffff)|_clock;
				memcpy(pdata + 8, &iType, sizeof(int));

				if( iSign == 0xAAAB )
					iStreamType = ( m_iStreamType&0x0000ffff ) + ( 1 << 16 );
				else
					iStreamType = m_iStreamType&0x0000ffff;

				icur = 0;
				while( icur < iLen  ) {
					int iread = rtsp_socket_.read_some( boost::asio::buffer(
						( char* )(pdata + 12 + icur), iLen - icur) , ec);
					if( ec ) { m_camera_record->SetIsBadSocket(true); return; }
					icur += iread;
				}
				m_camera_record->RecordData( pdata, iLen + 12, iStreamType );
			}
			else
			{
				icur = 0;
				while(true)
				{
					if (pdata[0 + icur] == '\r' && pdata[1 + icur] == '\n' && 
						pdata[2 + icur] == '\r' && pdata[3 + icur] == '\n' )
					{
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
						if( ec ) { m_camera_record->SetIsBadSocket(true); return; }
					}
					icur++;

					if(icur + 4 + 4 >= RAWDATALEN)break;
				}
			}
		}
	}
	catch (std::exception& e)
	{
		m_camera_record->SetIsBadSocket(true);
		std::cerr << e.what() << std::endl;
	}
}

}// end namespace