
#include "DataInfo.h"

CamDataInfo::CamDataInfo()
{

}

CamDataInfo::~CamDataInfo()
{

}
void CamDataInfo::ParseAlarmArea( char *str, VIDEO_DRAW *output)
{
    char *p = NULL;
    int index = 1;
    while ( (p = strstr(str, ",")) != NULL )
    {
        *p = 0;
        p ++;

        switch ( index )
        {
        case 1:
            output->StartX = atoi( str );
            break;
        case 2:
            output->StartY = atoi( str );
            break;
        case 3:
            output->EndX = atoi( str );
            break;
        case 4:
            output->EndY = atoi( str );
            break;
        }

        index ++;
        str = p;
    }

    output->Type = atoi( str );
}
void CamDataInfo::AreaToDB(char *areaStr, vector < VIDEO_DRAW >  &WatchRegion )
{
    int size1 = WatchRegion.size();
    char str[1024]={0};
    char str1[100]={0};
    int num = 0;

    for ( int index = 0; index < size1; index ++ )
    {
        int realStartX = WatchRegion[index].StartX;
        int realStartY = WatchRegion[index].StartY;
        int realEndX   = WatchRegion[index].EndX;
        int realEndY   = WatchRegion[index].EndY;
        int type       = WatchRegion[index].Type;

        char buf[1024] = {0};
        sprintf( buf, "%d,%d,%d,%d,%d|", realStartX, realStartY, realEndX, realEndY, type );
        sprintf( str1, "%s", buf );
        strcat(str,str1);
//        snprintf( areaStr, sizeof(buf),"%s", buf );
//        memcpy(areaStr,buf,sizeof(buf));
//        areaStr += strlen(buf);
        num += strlen(buf);
    }
     num--;
     if(str[num] == '|')
         str[num] = 0;
//     *areaStr = str;//

     memcpy(areaStr,str,num+1);
//     str.copy(*areaStr,num+1,0);
}

