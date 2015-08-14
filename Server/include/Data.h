#ifndef _DATA_H_
#define _DATA_H_

#include "Common.h"
#include "CmdDefine.h"

typedef struct _CAM_LIST{
	uint32 CameraID;
	char url[SINGLE_URL_LEN_128];
	  _CAM_LIST(){
		memset(this, 0, sizeof(_CAM_LIST));
	}
}T_CAM_LIST;


typedef struct _ALARM_TIME_{

	uint8 hour;
	uint8 min;

}T_AlarmTime;  /* time structure h-m*/

typedef struct _ALARM_TIME_INT{

	T_AlarmTime Start;
	T_AlarmTime End;

}ALARM_TIME_INT;

typedef struct _ALARM_DAY_INT{

    ALARM_TIME_INT time1;
    ALARM_TIME_INT time2;

}ALARM_DAY_INT;

typedef struct _ALARM_DAY{

  uint8 En;
  ALARM_DAY_INT dayTime;

}ALARM_DAY;

typedef struct _CAMERA_DB_PARAM
{
  uint32   CameraID;
  char    ip[IP_LEN_16];

  uint8   frameRate;
  char    CamUrl[SINGLE_URL_LEN_128];
  char    RtspUrl[SINGLE_URL_LEN_128];

  uint8   CameraFunc;  // 1是拍照  2是分析
  uint8   AnalyzeNUM;  // 分析个数
  uint16  AnalyzeType; // 分析类型

  uint8   CamStatus;
  _CAMERA_DB_PARAM(){
		memset(this, 0, sizeof(_CAMERA_DB_PARAM));
	}
}DBCAMERACONFI;

typedef struct _CMAERA_DB_FUNC_PARAM
{
	uint32   CameraID;
	char    ip[IP_LEN_16];
	uint8   AnalyzeNUM;  // 分析个数
	uint16  AnalyzeType;
	uint16  MaxHumanNum;
	float   ChangRate;

	uint16  AnalyzeType1;
	uint8   AnalyzeEn1;
	ALARM_DAY  AlarmTime1[WEEK_DAY_LEN_7]; // 7*9 = 63 bytes
	uint16      PkgNum1;
	vector < VIDEO_DRAW >  VideoDraw1;    //10*n bytes

	uint16  AnalyzeType2;
	uint8   AnalyzeEn2;
	ALARM_DAY  AlarmTime2[WEEK_DAY_LEN_7];
	uint16      PkgNum2;
	vector < VIDEO_DRAW >  VideoDraw2;

	_CMAERA_DB_FUNC_PARAM(){
		memset(this, 0, sizeof(_CMAERA_DB_FUNC_PARAM));
	}
}DBCAMERAFUNCPARAM;

#endif
