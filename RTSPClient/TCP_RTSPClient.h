#pragma once
#include "RTSPClient.h"

#include "rtpexternaltransmitter.h"

#include "rtpsession.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtpipv4address.h"
#include "rtppacket.h"

#define RTCP_DATASTZE  64512  //1024*63
#define RTPUDPEXTRANS_HEADERSIZE (20+8)

#define RTPUDPEXTRANS_RTPMAXSIZE 1400  //1024*63
#define RTPUDPEXTRANS_RTPMAXSIZE_PART 1200  //1024*62

#define RTP_PAYLOADTYPE  96 //h264

class MyRTPExternalSender;
class COpenCamera;
class CTCP_RTSPClient :
	public CRTSPClient
{
public:
	CTCP_RTSPClient(COpenCamera *pcam);
	~CTCP_RTSPClient(void);

public:
	virtual int startRTSPRequest(char* ip ,int port ,const char *url, const char* Range_clock = "-", float scale = 1.0);

protected:
	friend class MyRTPExternalSender;

	virtual int sendSetupCommand(const char * url);
	virtual void handleRecvVideoDataThread();

	bool SendRTCP(unsigned char *data, size_t len);

private:
	COpenCamera* m_popen_camera;

	MyRTPExternalSender* myrtpexternalsender;
	jrtplib::RTPExternalTransmissionInfo* rtpexternaltransinfo;
};

class MyRTPExternalSender
	:public jrtplib::RTPExternalSender
{
public:
	MyRTPExternalSender( CTCP_RTSPClient* rtsp_client ){  m_prtsp_clinet = rtsp_client; };
public:
	virtual bool SendRTP(const void *data, size_t len){ return true; };
	virtual bool SendRTCP(const void *data, size_t len){ return m_prtsp_clinet->SendRTCP( (unsigned char *)data, len); };
	virtual bool ComesFromThisSender(const jrtplib::RTPAddress *a){ return false; };
private:
	CTCP_RTSPClient* m_prtsp_clinet;
};