void CamDataInfo::DBToArea(char *m_areaNode,vector < VIDEO_DRAW >  &WatchRegion)
{
if ( strcmp( m_areaNode,"(null)") == 0 )
 {
     return;
 }


    
    char str[1024] = {0};
    char *p1 = NULL;
    char *p2 = NULL;
    int num = 1;
//       char *str=(char *)m_areaNode.data;
    strncpy( str, m_areaNode, sizeof(str)-1 );
    p2 = str;


    while ( (p1 = strstr(p2, "|")) != NULL )
    {
        p1 ++;
        p2 = p1;
        num ++;
    }

    int size = num * sizeof(VIDEO_DRAW) ;
    char *buf = (char*) malloc( size );
    if ( buf == NULL )
    {
        return;
    }

    memset( buf, 0, size );

    p1 = NULL; p2 = str;
    VIDEO_DRAW area;

    while( (p1 = strstr(p2, "|")) != NULL )
    {
        *p1 = 0;
        p1 ++;

        memset( &area, 0, sizeof(VIDEO_DRAW) );

        this->ParseAlarmArea( p2, &area );
        WatchRegion.push_back(area);
        memcpy( buf, &area, sizeof(VIDEO_DRAW) );
        buf += sizeof(VIDEO_DRAW);

        p2 = p1;

    }

    memset( &area, 0, sizeof(VIDEO_DRAW) );

    this->ParseAlarmArea( p2, &area );
    WatchRegion.push_back(area);
	free(buf);

}
int CamDataInfo::ConvertToTime(char *time,char *t)
{
    char *p1 = NULL;//23:59
    char p2[3];
    char hour = 0;
    char minute = 0;

    if((p1 = strstr(time,":"))!=NULL)
    {
        p2[0] = time[0];
        p2[1] = time[1];
//        memcpy(p2,time,2);
        hour = atoi(p2);
        if(hour < 0 || hour > 24)
        {
            return -1;
        }
    }

    if((p1 = strstr(time,":"))!=NULL)
    {
         p1++;
         minute = atoi(p1);
          if(minute < 0 || minute > 60)
          {
             return -1;
          }
    }
    t[0] = hour;
    t[1] = minute;
    return 0;

}
int CamDataInfo::TimeToConvert(char *t,char *time)
{
    char p[3];
    char buf[10];
    char buf1[10];
    p[0] = t[0];
    p[1] = t[1];

    sprintf(buf,"%02d",p[0]);
    sprintf(buf1,"%02d",p[1]);
    sprintf(time,"%s:%s",buf,buf1);
    return 0;

}
int CamDataInfo::anlyzetime(char *str, StrTimeDay * strtimeday,ALARM_DAY* alarmtime1)
{
    char buf[6]={0};
    char time[3] = {0};
    int num = 0;
    int index = 0;

	
    for(int day = 0; day < WEEK_DAY_LEN_7; day ++)
    {
        num = day * 42;
        for(int session = 0;session < 3; session ++)
        {
            index = session * 14;
            strtimeday[day].Time[session].En = atoi(&str[num]);
            for(int i=0;i<5;i++)
                buf[i] = str[num + index + i + 2];
            memcpy(strtimeday[day].Time[session].StartTime,buf,sizeof(strtimeday[day].Time[session].StartTime));
            for(int i=0;i<5;i++)
                buf[i] = str[num + index + i + 8];
             memcpy(strtimeday[day].Time[session].EndTime,buf,sizeof(strtimeday[day].Time[session].EndTime));
        }
    }

    for(int i = 0; i < WEEK_DAY_LEN_7; i ++)
    {
        alarmtime1[i].En                         = strtimeday[i].Time[0].En + (strtimeday[i].Time[1].En << 1) + (strtimeday[i].Time[2].En << 2);
        ConvertToTime(strtimeday[i].Time[0].StartTime,time);
        alarmtime1[i].dayTime.time[0].Start.hour   = time[0];
        alarmtime1[i].dayTime.time[0].Start.min    = time[1];

        ConvertToTime(strtimeday[i].Time[0].EndTime,time);
        alarmtime1[i].dayTime.time[0].End.hour     = time[0];
        alarmtime1[i].dayTime.time[0].End.min      = time[1];

        ConvertToTime(strtimeday[i].Time[1].StartTime,time);
        alarmtime1[i].dayTime.time[1].Start.hour   = time[0];
        alarmtime1[i].dayTime.time[1].Start.min    = time[1];

        ConvertToTime(strtimeday[i].Time[1].EndTime,time);
        alarmtime1[i].dayTime.time[1].End.hour     = time[0];
        alarmtime1[i].dayTime.time[1].End.min      = time[1];

        ConvertToTime(strtimeday[i].Time[2].StartTime,time);
        alarmtime1[i].dayTime.time[2].Start.hour   = time[0];
        alarmtime1[i].dayTime.time[2].Start.min    = time[1];

        ConvertToTime(strtimeday[i].Time[2].EndTime,time);
        alarmtime1[i].dayTime.time[2].End.hour     = time[0];
        alarmtime1[i].dayTime.time[2].End.min      = time[1];

    }
    return 0;
}
int CamDataInfo::TimeToDB(char *str, StrTimeDay * strtimeday,ALARM_DAY* alarmtime1,char sel)
{
    char hour[3] = {0};
    char min[3]  = {0};
    char time[6] = {0};
    char en[2]      = {0};
    int index = 0;

    memset(str,0,sizeof(str));
    for(int day = 0; day < WEEK_DAY_LEN_7; day ++)
    {
        for(int session = 0;session < 3; session ++)
        {
            sprintf(en,"%d",alarmtime1[day].En);
            strcat(str,en);
            //(alarmtime1[day].En > 0)?strcat(str,"1"):strcat(str,"0");

            index ++;
          //  *(str + index) = '|';
            strcat(str,"|");
            index += 5;
//            t[0] = alarmtime1[day].dayTime.time[session].Start.hour;
//            t[1] = alarmtime1[day].dayTime.time[session].Start.min;
//            TimeToConvert(t,time);
            sprintf(hour,"%02d",alarmtime1[day].dayTime.time[session].Start.hour);
            sprintf(min,"%02d",alarmtime1[day].dayTime.time[session].Start.min);
            sprintf(time,"%s:%s",hour,min);
            strcat(str,time);
            index ++;
            strcat(str,"|");

            index += 5;
            sprintf(hour,"%02d",alarmtime1[day].dayTime.time[session].End.hour);
            sprintf(min,"%02d",alarmtime1[day].dayTime.time[session].End.min);
            sprintf(time,"%s:%s",hour,min);
            strcat(str,time);
            index ++;
            strcat(str,"|");
            index ++;
        }
    }
    index --;
    *(str + index) ='\0';

    return 0;
}

