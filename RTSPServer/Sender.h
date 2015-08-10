#pragma once

#ifndef SENDER_H
#define SENDER_H


#include "RTSPServer.h"

#include "rtpsession.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtpipv4address.h"
#include "rtpexternaltransmitter.h"

#include <boost/atomic.hpp>

namespace live555RTSP
{

class CSender
{
public:

#define RTCP_DATASTZE  64512  //1024*63
#define RTPUDPEXTRANS_HEADERSIZE (20+8)
#define DATABUF_MAXSTZE  8388608  //1024*1024*8

#define RTPUDPEXTRANS_RTPMAXSIZE 1440  //1024*63 1464 1452
#define RTPUDPEXTRANS_RTPMAXSIZE_PART 1420  //1024*62

#define RTP_PAYLOADTYPE  96 //h264
#define RTP_TIMESTAMP_INC 3600

#define H264_MAX_DEVTYPE 127
#define SDK_AGENT_DEVTYPE 255

	enum SENDERTYPE {
		SENDERTYPE_UDP = 0,
		SENDERTYPE_TCP,
		SENDERTYPE_RAW
	};
	enum RTPSENDERSTATE
	{
		CREATE = 20001,
		INITED,
		START,
		WAITFOR_SENDHAED,
		WORKING,
		STOP
	};

	typedef struct _RTPExtensionData {
		int dwDataType;
		int dwtimeTamp;
		int a;
		int b;
	} RTPExtensionData;

public:
	CSender(void):m_sendbufsize(0){};
	virtual ~CSender(void){};

	virtual int InitRTPPackSender(
		unsigned int oursessionid, char* chip,
		unsigned short clientRTPPort,
		unsigned short clientRTCPPort,
		unsigned short& serverRTPPort, // out
		unsigned short& serverRTCPPort) = 0; // out

	virtual int Start() = 0;
	virtual int SendData(unsigned long dwDataType, unsigned char *data, unsigned long len, int timestampinc) = 0;
	virtual int RecvRTCPData(unsigned char *data, unsigned long len) = 0;
	virtual void Stop() = 0;

	virtual void SetSenderState(int sender_state) = 0;

	virtual unsigned int GetSessionID() = 0;
	virtual int GetRTPSenderState() = 0;

	virtual RTSPServer::RTSPClientPtr GetRTSPClientPtr() = 0;
protected:
	boost::atomic<int> m_sendbufsize;
};

class MyRTPExternalSender;
class CRTPSender
	: public CSender
{
public:
	CRTPSender(live555RTSP::RTSPServer* ourserver);
	~CRTPSender(void);
public:
	virtual int InitRTPPackSender(
		unsigned int oursessionid, char* chip,
		unsigned short clientRTPPort,
		unsigned short clientRTCPPort,
		unsigned short& serverRTPPort, // out
		unsigned short& serverRTCPPort) = 0; // out

	int Start();
	int SendData(unsigned long dwDataType, unsigned char *data, unsigned long len, int timestampinc);
	virtual int RecvRTCPData(unsigned char *data, unsigned long len) = 0;
	virtual void Stop() = 0;
	virtual RTSPServer::RTSPClientPtr GetRTSPClientPtr() = 0;

	void SetSenderState(int sender_state){ rtpsenderstate = sender_state; };
	unsigned int GetSessionID(){ return foursessionid; };
	int GetRTPSenderState(){ return rtpsenderstate; };
		
protected:
	friend class MyRTPExternalSender;

	virtual bool SendRTP(unsigned char *data, size_t len) = 0;
	virtual bool SendRTCP(unsigned char *data, size_t len) = 0;
	virtual void StartRecvRTCP() = 0;

protected:
	live555RTSP::RTSPServer* fourserver;
	int rtpsenderstate;

	unsigned int foursessionid;
	char clientipaddress[64];
	unsigned char* rtcprecvdata;
	bool stop_;

	jrtplib::RTPSession rtpsession_;
	jrtplib::RTPExternalTransmissionInfo* rtpexternaltransinfo;
	boost::recursive_mutex rtpsession_mutex_;

private:
	MyRTPExternalSender* myrtpexternalsender;
	int m_iCurClock;
};

class MyRTPExternalSender
	:public jrtplib::RTPExternalSender
{
public:
	MyRTPExternalSender( CRTPSender* pSender ){  m_pSender = pSender; };
public:
	virtual bool SendRTP(const void *data, size_t len){ return m_pSender->SendRTP( (unsigned char *)data, len); };
	virtual bool SendRTCP(const void *data, size_t len){ return m_pSender->SendRTCP( (unsigned char *)data, len); };
	virtual bool ComesFromThisSender(const jrtplib::RTPAddress *a){ return false; };
private:
	CRTPSender* m_pSender;
};

} //end namespace

#endif // RTPPACKSENDER_H