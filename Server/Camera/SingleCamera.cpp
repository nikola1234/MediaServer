#include "SingleCamera.h"

SingleCamera::SingleCamera();
{
	CameraID = 0;
	Param 	=NULL;
	Ana		=NULL;
}

SingleCamera::~SingleCamera()
{
	CameraID = 0;
	Param 	=NULL;
	Ana		=NULL;
}

string  SingleCamera:: get_rtsp_url()
{	string url;
	url =Param->get_camera_rtsp_url();
	return url;
}

void SingleCamera::set_camera_param(ST_VDCS_VIDEO_PUSH_CAM & addCam)
{
	Ana ->StopAnalyze();
	Param->release();
	Param->reset(addCam,CameraID);
}

