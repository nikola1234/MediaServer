#include "AgentServerMediasession.h"
#include "inc/VideoNetSDK.h"

namespace live555RTSP
{
int AgentServerMediasession::m_login_handle = 0;

AgentServerMediasession::AgentServerMediasession(RTSPServer* ourserver, CDataInfo::DBCAMERAINFO &cameradata, int coderid, std::string urlSuffix)
	: ServerMediasession(ourserver, cameradata, coderid)
{
	//标志可以复制转发
	//m_bliveMediaSession = false;
	m_strURLSuffix = urlSuffix;

	static bool bInitRTSPSDK = false;
	if( !bInitRTSPSDK )
	{
		video_device_init();
		VIDEODEVICE videodev;
		memset(&videodev, 0, sizeof(VIDEODEVICE));
		m_login_handle = video_device_login(&videodev);
	}

	char chUrl[256]; memset(chUrl, 0, 256);
	unsigned resultlen = 0;
	char* pBase64 = ( char* )base64Decode((char*)urlSuffix.c_str(), resultlen, True);
	if( resultlen < 128 ) memcpy(chUrl, pBase64, resultlen);
	m_rtsp_url = chUrl;
	delete[] pBase64;
}

AgentServerMediasession::~AgentServerMediasession(void)
{
	StopMediaServer();
}

int AgentServerMediasession::startStream(unsigned clientSessionId, std::string time_param, float scale)
{
	VIDEODEVICE videodev;
	memset(&videodev, 0, sizeof(VIDEODEVICE));
	videodev.handle = m_login_handle;
	videodev.reuse = VNMP_TRANSPORT_UDP;
	//videodev.reuse = VNMP_TRANSPORT_RAW;
	strcpy(videodev.channel, m_rtsp_url.c_str());
	m_video_devnum = video_device_capturing_startEx(&videodev, (void*)m_mediaid, (VIDEO_CAPTURE_CALLBACKEx)video_capture_callbackEx);
	if( m_video_devnum < 0 ){ strcpy(m_strResult, "对不起，打开前端码流失败！"); return -1;}

	return 0;
}

int AgentServerMediasession::StopMediaServer()
{
	VIDEODEVICE videodev;
	memset(&videodev, 0, sizeof(VIDEODEVICE));
	videodev.handle = m_login_handle;
	videodev.devnum = m_video_devnum;
	video_device_capturing_stop( &videodev );

	return 0;
}

} // end namespace