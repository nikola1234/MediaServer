// DataInfo.h: interface for the CDataInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAINFO_H__45D6DFB6_DE4F_4A45_A1AB_694F51FE2074__INCLUDED_)
#define AFX_DATAINFO_H__45D6DFB6_DE4F_4A45_A1AB_694F51FE2074__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <boost/asio.hpp>
#include <list>
#include <iostream>
#include <vector>
#include <boost/thread.hpp>
#include <unistd.h>
//#include <io.h>

#include <sys/types.h>
#include <sys/stat.h>

//#include "VideoDevice.h"
#include "CppSQLite3.h"
#include "MyLog.h"
//#include "ICamera.h"

using namespace std;
#define SQL_STRING_MAX 4096
#define STRATEGYID_ITSSELF  913156789

#define NEW_CONFIG_DB_500 //5.0

//#define OLD_SUBSYSTEMINFO_CONFIG_DB
//#define OLD_ITEMBASECONFIG_DB
//#define OLD_VIRTUALCHANNEL

#define sprintf_s sprintf
#define strcpy_s strcpy

#define IP_LEN_16            16
#define SINGLE_URL_LEN_128   128
#define WEEK_DAY_LEN_7        7
#define DB_OK                 0
#define DB_NOT_EXIST          -1
#define DB_PATH_LEN           128
#define DB_PATH               "./DataBase/"
#define DBNAME                "CameraDB.db"
#define DB_CREATE_ERROR       -1
typedef unsigned long  uint32;
typedef unsigned char  uint8;
typedef unsigned  int   uint16;
typedef struct _CAMERA_DB_PARAM
{
  uint32   CameraID;
  char    ip[IP_LEN_16];
  int     Port;
  uint8   frameRate;
  char    CamUrl[SINGLE_URL_LEN_128];
  char    RtspUrl[SINGLE_URL_LEN_128];

  uint8   CameraFunc;  // 1??????  2??¡¤???
  uint8   AnalyzeNUM;  // ¡¤???????
  uint16  AnalyzeType; // ¡¤????¨¤??
  int   CamStatus;
  _CAMERA_DB_PARAM(){
        memset(this, 0, sizeof(_CAMERA_DB_PARAM));
    }
}DBCAMERACONFI;

typedef struct _STR_TIME
{
    char En;
    char StartTime[6];
    char EndTime[6];
}StrTime;

typedef struct _STR_TIME_DAY
{
    StrTime Time[3];
}StrTimeDay;

//typedef struct _STR_TIME_DAY
//{
//    StrTimeData Time[WEEK_DAY_LEN_7];
//}StrTimeDay;

typedef struct _ALARM_TIME_{

    uint8 hour;
    uint8 min;

}T_AlarmTime;  /* time structure h-m*/

typedef struct _ALARM_TIME_INT{

    T_AlarmTime Start;
    T_AlarmTime End;

}ALARM_TIME_INT;

typedef struct _ALARM_DAY_INT{

    ALARM_TIME_INT time[3];

}ALARM_DAY_INT;
typedef struct _ALARM_DAY{

  char En;
  ALARM_DAY_INT dayTime;
//  char *str;
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


typedef struct _CMAERA_DB_FUNC_PARAM
{
    uint32   CameraID;
    char    ip[IP_LEN_16];
    uint8   AnalyzeNUM;
    uint16  AnalyzeType;
    uint16  MaxHumanNum;
    float   ChangeRate;

    uint16  AnalyzeType1;
    uint8   AnalyzeEn1;
//    ALARM_DAY  AlarmTime1[WEEK_DAY_LEN_7]; // 7*9 = 63 bytes
    char    AlarmTime1[295];
    uint16      PkgNum1;
//    vector < VIDEO_DRAW >  WatchRegion1;    //10*n bytes
//    char   WatchRegion1[1024];
    char  *WatchRegion1;
//    string  WatchRegion1;

    uint16  AnalyzeType2;
    uint8   AnalyzeEn2;
//    ALARM_DAY  AlarmTime2[WEEK_DAY_LEN_7];
    char      AlarmTime2[295];
    uint16      PkgNum2;
//    vector < VIDEO_DRAW >  WatchRegion2;
    char* WatchRegion2;

    _CMAERA_DB_FUNC_PARAM(){
        memset(this, 0, sizeof(_CMAERA_DB_FUNC_PARAM));
    }
}DBCAMERAFUNCPARAM;
class CamDataInfo
{

public:
    CamDataInfo();
    virtual ~CamDataInfo();
    virtual void ParseAlarmArea( char *str, VIDEO_DRAW *output);
    virtual void AreaToDB( char **areaStr, vector < VIDEO_DRAW >  &WatchRegion );
    virtual void DBToArea(char *m_areaNode,vector < VIDEO_DRAW >  &WatchRegion);
    virtual int ConvertToTime(char *time,char *t);
    virtual int TimeToConvert(char *t,char *time);
    virtual int anlyzetime(char *str, StrTimeDay * strtimeday,ALARM_DAY* alarmtime1);
    virtual int TimeToDB(char *str, StrTimeDay * strtimeday,ALARM_DAY* alarmtime1,char sel);
    virtual int Open(const char* szfilepath);
    virtual int exist(const char* dbname);
    virtual int sqlite3_create_db(const char *db_name);
    virtual int sqlite3_create_table(const char* db_name);
    /*********************query************************/
    virtual int getCameraInfo(int iCameraID, char chRemark);
    virtual int getCameraConfig( int cameraid, DBCAMERACONFI *camera );
    virtual int getCameraAlarmInfo( int cameraid, DBCAMERAFUNCPARAM *camera );
    /*********************set***********************/
    virtual int setCameraInfo(int iCameraID, char chRemark);
    virtual int setCameraConfig(int iCameraID, DBCAMERACONFI *camera );
    virtual int setCameraAlarmInfo(int iCameraID, DBCAMERAFUNCPARAM *camera);
    /*********************add***********************/
    virtual int AddCameraInfo(int iCameraID, char chRemark);
    virtual int AddCameraConfig(int iCameraID, DBCAMERACONFI *camera );
    virtual int AddCameraAlarmInfo(int iCameraID, DBCAMERAFUNCPARAM *camera);
    /*********************del***********************/
    virtual int DelCameraConfig(int iCamera);
    virtual int DelCameraAlarmInfo(int iCamera);
    virtual int GetMaxCameraID(const char *db_name);
    virtual int getAllCameraConfigID( DBCAMERACONFI *camera,vector<int> &res);
private:
  //  int GetMaxCameraID(void);

public:
    CMyLog m_log;
protected:
    boost::mutex m_db_mutex_;
    std::string m_szFileDB;
    cppsqlite3::CppSQLite3DB m_sqlite3db;
};

#endif // !defined(AFX_DATAINFO_H__45D6DFB6_DE4F_4A45_A1AB_694F51FE2074__INCLUDED_)
