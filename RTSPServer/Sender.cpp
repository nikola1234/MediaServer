#include "Sender.h"

namespace live555RTSP
{

CRTPSender::CRTPSender(live555RTSP::RTSPServer* ourserver)
	:myrtpexternalsender(NULL),
	fourserver(ourserver),
	stop_(false),
	rtpexternaltransinfo(NULL),
	rtpsenderstate(CREATE),
	foursessionid(0),
	m_iCurClock(0)
{
	memset(clientipaddress, 0 ,64);

	myrtpexternalsender = new MyRTPExternalSender(this);
	rtcprecvdata = new unsigned char[RTCP_DATASTZE];
}

CRTPSender::~CRTPSender(void)
{
	if(myrtpexternalsender){
		delete myrtpexternalsender;
		myrtpexternalsender = NULL;
	}

	if (rtpexternaltransinfo){
		rtpsession_.DeleteTransmissionInfo(rtpexternaltransinfo);
		//delete rtpexternaltransinfo;
		rtpexternaltransinfo = NULL;
	}

	if( rtcprecvdata ){
		delete[] rtcprecvdata;
		rtcprecvdata = NULL;
	}
}

int CRTPSender::Start()
{
	//init rtpsession
	jrtplib::RTPSessionParams sessionparams;
	sessionparams.SetOwnTimestampUnit(1.0/90000.0);
	sessionparams.SetMaximumPacketSize(RTPUDPEXTRANS_RTPMAXSIZE);

	jrtplib::RTPExternalTransmissionParams transparams(myrtpexternalsender,RTPUDPEXTRANS_HEADERSIZE);
	//jrtplib::RTPUDPv4TransmissionParams transparams;
	rtpsession_.Create(sessionparams,&transparams,jrtplib::RTPTransmitter::ExternalProto);

	rtpsession_.SetDefaultPayloadType(RTP_PAYLOADTYPE);
	rtpsession_.SetDefaultMark(false);
	rtpsession_.SetDefaultTimestampIncrement(RTP_TIMESTAMP_INC);

	rtpexternaltransinfo = (jrtplib::RTPExternalTransmissionInfo *)rtpsession_.GetTransmissionInfo();

	StartRecvRTCP();

	rtpsenderstate = START;
	return 0;
}

int CRTPSender::SendData(unsigned long dwDataType, unsigned char *data, unsigned long len, int timestampinc)
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
			//int iNowClock = (int)(((float)clock()/CLOCKS_PER_SEC ) * 1000.0);
			//int iTimeStampInc = iNowClock - m_iCurClock;
			//m_iCurClock = iNowClock;
			//if( iTimeStampInc > 90000 / 2 || iTimeStampInc <= 0)
			//	iTimeStampInc = RTP_TIMESTAMP_INC;
			//else
			//	iTimeStampInc = iTimeStampInc * 90;
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

	//if ( status < 0 )
	//	std::cerr << "CSender::HandleSendRTPData" << jrtplib::RTPGetErrorString(status) << std::endl;

	return status;
}

}// end namespace