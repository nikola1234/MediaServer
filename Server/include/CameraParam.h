
#ifndef _CAMERA_PARAM_H_
#define _CAMERA_PARAM_H_
#include "Common.h"

class CamParam
{
public:
  CamParam();
  ~CamParam();

  void set_camera_id(uint32 ID){CameraID = ID;}
  uint32 get_camera_id(){return CameraID;}

  void  set_camera_ip(string ip){CameraIP =ip;}
  string get_camera_ip(){return CameraIP;}

  void change_framerate(uint8 rate){frameRate = rate;}

  void  set_camera_url(string url){CamUrl =url;}
  string get_camera_url(){return CamUrl;}

  int get_camera_rtspurl(string &rtspurl);

  void set_camera_func(uint8 func) {CameraFunc = func;}
  uint8 get_camera_func() {return CameraFunc;}

  void set_analyze_num(uint8 num)  {AnalyzeNUM = num;}
  uint8 get_analyze_num() {return AnalyzeNUM;}

private:

  uint32  CameraID;
  string  CameraIP;
  uint8   frameRate;
  string  CamUrl;
  string  RtspUrl;

  uint8   CameraFunc; // 1 take photo 2 analyze
  uint8   AnalyzeNUM;
  uint16  AnalyzeType;

	uint16  MaxHumanNum;
	float   ChangRate;

	uint16  AnalyzeType1;
	uint8   AnalyzeEn1;
	ALARM_DAY  AlarmTime1[WEEK_DAY_LEN_7];
	uint16      PkgNum1;
	vector < VIDEO_DRAW >  VideoDraw1;

	uint16  AnalyzeType2;
	uint8   AnalyzeEn2;
	ALARM_DAY  AlarmTime2[WEEK_DAY_LEN_7];
	uint16      PkgNum2;
	vector < VIDEO_DRAW >  VideoDraw2;

};

#endif
