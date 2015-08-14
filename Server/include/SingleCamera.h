#ifndef _SINGLE_CAMERA_H_
#define _SINGLE_CAMERA_H_

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Common.h"
#include "CameraAnalyze.h"
#include "CameraParam.h"
#include "Data.h"
#include "CameraManage.h"

class SingleCamera
:public boost::enable_shared_from_this<SingleCamera>
{

public:
	SingleCamera(ManageCamera*cam,uint32 ID);
	~SingleCamera();
	
	uint32 GetCameraID(){return CameraID;}
	
	int set_camera_param(ST_VDCS_VIDEO_PUSH_CAM & addCam);
	void reset_camera_param(ST_VDCS_VIDEO_PUSH_CAM & addCam);
	string  get_rtsp_url();
private:
	uint32 CameraID;
	CamParam * Param;
	CamAnalyze *Ana;
	ManageCamera * ManaCam;
};
#endif