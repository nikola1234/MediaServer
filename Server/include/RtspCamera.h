#ifndef _RTSP_CAMERA_H_
#define _RTSP_CAMERA_H_
#include "Common.h"
#include "MediaServerDLL.h"
#include "ICamera.h"

class CRtspCamera : public ICamera
{
public:
	CRtspCamera(){ m_capture_callback = NULL; };
	virtual ~CRtspCamera(){ };

	virtual int openCamera(int CameraID, int ChannelID, int user, VIDEO_CAPTURE_CALLBACKEX capture_callback, RESULT_PARAM_S *resultParam)
	{
		if( m_capture_callback == NULL ) m_capture_callback = capture_callback;
		return 0;
	};

	virtual int closeCamera(int CameraID) { return 0; };
	virtual int switchCamera(int CameraID, int oldCameraID, int ChannelID, int user, VIDEO_CAPTURE_CALLBACKEX capture_callback) { return -1; };
	virtual int controlCamera(int CameraID, int ChannelID, int ControlType, int ControlSpeed) { return -1; };

	VIDEO_CAPTURE_CALLBACKEX m_capture_callback;
};

#endif
