#include "SingleCamera.h"

SingleCamera::SingleCamera(ManageCamera*cam,uint32 ID)
{
	CameraID = ID;
	ManaCam = cam;
	
	Param 	=  new CamParam(ID,this);
	
	Ana		=  new CamAnalyze(ID);
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

int SingleCamera:: set_camera_param(ST_VDCS_VIDEO_PUSH_CAM & addCam)
{
	Param->set(addCam);
}

void SingleCamera::reset_camera_param(ST_VDCS_VIDEO_PUSH_CAM & addCam)
{
	Ana ->StopAnalyze();
	Param->reset(addCam);
}

