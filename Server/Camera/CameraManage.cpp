
#include "CameraManage.h"

ManageCamera::ManageCamera()
{

}

ManageCamera::~ManageCamera()
{

}

int ManageCamera::InitFromDB()
{
  	return 0;
}

int ManageCamera::try_to_open(string stream)
{
	VideoCapture  vcap;
	 if(!vcap.open(stream)) {
	        return -1;
	}
	return 0;
}

string &Create_or_Renew_Camera(ST_VDCS_VIDEO_PUSH_CAM & addCam)
{
	int iRet = -1;
	string url = "TT";
	iRet =Find_Cam_in_list_by_ip();  /* 0 have 1 none */
	if(iRet == 0){
		reset_param();
		set_or_change_analyze(); /*start with stop*/
		url = get_url();
		Add_or_Renew_DB();
		return url;
	}

	if(iRet ==  1){
		create_camera_ID();
		set_param();
		set_or_change_analyze();
		url =getUrl();
		Add_or_Renew_DB();
		return url;
	}
	return url;
}

