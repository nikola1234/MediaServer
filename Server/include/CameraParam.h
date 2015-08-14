
#ifndef _CAMERA_PARAM_H_
#define _CAMERA_PARAM_H_

#include "Common.h"
#include "CmdDefine.h"
#include "Data.h"
#include "SingleCamera.h"

class CamParam
{
public:
	CamParam(uint32 ID,SingleCamera *sincam);
	~CamParam();

	string get_camera_rtsp_url();

	
	int set(ST_VDCS_VIDEO_PUSH_CAM & addCam);
	void reset(ST_VDCS_VIDEO_PUSH_CAM & addCam);

private:
	SingleCamera *SinCam;

	uint32  CameraID;
	char   CameraIP[IP_LEN_16];
	uint8   frameRate;
	char  CamUrl[SINGLE_URL_LEN_128];
	char  RtspUrl[SINGLE_URL_LEN_128];

	uint8   CameraFunc; // 1 take photo 2 analyze
	uint8   AnalyzeNUM;
	uint16  AnalyzeType;

	uint16  AnalyzeType1;
	uint16 AnalyzeType2;
	
	int generate_url();
	int parse_type(uint16 & type);
/*
	uint16  MaxHumanNum;
	float    ChangRate;

	ALARM_DAY  AlarmTime1[WEEK_DAY_LEN_7];
	uint16      PkgNum1;
	vector < VIDEO_DRAW >  VideoDraw1;

	ALARM_DAY  AlarmTime2[WEEK_DAY_LEN_7];
	uint16      PkgNum2;
	vector < VIDEO_DRAW >  VideoDraw2;

*/
};

#endif