int CamDataInfo::exist(const char* db_name)
{
    char db_tpath[DB_PATH_LEN] = {0};
    char check_result = -1;
    sprintf(db_tpath,"%s%s",DB_PATH,db_name);

    check_result = access(db_tpath,F_OK);
    if(0 == check_result)
    {
        return DB_OK;
    }
    else
    {
        return DB_NOT_EXIST;
    }
}

int CamDataInfo::sqlite3_create_db(const char *db_name)
{

    sqlite3* db = NULL;

    char open_db_result = 0;
    char db_tpath[DB_PATH_LEN] = {0};
    int db_exist_result = 0;
    sprintf(db_tpath,"%s%s",DB_PATH,db_name);

    db_exist_result = exist(db_name);

    if(DB_NOT_EXIST != db_exist_result)
    {
        return DB_OK;
    }

    mkdir(DB_PATH, 0755);

    open_db_result = sqlite3_open(db_tpath,&db);

    if(SQLITE_OK == open_db_result)
    {
        sqlite3_close(db);
        return DB_OK;
    }
    else
    {
        sqlite3_close(db);
        return DB_CREATE_ERROR;
    }
}

int CamDataInfo::sqlite3_create_table(const char* db_name)
{
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    char db_tpath[DB_PATH_LEN] = {0};

    sprintf(db_tpath,"%s%s",DB_PATH,db_name);
    sqlite3_open(db_tpath,&db);
    const char *SQL1="create table CameraInfo (CameraID INTEGER PRIMARY KEY AUTOINCREMENT,IP varchar(16),Port INTEGETER,"
                     "frameRate INTEGETER,CamUrl varchar(128),RtspUrl varchar(128),CameraFunc INTEGETER,"
                     "AnalyzeNUM INTEGETER, AnalyzeType INTEGETER, CamStatus INTEGETER);";

    ret = sqlite3_exec(db,SQL1,0,0,&ErrMsg);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr,"SQL1 Error:%s\n",ErrMsg);
        sqlite3_free(ErrMsg);
    }
    const char *SQL2="create table CameraFuncParam (CameraID INTEGER PRIMARY KEY AUTOINCREMENT,IP varchar(16),CameraUrl varchar(128),AnalyzeNUM INTEGETER,"
                     "AnalyzeType INTEGETER,MaxHumanNum INTEGETER,ChangeRate REAL,AnalyzeType1 INTEGETER,"
                     "AnalyzeEn1 INTEGETER,AlarmTime1 VARCHAR2(295), PkgNum1 INTEGETER, WatchRegion1 varchar(1024),"
                     " AnalyzeType2 INTEGETER,"
                     "AnalyzeEn2 INTEGETER,AlarmTime2 VARCHAR2(295), PkgNum2 INTEGETER, WatchRegion2 varchar(1024));";

    ret = sqlite3_exec(db,SQL2,0,0,&ErrMsg);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr,"SQL2 Error:%s\n",ErrMsg);
        sqlite3_free(ErrMsg);
    }

    sqlite3_close(db);
    return 0;
}

int CamDataInfo::Open(const char* szfilepath)
{
	try
	{
		{
			boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
			m_szFileDB = szfilepath;
			m_sqlite3db.open(szfilepath);
		}

	}
    catch (cppsqlite3::CppSQLite3Exception& e)
	{
        //std::cerr << "CamDataInfo::Open: " << e.errorCode() << ":" << e.errorMessage() << std::endl;
      //  m_log.Add("CamDataInfo::Open: %d : %s ", e.errorCode(), e.errorMessage());
	}
	return 0;
}
int CamDataInfo::GetMaxCameraID(const char *db_name)
{
    int ret;
    char *ErrMsg=0;
    sqlite3* db = NULL;
    char db_tpath[DB_PATH_LEN] = {0};

    sprintf(db_tpath,"%s%s",DB_PATH,db_name);
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));

    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

        sqlite3_open(db_tpath,&db);

        const char *SQL1 = "SELECT COUNT (*) as CNT FROM sqlitemaster where type = 'table' and name = 'CameraInfo'";
        ret = sqlite3_exec(db,SQL1,0,0,&ErrMsg);
        if(ret == 0)
        {
            fprintf(stderr,"SQL Error:%s\n",ErrMsg);
            sqlite3_free(ErrMsg);
            sqlite3_close(db);
            return 0;
        }
		sqlite3_free(ErrMsg);  //nikola
        sqlite3_close(db); //nikola
    }
    catch (cppsqlite3::CppSQLite3Exception& e)
    {
        return -1;
    }
    return 0;
}

