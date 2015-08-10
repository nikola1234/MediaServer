#pragma once

#ifndef UDPSENDER_H
#define UDPSENDER_H

#include "Sender.h"


namespace live555RTSP
{

class CUDPSender
	: public CSender
	, public boost::enable_shared_from_this<CUDPSender>
{
public:
	CUDPSender(live555RTSP::RTSPServer* ourserver);
	~CUDPSender(void);

public:
	int InitRTPPackSender(
		unsigned int oursessionid, char* chip,
		unsigned short clientRTPPort,
		unsigned short clientRTCPPort,
		unsigned short& serverRTPPort, // out
		unsigned short& serverRTCPPort); // out

	int Start();
	int SendData(unsigned long dwDataType, unsigned char *data, unsigned long len, int timestampinc);
	int RecvRTCPData(unsigned char *data, unsigned long len){ return false; };
	void Stop();

	unsigned int GetSessionID(){ return foursessionid; };
	int GetRTPSenderState(){ return rtpsenderstate; };
	void SetSenderState(int sender_state){ rtpsenderstate = sender_state; };
	RTSPServer::RTSPClientPtr GetRTSPClientPtr(){ RTSPServer::RTSPClientPtr t; return t; };

protected:
	bool SendRTP(unsigned char *data, size_t len){ return false; };
	bool SendRTCP(unsigned char *data, size_t len){ return false; };
	void StartRecvRTCP(){ return ; };

protected:
	live555RTSP::RTSPServer* fourserver;

	int rtpsenderstate;
	unsigned int foursessionid;

	bool stop_;

	char clientipaddress[64];
	unsigned short rtcpserverport;
	unsigned short rtpserverport;

	unsigned short rtcpclientport;
	unsigned short rtpclientport;

	jrtplib::RTPSession rtpsession_;
	boost::recursive_mutex rtpsession_mutex_;
};

} //end namespace
#endif // UDPSENDER_H