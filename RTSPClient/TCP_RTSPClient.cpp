#include "stdafx.h"
#include "TCP_RTSPClient.h"
#include "OpenCamera.h"

CTCP_RTSPClient::CTCP_RTSPClient(COpenCamera *pcam)
	: m_popen_camera( pcam ),
	myrtpexternalsender(NULL),
	rtpexternaltransinfo(NULL)
{
	myrtpexternalsender = new MyRTPExternalSender(this);
}


CTCP_RTSPClient::~CTCP_RTSPClient(void)
{
	if(myrtpexternalsender)
		delete myrtpexternalsender;

	if (rtpexternaltransinfo)
		delete rtpexternaltransinfo;
}

int CTCP_RTSPClient::startRTSPRequest(char* ip ,int port ,const char *url, const char* Range_clock, float scale)
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

int CTCP_RTSPClient::sendSetupCommand(const char * url)
{
	char sendBuf[RTSPCOMMANDLEN];
	memset(sendBuf, 0, RTSPCOMMANDLEN);
	char * OptionsInfo = 
		"SETUP %s/track1 RTSP/1.0\r\n"
		"CSeq: %d\r\n"
		"Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n"
		"User-Agent:%s\r\n"
		"\r\n";

	sprintf(sendBuf, OptionsInfo, url, m_icseq, m_user_agent);
	m_iCommandStatus = SETUPCOMMAND;
	return sendtoSvr(sendBuf,strlen(sendBuf));
	return 0;
}