int select_callback(void *data,int col_count,char **col_values,char **col_name)
{
//    int i;
//    for(i=0;i<col_count;i++)
//    {
 //       printf("%s=%s\n",col_name[i],col_values[i]==0?"NULL":col_values[i]);
 //   }
    return 0;
}

int CamDataInfo::getCameraConfig( int cameraid, DBCAMERACONFI *camera )
{
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    char **pszResult = 0;
    int nRow = 0;
    int nColumn = 0;
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));

	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        sqlite3_open("./DataBase/CameraDB.db",&db);

        sprintf_s(SqlBuf,"select CameraID, IP, Port, frameRate, CamUrl,"
            "RtspUrl, CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus "
            "from CameraInfo where CameraID = %d ", cameraid);

        ret = sqlite3_get_table(db, SqlBuf, &pszResult, &nRow, &nColumn, &ErrMsg);
        if (ret != SQLITE_OK)
        {
            std::cout << "SQL error " << ErrMsg << std::endl;
            sqlite3_free(ErrMsg);
            ErrMsg = NULL;
            sqlite3_free_table(pszResult);
            sqlite3_close(db);
            return -1;
        }
        else
        {
            if((nRow == 0)&&(nColumn == 0))
            {
                sqlite3_close(db);
                return 0;
            }
            for (int i = 0; i <= nRow; ++i)
            {
                camera->CameraID = atoi(*(pszResult + nColumn * i + 0));
                strcpy(camera->ip, *(pszResult + nColumn * i  + 1));
                camera->Port = atoi(*(pszResult + nColumn * i  + 2));
                camera->frameRate = atoi(*(pszResult + nColumn * i + 3));
                strcpy(camera->CamUrl, *(pszResult + nColumn * i  + 4));
                strcpy(camera->RtspUrl, *(pszResult + nColumn * i  + 5));
                camera->CameraFunc = atoi(*(pszResult + nColumn * i + 6));
                camera->AnalyzeNUM = atoi(*(pszResult + nColumn * i + 7));
                camera->AnalyzeType = atoi(*(pszResult + nColumn * i + 8));
                camera->CamStatus = atoi(*(pszResult + nColumn * i + 9));
            }
        }
        if (ErrMsg != NULL) sqlite3_free(ErrMsg);
        ErrMsg = NULL;
        //sqlite3_free_table(pszResult);
        sqlite3_close(db);
        return 0;
	}
    catch(cppsqlite3::CppSQLite3Exception& e)
	{
//		std::cerr << "CamDataInfo::getCameraInfo" << e.errorCode() << ":" << e.errorMessage() << std::endl;
//		m_log.Add("CamDataInfo::getCameraInfo: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
}
int CamDataInfo::getAllCameraConfigID( DBCAMERACONFI *camera,vector<int> &res)
{
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    char **pszResult = 0;
    int nRow = 0;
    int nColumn = 0;
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));

    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        sqlite3_open("./DataBase/CameraDB.db",&db);

        sprintf_s(SqlBuf,"select * from CameraInfo;");
        ret = sqlite3_get_table(db, SqlBuf, &pszResult, &nRow, &nColumn, &ErrMsg);

        if (ret != SQLITE_OK)
        {
            sqlite3_free(ErrMsg);
            ErrMsg = NULL;
            sqlite3_free_table(pszResult);
            sqlite3_close(db);

            return -1;
        }
        else
        {

            if((nRow == 0)&&(nColumn == 0))
            {
                sqlite3_close(db);

                return 0;
            }
            
            for (int i = 1; i <= nRow; ++i)
            {
		res.push_back(atoi(*(pszResult + nColumn * i + 0)));
            }
        }
        if (ErrMsg != NULL) sqlite3_free(ErrMsg);
        ErrMsg = NULL;
        //sqlite3_free_table(pszResult);
        sqlite3_close(db);
        return 0;
    }
    catch(cppsqlite3::CppSQLite3Exception& e)
    {
        return -1;
    }
}

