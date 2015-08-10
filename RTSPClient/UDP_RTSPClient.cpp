#include "stdafx.h"
#include "UDP_RTSPClient.h"
#include "OpenCamera.h"

#include "rtpsession.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtppacket.h"

#ifndef WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif // WIN32

#include "h264_rtp_unpack.h"

live555RTSP::UDPPortVector CUDP_RTSPClient::s_udpportvector;

CUDP_RTSPClient::CUDP_RTSPClient(COpenCamera *pcam)
		: m_popen_camera( pcam ),
		m_clinet_video_rtpport(0),
		m_clinet_audio_rtpport(0)
{
	memset(m_realConnectIP, 0, 256);
}


CUDP_RTSPClient::~CUDP_RTSPClient(void)
{
	if(m_clinet_video_rtpport)
		s_udpportvector.FreeUDPPort(m_clinet_video_rtpport + 1);

	if(m_clinet_audio_rtpport)
		s_udpportvector.FreeUDPPort(m_clinet_audio_rtpport + 1);

	Heartbeat_thread_.join();
}


int CUDP_RTSPClient::startRTSPRequest(char* ip ,int port ,const char *url, const char* Range_clock, float scale)
{
	strcpy(this->m_url, url);
	strcpy(this->m_realConnectIP, ip);

	int i = connectRtspSrv(ip,port);
	if( i < 0 )return i;

	i = sendSomeCommand(url);
	if(i < 0)return i;

	i = sendPlayCommand(url, scale, Range_clock);
	if( i < 0 )return i;

	i = HandleIncomingData();
	if( i < 0 )return i;

	if( i >= 0 )
		Heartbeat_thread_ = boost::thread(&CUDP_RTSPClient::handleHeartbeatThread , this);

	return i;
}

int CUDP_RTSPClient::sendSetupCommand(const char * url)
{
	char* authenticatorStr = createAuthenticatorString("SETUP", url);

	char sendBuf[RTSPCOMMANDLEN];
	memset(sendBuf, 0, RTSPCOMMANDLEN);
	std::string strSession = m_SessionID;
	if( strSession.length() > 0 ){
		strSession = "Session: ";
		strSession += m_SessionID;
		strSession += "\r\n";
	}

	char * OptionsInfo = 
		"SETUP %s RTSP/1.0\r\n"
		"CSeq: %d\r\n"
		"%s"
		"Transport: RTP/AVP;unicast;client_port=%d-%d\r\n"
		"%s"
		"User-Agent:%s\r\n"
		"\r\n";

	int iClientRTPPort = 0;
	iClientRTPPort = s_udpportvector.GetFreeUDPPort() - 1;

	if( m_bSendingVideoSETUP )
		m_clinet_video_rtpport = iClientRTPPort;
	if( m_bSendingAudioSETUP )
		m_clinet_audio_rtpport = iClientRTPPort;

	sprintf(sendBuf, OptionsInfo, url, m_icseq, authenticatorStr/*m_strAuthenticator.c_str()*/, iClientRTPPort, iClientRTPPort + 1, strSession.c_str(), m_user_agent);

	delete[] authenticatorStr;

	m_iCommandStatus = SETUPCOMMAND;
	return sendtoSvr( sendBuf, strlen( sendBuf ) );
	return 0;
}