void CTCP_RTSPClient::handleRecvVideoDataThread()
{
	try
	{
		int status = 0;
		jrtplib::RTPSessionParams sessionparams;
		sessionparams.SetOwnTimestampUnit(1.0/90000.0);
		sessionparams.SetMaximumPacketSize(RTPUDPEXTRANS_RTPMAXSIZE);

		jrtplib::RTPExternalTransmissionParams transparams(myrtpexternalsender,RTPUDPEXTRANS_HEADERSIZE);
		jrtplib::RTPSession rtpsession_;
		status = rtpsession_.Create(sessionparams,&transparams,jrtplib::RTPTransmitter::ExternalProto);
		if(status < 0)return;

		rtpsession_.SetDefaultPayloadType(RTP_PAYLOADTYPE);
		rtpsession_.SetDefaultMark(false);
		rtpsession_.SetDefaultTimestampIncrement(1);

		rtpexternaltransinfo = (jrtplib::RTPExternalTransmissionInfo *)rtpsession_.GetTransmissionInfo();

		boost::asio::ip::tcp::endpoint serverendpoint = rtsp_socket_.remote_endpoint();
		boost::asio::ip::address ipaddress = serverendpoint.address();
		unsigned short serverrtsp_port = serverendpoint.port();
		boost::system::error_code ec;
		std::string serveripaddr = ipaddress.to_string(ec);

		boost::shared_ptr<char> rtpdatabuf = boost::shared_ptr<char>(new char[RTPDATALEN]);
		unsigned char* prtpdata = (unsigned char *)rtpdatabuf.get();
		memset(prtpdata, 0 ,RTPDATALEN);

		boost::shared_ptr<char> rawdatabuf = boost::shared_ptr<char>(new char[RAWDATALEN]);
		unsigned char* prawdata = (unsigned char *)rawdatabuf.get();
		memset(prawdata, 0 ,RAWDATALEN);

		while(m_brun){
			if(m_iCommandStatus == PLAYCOMMANDED)break;
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		}

		int ipos = 0;
		int irecvcur = 0;
		while(m_brun)
		{
			int icur = 0;
			while( icur < (4 - ipos) ) {
				int iread = rtsp_socket_.read_some( boost::asio::buffer(
					( char* )(prtpdata + ipos + icur), 4 - ipos - icur) , ec);
				if( ec )return;
				icur += iread;
			}
			ipos = 0;

			if (prtpdata[0] == '$' && ( prtpdata[1] == 0x00 || prtpdata[1] == 0x01 ) )
			{
				int len = prtpdata[2] * 256 + prtpdata[3];
				icur = 0;
				while( icur < len ){
					int ireed = rtsp_socket_.read_some( boost::asio::buffer(
						( char* )prtpdata + 4 + icur, len - icur) , ec);
					if( ec )return;
					icur += ireed;
				}

				jrtplib::RTPIPv4Address ipv4addr;
				ipv4addr.SetIP(boost::asio::ip::address_v4::from_string(serveripaddr).to_ulong());
				ipv4addr.SetPort(serverrtsp_port);
				if(prtpdata[1] == 0x00)
				{
					rtpexternaltransinfo->GetPacketInjector()->InjectRTP((const void*) (prtpdata + 4), len, ipv4addr);
				}
				else if (prtpdata[1] == 0x01)
				{
					rtpexternaltransinfo->GetPacketInjector()->InjectRTCP((const void*) (prtpdata + 4), len, ipv4addr);
				}
				//memset(prtpdata, 0 ,RTPDATALEN);
			}
			else
			{
				icur = 0;
				while(true)
				{
					if (prtpdata[0 + icur] == '\r' && prtpdata[1 + icur] == '\n' && 
						prtpdata[2 + icur] == '\r' && prtpdata[3 + icur] == '\n' )
					{
						//todo
						int newBytesRead = icur + 4;
						memcpy((char*)&fResponseBuffer[fResponseBytesAlreadySeen], prtpdata, newBytesRead);
						int i = handleResponseBytes(newBytesRead);
						if( i != NOEnoughData )
							break;
					}

					int ireed = 0;
					while(ireed < 1){
						ireed = rtsp_socket_.read_some( boost::asio::buffer(
							( char* )prtpdata + 4 + icur, 1) , ec);
						if( ec )return;
					}
					icur++;

					if(icur + 4 + 4 >= RTPDATALEN){
						break;
					}
				}
			}
			if(!m_brun)break;
			unsigned long dwDataType = 0;
			rtpsession_.BeginDataAccess();
			// check incoming packets
			if (rtpsession_.GotoFirstSourceWithData())
			{
				do
				{
					jrtplib::RTPPacket *pack;
					while ((pack = rtpsession_.GetNextPacket()) != NULL)
					{
						int timestamp = pack->GetTimestamp();

						if(pack->GetExtensionLength() >= sizeof(int))
							memcpy(&dwDataType, pack->GetExtensionData(), sizeof(int));
						if(pack->GetExtensionLength() >= sizeof(int)*2)
							memcpy(&timestamp, pack->GetExtensionData() + sizeof(int), sizeof(int));

						if( irecvcur + pack->GetPayloadLength() >= RAWDATALEN){
							m_popen_camera->RecvData(timestamp, dwDataType, prawdata, irecvcur);
							irecvcur = 0;
						}

						memcpy(prawdata + irecvcur, pack->GetPayloadData(), pack->GetPayloadLength());
						irecvcur += pack->GetPayloadLength();

						if (pack->HasMarker()){
							m_popen_camera->RecvData(timestamp, dwDataType, prawdata, irecvcur);
							irecvcur = 0;
						}
						rtpsession_.DeletePacket(pack);
					}
				} while (rtpsession_.GotoNextSourceWithData());
			}
			rtpsession_.EndDataAccess();

			if(!m_brun)break;

			status = rtpsession_.Poll();
			if( status < 0 )
				break;
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

bool CTCP_RTSPClient::SendRTCP(unsigned char *data, size_t len)
{
	try{
		unsigned char* sendbuf = new unsigned char[len + 4];
		memcpy(sendbuf + 4,data,len);
		memcpy(sendbuf,"$",1);
		sendbuf[1] = 0x01;
		sendbuf[2] = len/256;
		sendbuf[3] = len%256;

		int icur = 0,reallen = len + 4;
		boost::system::error_code ec;
		while(icur < reallen){
			int byteswrite = rtsp_socket_.write_some( boost::asio::buffer(
				(char*)(sendbuf + icur), reallen - icur), ec);
			icur += byteswrite;
			if( ec ){
				delete[] sendbuf;
				return false;
			}
		}

		delete[] sendbuf;
		return true;
	}catch (std::exception& e){
		std::cerr << e.what() << std::endl;
	}
	return true;
}