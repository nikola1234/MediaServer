#pragma once
#include "RTSPClient.h"
#include "UdpPortVector.h"

class COpenCamera;
class CUDP_RTSPClient :
	public CRTSPClient
{
public:
	CUDP_RTSPClient(COpenCamera *pcam);
	~CUDP_RTSPClient(void);

public:
	virtual int startRTSPRequest(char* ip ,int port ,const char *url, const char* Range_clock = "-", float scale = 1.0);

protected:
	virtual int sendSetupCommand(const char * url);
	virtual void handleRecvVideoDataThread();

private:
	void handleHeartbeatThread();

public:
	static live555RTSP::UDPPortVector s_udpportvector;
	char m_realConnectIP[256];
private:
	boost::thread Heartbeat_thread_;
	COpenCamera* m_popen_camera;

	int m_clinet_video_rtpport;
	int m_clinet_audio_rtpport;
};