int CamDataInfo::getCameraAlarmInfo( int cameraid, DBCAMERAFUNCPARAM *camera )
{

    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    char **pszResult = 0;
    int nRow = 0;
    int nColumn = 0;
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));

    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        sqlite3_open("./DataBase/CameraDB.db",&db);

        sprintf_s(SqlBuf,"select CameraID, IP, CameraUrl,AnalyzeNUM, AnalyzeType, MaxHumanNUM,"
            "ChangeRate, AnalyzeType1, AnalyzeEn1, AlarmTime1, PkgNum1, WatchRegion1, "
            "AnalyzeType2, AnalyzeEn2, AlarmTime2, PkgNum2, WatchRegion2 from CameraFuncParam where CameraID = %d ", cameraid);

        ret = sqlite3_get_table(db, SqlBuf, &pszResult, &nRow, &nColumn, &ErrMsg);
        if (ret != SQLITE_OK)
        {
            sqlite3_free(ErrMsg);
            ErrMsg = NULL;
            sqlite3_free_table(pszResult);
            sqlite3_close(db);
            return -1;
        }
        else
        {
            if((nRow == 0)&&(nColumn == 0))
            {
                sqlite3_close(db);
                return 0;
            }
            for (int i = 0; i <= nRow; ++i)
            {
                camera->CameraID = atoi(*(pszResult + nColumn * i + 0));
                strcpy(camera->ip, *(pszResult + nColumn * i  + 1));
                strcpy(camera->CameraUrl, *(pszResult + nColumn * i  + 2));
                camera->AnalyzeNUM = atoi(*(pszResult + nColumn * i  + 3));
                camera->AnalyzeType = atoi(*(pszResult + nColumn * i + 4));
                camera->MaxHumanNum = atoi(*(pszResult + nColumn * i + 5));
                camera->ChangeRate = atof(*(pszResult + nColumn * i + 6));
                camera->AnalyzeType1 = atoi(*(pszResult + nColumn * i + 7));
                camera->AnalyzeEn1 = atoi(*(pszResult + nColumn * i + 8));

                memset(camera->AlarmTime1, 0, sizeof(camera->AlarmTime1));
                strcpy(camera->AlarmTime1,*(pszResult + nColumn * i  + 9));
                camera->PkgNum1 = atoi(*(pszResult + nColumn * i + 10));

                memset(&camera->WatchRegion1, 0, sizeof(camera->WatchRegion1));
                memcpy(&camera->WatchRegion1, (pszResult + nColumn * i  + 11), sizeof(camera->WatchRegion1));

                camera->AnalyzeType2 = atoi(*(pszResult + nColumn * i + 12));
                camera->AnalyzeEn2 = atoi(*(pszResult + nColumn * i + 13));

                memset(camera->AlarmTime2, 0, sizeof(camera->AlarmTime2));
                strcpy(camera->AlarmTime2,*(pszResult + nColumn * i  + 14));
                camera->PkgNum2 = atoi(*(pszResult + nColumn * i + 15));

                memset(&camera->WatchRegion2, 0, sizeof(camera->WatchRegion2));
                memcpy(&camera->WatchRegion2, (pszResult + nColumn * i  + 16), sizeof(camera->WatchRegion2));
            }
        }
        if (ErrMsg != NULL) 
		{	
			sqlite3_free(ErrMsg);
        	ErrMsg = NULL;
        }
        //sqlite3_free_table(pszResult);
        sqlite3_close(db);
        return 0;
    }
    catch(cppsqlite3::CppSQLite3Exception& e)
    {
        return -1;
    }


}

int CamDataInfo::setCameraConfig( int cameraid, DBCAMERACONFI *camera )
{
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));

    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        sqlite3_open("./DataBase/CameraDB.db",&db);

        sprintf_s(SqlBuf,"update CameraInfo set IP = '%s', Port = %d,frameRate = %d, CamUrl = '%s',RtspUrl = '%s' ,CameraFunc = %d,"
                         "AnalyzeNUM = %d,AnalyzeType = %d,CamStatus =%d where CameraID = %d;",
            camera->ip, camera->Port,camera->frameRate,camera->CamUrl,camera->RtspUrl,camera->CameraFunc,camera->AnalyzeNUM,camera->AnalyzeType,camera->CamStatus,cameraid);
