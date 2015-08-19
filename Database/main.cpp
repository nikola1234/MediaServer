#include<stdio.h>
#include "sqlite3.h"
#include "DataInfo.h"

int main(void)
{
    sqlite3 *db = NULL;
    char *zErrMsg = 0;
    int rc;
    int ret = 0;
    CamDataInfo CDataInfo;
    DBCAMERACONFI camera;
    DBCAMERAFUNCPARAM cameraparam;
    vector<int> id_res;
    //open db ,if not exist create a new one
    rc = CDataInfo.sqlite3_create_db(DBNAME);
    CDataInfo.sqlite3_create_table(DBNAME);
    if(rc)
    {
       fprintf(stderr,"can't open database:%s\n",sqlite3_errmsg(db));
       sqlite3_close(db);
    }
    else
    {

        {
            camera.CameraID=7;
            //camera->ip="123456789012345";
            memcpy(camera.ip,"192.168.1.125",sizeof(camera.ip));
            camera.Port=33;
            camera.frameRate=4;
            memcpy(camera.CamUrl,"http://www.baidu.com",sizeof(camera.CamUrl));
            memcpy(camera.RtspUrl,"http://www.sohu.com",sizeof(camera.RtspUrl));
            camera.CameraFunc=5;
            camera.AnalyzeNUM=6;
            camera.AnalyzeType=7;
            camera.CamStatus = 8;
        }
        printf("AddCameraConfig = %d\n",CDataInfo.AddCameraConfig(NULL,&camera));
        printf("AddCameraConfig = %d\n",CDataInfo.AddCameraConfig(NULL,&camera));
        memset(&camera,0,sizeof(camera));
        printf("getCameraConfig = %d\n",CDataInfo.getCameraConfig(1,&camera));
        printf("camera.ip=%s\n",camera.ip);
        memcpy(camera.ip,"192.168.1.135",sizeof(camera.ip));
        printf("setCameraConfig = %d\n",CDataInfo.setCameraConfig( 1, &camera ));
        printf("AddCameraConfig = %d\n",CDataInfo.AddCameraConfig(NULL,&camera));
        printf("MaxCameraID = %d\n",CDataInfo.GetMaxCameraID(DBNAME));
        CDataInfo.getAllCameraConfigID(&camera,id_res);

        StrTimeDay strtimeday[7];
        ALARM_DAY  alarmtime1[7];
        VIDEO_DRAW watchregion1;
        vector < VIDEO_DRAW >  WatchRegion1;

        {
            cameraparam.CameraID=1;
            memcpy(cameraparam.ip,"http://www.ss",16);
            cameraparam.AnalyzeNUM=3;
            cameraparam.AnalyzeType=4;
            cameraparam.MaxHumanNum=5;
            cameraparam.ChangeRate=6;
            cameraparam.AnalyzeType1=7;
            cameraparam.AnalyzeEn1=8;
            char *str ="1|00:00|00:01|1|01:00|01:01|1|01:10|01:11|2|20:00|20:01|2|21:00|21:01|2|21:10|21:11|3|23:00|00:01|3|23:59|23:01|3|01:10|01:11|"
                       "1|00:00|00:01|1|01:00|01:01|1|01:10|01:11|1|00:00|00:01|1|01:00|01:01|1|01:10|01:11|1|00:00|00:01|1|01:00|01:01|1|01:10|01:11|"
                       "1|00:00|00:01|1|01:00|01:01|1|01:10|01:11";
            cameraparam.WatchRegion1 = "0,0,0,0,1|1024,1024,0,0,2|0,0,1024,1024,1|1024,1024,104,1024,2";

            cameraparam.AnalyzeType2=11;
            cameraparam.AnalyzeEn2=12;
            memcpy(cameraparam.AlarmTime1,str,sizeof(cameraparam.AlarmTime1));
            memcpy(cameraparam.AlarmTime2,cameraparam.AlarmTime1,sizeof(cameraparam.AlarmTime2));
            cameraparam.PkgNum1=13;
            cameraparam.PkgNum2=14;
            memcpy(&cameraparam.WatchRegion2,&cameraparam.WatchRegion1,sizeof(cameraparam.WatchRegion2));
            CDataInfo.DBToArea(cameraparam.WatchRegion1 ,WatchRegion1);
        }
        printf("AddAlarmInfo1 = %d\n",CDataInfo.AddCameraAlarmInfo(NULL,&cameraparam));

        {
                for(int day = 0; day < WEEK_DAY_LEN_7; day ++)
                {
                    for(int session = 0;session < 3; session ++)
                    {
                        alarmtime1[day].En = day;
                        alarmtime1[day].dayTime.time[session].Start.hour = 0;
                        alarmtime1[day].dayTime.time[session].Start.min  = 0;

                        alarmtime1[day].dayTime.time[session].End.hour = 23;
                        alarmtime1[day].dayTime.time[session].End.min  = 59;
                    }
                }
                char str1[295];
                CDataInfo.TimeToDB(str1, NULL, alarmtime1, 0);
                memcpy(cameraparam.AlarmTime1,str1,sizeof(cameraparam.AlarmTime1));
            //    //ret = CDataInfo.anlyzetime(time, cameraparam.AlarmTime1);
                //    memset(cameraparam.WatchRegion1,0,sizeof(cameraparam.WatchRegion1));
            //        cameraparam.WatchRegion1 = NULL;
//                  memset(cameraparam.WatchRegion1,0,sizeof(cameraparam.WatchRegion1));
                  CDataInfo.AreaToDB(&cameraparam.WatchRegion1,WatchRegion1);
                 //   CDataInfo.anlyzetime(str, strtimeday,alarmtime1);

        }
        printf("AddAlarmInfo2 = %d\n",CDataInfo.AddCameraAlarmInfo(NULL,&cameraparam));
        memset(&cameraparam,0,sizeof(cameraparam));
        printf("getalarmAlarmInfo = %d\n",CDataInfo.getCameraAlarmInfo(1,&cameraparam));
      //
        memset(alarmtime1,0,sizeof(alarmtime1));
        CDataInfo.anlyzetime(cameraparam.AlarmTime1, strtimeday,alarmtime1);
        printf("delConfig1 = %d\n",CDataInfo.DelCameraConfig(1));
        printf("delConfig2 = %d\n",CDataInfo.DelCameraAlarmInfo(1));

        printf("open success\n");
    }


    return 0;
}
