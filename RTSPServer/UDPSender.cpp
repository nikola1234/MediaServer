#include "UDPSender.h"
#include "rtpudpv4transmitter.h"
#include "RTPPackSender.h"

namespace live555RTSP
{

CUDPSender::CUDPSender(live555RTSP::RTSPServer* ourserver)
	: fourserver(ourserver),
	rtpsenderstate(CREATE),
	foursessionid(0),
	stop_(false)
{
	memset(clientipaddress, 0 ,64);
}

CUDPSender::~CUDPSender(void)
{
}

int CUDPSender::InitRTPPackSender(
	unsigned int oursessionid, char* chip,
	unsigned short clientRTPPort,
	unsigned short clientRTCPPort,
	unsigned short& serverRTPPort, // out
	unsigned short& serverRTCPPort) // out
{
	foursessionid = oursessionid;

	rtcpclientport = clientRTCPPort;
	rtpclientport = clientRTPPort;

	rtcpserverport = CUDPRTPSender::s_udpportvector.GetFreeUDPPort();
	rtpserverport = rtcpserverport - 1;

	if(rtcpserverport == 0)
		return -1;

	serverRTCPPort = rtcpserverport;
	serverRTPPort = rtpserverport;

	strcpy(clientipaddress, chip);
	foursessionid = oursessionid;

	fourserver->m_log.Add("UDPSender::UDPSender foursessionid = %d", foursessionid);

	rtpsenderstate = INITED;
	return 0;
}

int CUDPSender::Start()
{
	//init rtpsession
	jrtplib::RTPSessionParams sessionparams;
	sessionparams.SetOwnTimestampUnit(1.0/90000.0);
	sessionparams.SetMaximumPacketSize(RTPUDPEXTRANS_RTPMAXSIZE);

	//jrtplib::RTPExternalTransmissionParams transparams(myrtpexternalsender,RTPUDPEXTRANS_HEADERSIZE);
	jrtplib::RTPUDPv4TransmissionParams transparams;
	transparams.SetBindIP(boost::asio::ip::address_v4::from_string(fourserver->getRTSPAddress().c_str()).to_ulong());
	transparams.SetPortbase(rtpserverport);

	transparams.SetRTPSendBuffer(64*65536);

	rtpsession_.Create(sessionparams,&transparams,jrtplib::RTPTransmitter::IPv4UDPProto);

	rtpsession_.SetDefaultPayloadType(RTP_PAYLOADTYPE);
	rtpsession_.SetDefaultMark(false);
	rtpsession_.SetDefaultTimestampIncrement(RTP_TIMESTAMP_INC);

	jrtplib::RTPIPv4Address addr(boost::asio::ip::address_v4::from_string(clientipaddress).to_ulong(), rtpclientport);

	int status = rtpsession_.AddDestination(addr);

	rtpsenderstate = START;

	return 0;
}

int CUDPSender::SendData(unsigned long dwDataType, unsigned char *data, unsigned long len, int timestampinc)
{
	if (stop_)return -1;
	if(!( rtpsenderstate == WAITFOR_SENDHAED || rtpsenderstate == WORKING ))return -2;

	if( (dwDataType >> 16)  > 0){
		rtpsenderstate = WORKING;
	}

	int cursize = len;
	unsigned char* sendbuf = data;
	int status = 0;

	RTPExtensionData t_RTPExtensionData;
	t_RTPExtensionData.dwDataType = dwDataType;
	t_RTPExtensionData.dwtimeTamp = timestampinc;  // 失效

	int t_ExtensionLenth = 1;
	//t_ExtensionLenth > 1时jrtp工作异常 发送数据好像丢包
	//if(timestampinc != -1)
	//	t_ExtensionLenth = 2;

	int devtype = dwDataType & 0xffff;
	if( devtype < H264_MAX_DEVTYPE && ( (dwDataType >> 16)  > 0 ) )
		return 0;

	boost::recursive_mutex::scoped_lock lock_(rtpsession_mutex_);
	if( devtype <= H264_MAX_DEVTYPE )
	{
		jrtplib::RTPHeader rtpHeader;
		memcpy(&rtpHeader, sendbuf, sizeof(jrtplib::RTPHeader));

		if(rtpHeader.marker == 1)
		{
			status = rtpsession_.SendPacket(sendbuf + 12, len - 12, RTP_PAYLOADTYPE, true, /*iTimeStampInc*/RTP_TIMESTAMP_INC);
		}
		else
			status = rtpsession_.SendPacket(sendbuf + 12, len - 12, RTP_PAYLOADTYPE, false, 0);
	}
	else
	{
		while( cursize > 0 )
		{
			if( cursize >= RTPUDPEXTRANS_RTPMAXSIZE_PART )
			{
				status = rtpsession_.SendPacketEx(sendbuf,RTPUDPEXTRANS_RTPMAXSIZE_PART,RTP_PAYLOADTYPE,false,0, 1, &t_RTPExtensionData, t_ExtensionLenth);
				sendbuf = sendbuf + RTPUDPEXTRANS_RTPMAXSIZE_PART;
				cursize = cursize - RTPUDPEXTRANS_RTPMAXSIZE_PART;
			}
			else
			{
				status = rtpsession_.SendPacketEx(sendbuf,cursize,RTP_PAYLOADTYPE,true,RTP_TIMESTAMP_INC, 1, &t_RTPExtensionData, t_ExtensionLenth);
				sendbuf = sendbuf + cursize;
				cursize = 0;
			}
		}
	}

	status = rtpsession_.Poll();
	lock_.unlock();

	return status;
}

void CUDPSender::Stop()
{
	if (stop_)return;
	stop_ = true;
	rtpsenderstate = STOP;
}

} //end namespace