//        sprintf_s(SqlBuf,"REPLACE INTO CameraInfo (CameraID, IP, Port, frameRate, CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus) VALUES (%d, %d, %d, %d, %d, %d, %d);",
//            camera->CameraID, camera->ip, camera->Port,camera->frameRate,camera->CameraFunc,camera->AnalyzeNUM,camera->CamStatus,);
//        return m_sqlite3db.execDML( SqlBuf );
         ret = sqlite3_exec(db,SqlBuf,0,0,&ErrMsg);
		 sqlite3_close(db);// nikola

         return ret;
    }
    catch(cppsqlite3::CppSQLite3Exception& e)
    {
        return -1;
    }
}

int CamDataInfo::setCameraAlarmInfo( int cameraid, DBCAMERAFUNCPARAM *camera )
{
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;

    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));
    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
//        ContentValues cv = new ContentValues();
//        cv.put("CameraID","camera->CameraID");
		sqlite3_open("./DataBase/CameraDB.db",&db);

        camera->CameraID = cameraid;
        sprintf_s(SqlBuf,"update CameraFuncParam set IP = '%s', CameraUrl = '%s',AnalyzeNUM = %d,"
                         "AnalyzeType = %d,MaxHumanNUM =%d ,ChangeRate = %f,"
                         "AnalyzeType1 = %d, AnalyzeEn1 = %d,AlarmTime1 = '%s',PkgNum1 = %d,WatchRegion1 = '%s',"
                         "AnalyzeType2 = %d, AnalyzeEn2 = %d,AlarmTime2 = '%s',PkgNum2 = %d,WatchRegion2 = '%s'   where CameraID = %d;",
            camera->ip, camera->CameraUrl,camera->AnalyzeNUM,camera->AnalyzeType,camera->MaxHumanNum,camera->ChangeRate,
            camera->AnalyzeType1,camera->AnalyzeEn1,camera->AlarmTime1,camera->PkgNum1,camera->WatchRegion1,
            camera->AnalyzeType2,camera->AnalyzeEn2,camera->AlarmTime2,camera->PkgNum2,camera->WatchRegion2,camera->CameraID);
//        sprintf_s(SqlBuf,"REPLACE INTO CameraInfo (CameraID, IP, Port, frameRate, CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus) VALUES (%d, %d, %d, %d, %d, %d, %d);",
//            camera->CameraID, camera->ip, camera->Port,camera->frameRate,camera->CameraFunc,camera->AnalyzeNUM,camera->CamStatus,);
    //    return m_sqlite3db.execDML( SqlBuf );
       ret = sqlite3_exec(db,SqlBuf,0,0,&ErrMsg);
                sqlite3_close(db);
        return ret;
    }
    catch(cppsqlite3::CppSQLite3Exception& e)
    {
        return -1;
    }
}

int CamDataInfo::AddCameraConfig( int cameraid, DBCAMERACONFI *camera )
{
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        sqlite3_open("./DataBase/CameraDB.db",&db);

//        sprintf_s(SqlBuf,"REPLACE INTO CameraInfo (CameraID, IP, Port, frameRate, CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus) VALUES (%d, %d, %d, %d, %d, %d, %d);",
//            camera->CameraID, camera->ip, camera->Port,camera->frameRate,camera->CameraFunc,camera->AnalyzeNUM,camera->CamStatus,);
        camera->CameraID=cameraid;
        if(cameraid == 0)
        {
            sprintf_s(SqlBuf,"insert into CameraInfo (IP, Port, frameRate, CamUrl, RtspUrl ,CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus) values ('%s', %d, %d,'%s','%s', %d, %d, %d, %d);",
                camera->ip, camera->Port,camera->frameRate,camera->CamUrl,camera->RtspUrl,camera->CameraFunc,camera->AnalyzeNUM,camera->AnalyzeType,camera->CamStatus);
        }
        else
        {
                    sprintf_s(SqlBuf,"insert into CameraInfo (CameraID, IP, Port, frameRate, CamUrl, RtspUrl ,CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus) values (%d,'%s', %d, %d,'%s','%s', %d, %d, %d, %d);",
                        camera->CameraID, camera->ip, camera->Port,camera->frameRate,camera->CamUrl,camera->RtspUrl,camera->CameraFunc,camera->AnalyzeNUM,camera->AnalyzeType,camera->CamStatus);
        }

//        return m_sqlite3db.execDML( SqlBuf );
        ret = sqlite3_exec(db,SqlBuf,0,0,&ErrMsg);
         sqlite3_close(db);
         return ret;
    }
    catch(cppsqlite3::CppSQLite3Exception& e)
    {
    	printf("ssssssssssssssssssssss\n");
        return -1;
    }
}

