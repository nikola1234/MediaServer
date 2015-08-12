#ifndef _DATA_H_
#define _DATA_H_

#include "Common.h"
#include "CmedDefine.h"

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

typedef struct _VIDEO_DRAW  // pkg means special parameter
{
	uint16 	StartX;				//StartX
	uint16 	StartY;				//StartY
	uint16 	EndX;					//EndX    if draw is rectangle  means width
	uint16 	EndY;					//EndY    if draw is rectangle  means height
	uint16  Type;					//1 monitor 2 Inward and outward 3 region

	_VIDEO_DRAW(){
		memset(this, 0, sizeof(_VIDEO_DRAW));
	}
}VIDEO_DRAW;

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
/*
class CamDataInfo:
	public IData
{

public:
	CamDataInfo();
	virtual ~CamDataInfo();

	virtual int Open(const char* szfilepath); //打开数据库

	virtual int getCameraInfo(int iCameraID, char chRemark);
	virtual int getCameraConfig( int cameraid, DBCAMERACONFI *camera );
	virtual int getCameraAlarmInfo( int cameraid, DBCAMERAFUNCPARAM *camera );

	virtual int GetMaxCameraID(int iDevID);

	virtual int setCameraInfo(int iCameraID, char chRemark);
	virtual int setCameraInfo(int iCameraID, DBCAMERACONFI *camera );
	virtual int setCameraInfo(int iCameraID, DBCAMERAFUNCPARAM *camera);

	virtual int AddCameraInfo(int iCameraID, char chRemark);
	virtual int AddCameraInfo(int iCameraID, DBCAMERACONFI *camera );
	virtual int AddCameraInfo(int iCameraID, DBCAMERAFUNCPARAM *camera);

	virtual int DelCameraInfo(int iCameraID);//
public:
	CMyLog m_log;
protected:
	boost::mutex m_db_mutex_;
	std::string m_szFileDB;
	cppsqlite3::CppSQLite3DB m_sqlite3db;
};
*/
#endif
