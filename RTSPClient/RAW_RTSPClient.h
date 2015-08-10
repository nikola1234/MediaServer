#pragma once
#include "RTSPClient.h"

class COpenCamera;
class CRAW_RTSPClient :
	public CRTSPClient
{
public:
	CRAW_RTSPClient(COpenCamera *pcam);
	~CRAW_RTSPClient(void);

public:
	virtual int startRTSPRequest(char* ip ,int port ,const char *url, const char* Range_clock = "-", float scale = 1.0);

protected:
	virtual int sendSetupCommand(const char * url);
	virtual void handleRecvVideoDataThread();

private:
	COpenCamera* m_popen_camera;
	int m_iStreamType;
	int m_iTimestampSeconds;
};