int CamDataInfo::AddCameraAlarmInfo( int cameraid, DBCAMERAFUNCPARAM *camera )
{
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        sqlite3_open("./DataBase/CameraDB.db",&db);
        camera->CameraID=cameraid;
        if(cameraid == 0)
        {
            sprintf_s(SqlBuf,"insert into CameraFuncParam (IP, CameraUrl,AnalyzeNUM, AnalyzeType, MaxHumanNUM, ChangeRate,"
                             "AnalyzeType1, AnalyzeEn1, AlarmTime1, PkgNum1, WatchRegion1,"
                             "AnalyzeType2, AnalyzeEn2, AlarmTime2, PkgNum2, WatchRegion2) VALUES ('%s', '%s', %d, %d, %d, %f, %d, %d,'%s', %d, '%s', %d, %d,'%s',%d,'%s');",
                camera->ip, camera->CameraUrl,camera->AnalyzeNUM,camera->AnalyzeType,camera->MaxHumanNum,camera->ChangeRate,
                camera->AnalyzeType1,camera->AnalyzeEn1,camera->AlarmTime1,camera->PkgNum1,camera->WatchRegion1,
                camera->AnalyzeType2,camera->AnalyzeEn2,camera->AlarmTime2,camera->PkgNum2,camera->WatchRegion2);
        }
        else
        {
            sprintf_s(SqlBuf,"insert into CameraFuncParam (CameraID, IP, CameraUrl, AnalyzeNUM, AnalyzeType, MaxHumanNUM, ChangeRate,"
                             "AnalyzeType1, AnalyzeEn1, AlarmTime1, PkgNum1, WatchRegion1,"
                             "AnalyzeType2, AnalyzeEn2, AlarmTime2, PkgNum2, WatchRegion2) VALUES (%d, '%s', '%s', %d, %d, %d, %f, %d, %d,'%s', %d, '%s', %d, %d,'%s',%d,'%s');",
                camera->CameraID, camera->ip, camera->CameraUrl,camera->AnalyzeNUM,camera->AnalyzeType,camera->MaxHumanNum,camera->ChangeRate,
                camera->AnalyzeType1,camera->AnalyzeEn1,camera->AlarmTime1,camera->PkgNum1,camera->WatchRegion1,
                camera->AnalyzeType2,camera->AnalyzeEn2,camera->AlarmTime2,camera->PkgNum2,camera->WatchRegion2);
        }


//        return m_sqlite3db.execDML( SqlBuf );
          ret = sqlite3_exec(db,SqlBuf,0,0,&ErrMsg);
          sqlite3_close(db);
          return ret;
    }
    catch(cppsqlite3::CppSQLite3Exception& e)
    {
    	printf("db---AddCameraAlarmInfo\n");
        return -1;
    }
}



int CamDataInfo::DelCameraConfig(int iCamera)
{
//    SQLiteDatabase db = databaseHelper.getWritableDatabase();
//    db.delete("person", "personid<?", new String[]{"2"});
//    db.close();
	char SqlBuf[SQL_STRING_MAX];
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        sqlite3_open("./DataBase/CameraDB.db",&db);
        sprintf_s(SqlBuf,"Delete from CameraInfo where CameraID = %d;", iCamera);
//		return m_sqlite3db.execDML( SqlBuf );
        ret = sqlite3_exec(db,SqlBuf,0,0,&ErrMsg);
        sqlite3_close(db);
        return ret;
	}
    catch (cppsqlite3::CppSQLite3Exception& e)
	{
//        m_log.Add("CamDataInfo::DelCameraInfo: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
}
int CamDataInfo::DelCameraAlarmInfo(int iCamera)
{
    char SqlBuf[SQL_STRING_MAX];
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    memset(SqlBuf,0,sizeof(SqlBuf));
    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        sqlite3_open("./DataBase/CameraDB.db",&db);
        sprintf_s(SqlBuf,"Delete from CameraFuncParam where CameraID = %d;", iCamera);
//		return m_sqlite3db.execDML( SqlBuf );
        ret = sqlite3_exec(db,SqlBuf,0,0,&ErrMsg);
        sqlite3_close(db);
        return ret;
    }
    catch (cppsqlite3::CppSQLite3Exception& e)
    {
        return -1;
    }
}
