#ifndef _SINGLE_CAMERA_H_
#define _SINGLE_CAMERA_H_

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Common.h"
#include "CamReadThread.h"
#include "CamAnaThread.h"
#include "CamTimeThread.h"
#include "Data.h"
#include "CameraManage.h"


class CamReadThread;
class CamTimeThread;
class CamAnaThread;


uint16 check_analyzetype(uint16 &type);

class SingleCamera
:public boost::enable_shared_from_this<SingleCamera>
{

public:
	SingleCamera(ManageCamera*cam,uint32 ID);
	~SingleCamera();
	
	uint32 GetCameraID(){return CameraID;}
	string  get_rtsp_url();

	int set_camera_fix_param(ST_VDCS_VIDEO_PUSH_CAM & addCam);
	void reset_camera_fix_param(ST_VDCS_VIDEO_PUSH_CAM & addCam);

	int reset_camera_var_param(T_VDCS_VIDEO_CAMERA_PARAM* pt_CameraParam,vector <VIDEO_DRAW> & Pkg);
public:
	uint32 CameraID;

	char   CameraIP[IP_LEN_16];
	uint8   frameRate;
	char  CamUrl[SINGLE_URL_LEN_128];
	char  RtspUrl[SINGLE_URL_LEN_128];

	uint8   CameraFunc; // 1 take photo 2 analyze
	uint8   AnalyzeNUM;
	uint16  AnalyzeType;

	uint16  AnalyzeType1;
	uint16 AnalyzeType2;
	
	
	CamReadThread*  ReadThread;
	CamTimeThread*  TimeThread;
	CamAnaThread*	Ana1Thread;
	CamAnaThread*	Ana2Thread;
	
	ManageCamera * ManaCam;
	
private:
	int generate_url();
	int parse_type(uint16 & type);
};
#endif
