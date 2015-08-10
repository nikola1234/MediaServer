// DllCamera.h: interface for the CHikCamera class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AnalyzeCAMERA_H__E996F147_05C9_43FF_A467_E9C455632E5B__INCLUDED_)
#define AFX_AnalyzeCAMERA_H__E996F147_05C9_43FF_A467_E9C455632E5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ICamera.h"

class CAnalyzeCamera : public ICamera  
{
public:
	CAnalyzeCamera(){ m_capture_callback = NULL; };
	virtual ~CAnalyzeCamera(){ };
	
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

#endif // !defined(AFX_AnalyzeCAMERA_H__E996F147_05C9_43FF_A467_E9C455632E5B__INCLUDED_)
