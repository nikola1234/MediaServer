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

void SingleCamera::set_camera_param(ST_VDCS_VIDEO_PUSH_CAM & addCam)
{
	
	char		ip[IP_LEN_16];
	char		CameUrL[SINGLE_URL_LEN_128];

	uint8		Enable;
	uint8		frameRate;
	uint8		CameraFunc;
	uint8		AnalyzeNUM;
	uint16      AnalyzeType; //最多两个分析或运算

	
	Ana ->stop();
	Param->release();

	Param->CameraID = CameraID

	
	uint32	CameraID;
		string	CameraIP;
		uint8	frameRate;
		string	CamUrl;
		string	RtspUrl;
	
		uint8	CameraFunc; // 1 take photo 2 analyze
		uint8	AnalyzeNUM;
		uint16	AnalyzeType;
	
		uint16	MaxHumanNum;
		float	ChangRate;
	
		uint16	AnalyzeType1;
		uint8	AnalyzeEn1;
		ALARM_DAY  AlarmTime1[WEEK_DAY_LEN_7];
		uint16		PkgNum1;
		vector < VIDEO_DRAW >  VideoDraw1;
	
		uint16	AnalyzeType2;
		uint8	AnalyzeEn2;
		ALARM_DAY  AlarmTime2[WEEK_DAY_LEN_7];
		uint16		PkgNum2;
		vector < VIDEO_DRAW >  VideoDraw2;
}