void CUDP_RTSPClient::handleRecvVideoDataThread()
{
	bool bAudio = false;

	jrtplib::RTPSession sess;
	uint16_t portbase,destport;
	uint32_t destip;
	std::string ipstr;
	int status;

	// First, we'll ask for the necessary information
	portbase = m_clinet_video_rtpport;

	if( strlen( m_SrvAddressStr ) > 0 )
		destip = inet_addr( m_SrvAddressStr );
	else
		destip = inet_addr( m_realConnectIP );

	if ( destip == INADDR_NONE ){
		std::cerr << "Bad IP address specified" << std::endl;
		return;
	}

	// The inet_addr function returns a value in network byte order, but
	// we need the IP address in host byte order, so we use a call to
	// ntohl
	destip = ntohl(destip);

	if( !m_bVideoThreadRun )
	{
		destport = m_SrvVideoUDPPort;
		m_bVideoThreadRun = true;
		if( ( !m_recvAudioThread_.joinable() ) && m_SrvAudioUDPPort != 0 ) 
			m_recvAudioThread_ = boost::thread( &CRTSPClient::handleRecvVideoDataThread, this );
	}
	else
	{
		destport = m_SrvAudioUDPPort;
		portbase = m_clinet_audio_rtpport;

		bAudio = true;
	}

	// Now, we'll create a RTP session, set the destination, send some
	// packets and poll for incoming data.
	jrtplib::RTPUDPv4TransmissionParams transparams;
	jrtplib::RTPSessionParams sessparams;

	// IMPORTANT: The local timestamp unit MUST be set, otherwise
	//            RTCP Sender Report info will be calculated wrong
	// In this case, we'll be sending 10 samples each second, so we'll
	// put the timestamp unit to (1.0/10.0)
	sessparams.SetOwnTimestampUnit(1.0/90000.0);		

	//transparams.SetRTPSendBuffer(65536);
	transparams.SetRTPReceiveBuffer(65536*128);

	transparams.SetPortbase(portbase);
	status = sess.Create(sessparams,&transparams);	
	if(status < 0)return;

	jrtplib::RTPIPv4Address addr(destip,destport);
	status = sess.AddDestination(addr);
	if(status < 0)return;

	HRESULT hr;
	CH264_RTP_UNPACK m_h264_rtp_unpack(hr);
	bool bFrist = true;

	boost::shared_ptr<char> databuf = boost::shared_ptr<char>(new char[RAWDATALEN]);
	unsigned char* pdata = (unsigned char *)databuf.get();

	int icur = 0;
	unsigned long dwDataType = 0;
	while(m_brun){

		sess.BeginDataAccess();
		// check incoming packets
		if (sess.GotoFirstSourceWithData())
		{
			do
			{
				jrtplib::RTPPacket *pack;
				while ((pack = sess.GetNextPacket()) != NULL)
				{
					// You can examine the data here
					int timestamp = pack->GetTimestamp();

					RTPExtensionData t_RTPExtensionData;
					if(pack->GetExtensionLength() == sizeof(int))
						memcpy(&dwDataType, pack->GetExtensionData(), sizeof(int));
					if(pack->GetExtensionLength() >= sizeof(int)*2){
						memcpy(&t_RTPExtensionData, pack->GetExtensionData(), sizeof(int)*2);
						timestamp = t_RTPExtensionData.dwtimeTamp;
						dwDataType = t_RTPExtensionData.dwDataType;
					}

					if( pack->GetExtensionLength() > 0 )
					{
						//dwDataType = 2819;//mpeg4使用
						//if( bFrist ){
						//	unsigned char head[40];memset(head, 0, 40);
						//	if( !bAudio ) m_popen_camera->RecvData(0, 65536 + dwDataType, head, 40);
						//	bFrist = false;
						//}

						int i = sizeof(RTPExtensionData);
						if(icur + pack->GetPayloadLength() >= RAWDATALEN){
							if( !bAudio ) m_popen_camera->RecvData(timestamp, dwDataType, pdata, icur);
							icur = 0;
						}

						memcpy(pdata + icur, pack->GetPayloadData(), pack->GetPayloadLength());
						icur += pack->GetPayloadLength();

						if (pack->HasMarker()){
							if( !bAudio ) m_popen_camera->RecvData(timestamp, dwDataType, pdata, icur);
							icur = 0;
						}
					}
					else
					{
						//const int H264Type = 2819; //0x0B03
						const int H264Type = 20483;  //0x5003

						if( bFrist ){
							unsigned char head[40];memset(head, 0, 40);
							if( !bAudio ) m_popen_camera->RecvData(0, 65536 + H264Type, head, 40);
							bFrist = false;
						}

						// You can examine the data here
						int timestamp = pack->GetTimestamp();
						memcpy(pdata, pack->GetPacketData(), pack->GetPacketLength());

						int outsize = 0;
						BYTE* pbuf = m_h264_rtp_unpack.Parse_RTP_Packet(pdata,  pack->GetPacketLength(), &outsize);
						//m_log.Add("Parse_RTP_Packet pbuf = %d outsize=%d\n",pbuf, outsize);
						if( pbuf != NULL && outsize > 0 )
							if( !bAudio ) m_popen_camera->RecvData(0, H264Type, pbuf, outsize);
					}

					// we don't longer need the packet, so
					// we'll delete it
					sess.DeletePacket(pack);
				}
			} while (sess.GotoNextSourceWithData());
		}
		sess.EndDataAccess();

		status = sess.Poll();
		if(status < 0 || !m_brun)break;

		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
	//没有send BYE 对方UDP端口很难快速退出
	sess.BYEDestroy(jrtplib::RTPTime(3,0),0,0);
}

void CUDP_RTSPClient::handleHeartbeatThread()
{
	try{
		while(m_brun){
			int iseconds = 5*60*100/*5*60*100*/;
			while(iseconds > 0){
				boost::this_thread::sleep(boost::posix_time::milliseconds(10));
				if(!m_brun)return;
				iseconds--;

				boost::asio::socket_base::bytes_readable command(true);
				rtsp_socket_.io_control(command);
				std::size_t bytes_readable = command.get();
				if( bytes_readable > 0 )
				{
					int iret = HandleIncomingData();
					if( iret < 0 && iret != Failed ){
						g_MSNetSDKlog.Add("CUDP_RTSPClient::HandleIncomingData1 iret = %d", iret);
						return;
					}
				}
			}
			int iret = sendGet_ParameterCommand( m_url );
			if(iret < 0)return;
			iret = HandleIncomingData();
			if( iret < 0 && iret != Failed )
			{
				g_MSNetSDKlog.Add("CUDP_RTSPClient::HandleIncomingData2 iret = %d", iret);
				return;
			}
		}
	}catch (std::exception& e){
		g_MSNetSDKlog.Add("CUDP_RTSPClient::handleHeartbeatThread error = %s", e.what());
	}
}