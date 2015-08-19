
#ifndef _DATA_INFO_H_
#define _DATA_INFO_H_

#include "Common.h"
#include "CmdDefine.h"

#include "CppSQLite3.h"


#define SQL_STRING_MAX 4096
#define STRATEGYID_ITSSELF  913156789

#define NEW_CONFIG_DB_500 //5.0

#define sprintf_s sprintf
#define strcpy_s strcpy

#define DB_OK                 0
#define DB_NOT_EXIST          -1
#define DB_PATH_LEN           128
#define DB_PATH               "./DataBase/"
#define DBNAME                "CameraDB.db"
#define DB_CREATE_ERROR       -1


typedef struct _CAMERA_DB_PARAM
{
  int     CameraID;
  char    ip[IP_LEN_16];
  int     Port;
  uint8   frameRate;
  char    CamUrl[SINGLE_URL_LEN_128];
  char    RtspUrl[SINGLE_URL_LEN_128];

  uint8   CameraFunc;
  uint8   AnalyzeNUM;
  uint16  AnalyzeType;
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
	 _STR_TIME_DAY(){
        memset(this, 0, sizeof(_STR_TIME_DAY));
    }

}StrTimeDay;

typedef struct _CMAERA_DB_FUNC_PARAM
{
    int   CameraID;
    char  ip[IP_LEN_16];
    char  CameraUrl[128];
    uint8   AnalyzeNUM;
    uint16  AnalyzeType;
    uint16  MaxHumanNum;
    float   ChangeRate;

    uint16  AnalyzeType1;
    uint8   AnalyzeEn1;

    char    AlarmTime1[295];
    uint16  PkgNum1;
    char*   WatchRegion1;


    uint16  AnalyzeType2;
    uint8   AnalyzeEn2;

    char    AlarmTime2[295];
    uint16  PkgNum2;
    char*   WatchRegion2;

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
    virtual void AreaToDB( char *areaStr, vector < VIDEO_DRAW >  &WatchRegion );
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
//    virtual int getCameraInfo(int iCameraID, char chRemark);
    virtual int getCameraConfig( int cameraid, DBCAMERACONFI *camera );
    virtual int getCameraAlarmInfo( int cameraid, DBCAMERAFUNCPARAM *camera );
    /*********************set***********************/
//    virtual int setCameraInfo(int iCameraID, char chRemark);
    virtual int setCameraConfig(int iCameraID, DBCAMERACONFI *camera );
    virtual int setCameraAlarmInfo(int iCameraID, DBCAMERAFUNCPARAM *camera);
    /*********************add***********************/
//    virtual int AddCameraInfo(int iCameraID, char chRemark);
    virtual int AddCameraConfig(int iCameraID, DBCAMERACONFI *camera );
    virtual int AddCameraAlarmInfo(int iCameraID, DBCAMERAFUNCPARAM *camera);
    /*********************del***********************/
    virtual int DelCameraConfig(int iCamera);
    virtual int DelCameraAlarmInfo(int iCamera);
    virtual int GetMaxCameraID(const char *db_name);
    virtual int getAllCameraConfigID( DBCAMERACONFI *camera,vector<int> &res);
private:

protected:
    boost::mutex m_db_mutex_;
    std::string m_szFileDB;
    cppsqlite3::CppSQLite3DB m_sqlite3db;
};

#endif
