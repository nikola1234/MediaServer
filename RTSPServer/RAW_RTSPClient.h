#pragma once
#include "RTSPClient.h"

namespace MediaSave
{

class CCameraRecord;
class CRAW_RTSPClient :
	public CRTSPClient
{
public:
	CRAW_RTSPClient(CCameraRecord *pcam,  int tokenType = 2 );
	~CRAW_RTSPClient(void);

	void setUserAgentByToken(){ sprintf(m_user_agent, "VNMPNetSDK V1.0 %02d %d", m_tokenType, m_sClientID); };
public:
	virtual int startRTSPRequest(char* ip ,int port ,const char *url, const char* Range_clock = "-", float scale = 1.0);

protected:
	virtual int sendSetupCommand(const char * url);
	virtual void handleRecvVideoDataThread();

private:
	CCameraRecord* m_camera_record;
	int m_tokenType;

	int m_iStreamType;
};

}// end namespace
