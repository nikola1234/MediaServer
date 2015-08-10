#pragma once
#include "ServerMediasession.h"
#include "inc/VideoNetSDK.h"

namespace live555RTSP
{

class AgentServerMediasession :
	public ServerMediasession
{
public:
	AgentServerMediasession(RTSPServer* ourserver, CDataInfo::DBCAMERAINFO &cameradata, int coderid, std::string urlSuffix);
	~AgentServerMediasession(void);

	virtual int startStream( unsigned clientSessionId, std::string time_param, float scale = 1.0);
	virtual int StopMediaServer();

private:
	std::string m_rtsp_url;
	int m_video_devnum;
	static int m_login_handle;
};

} // end namespace