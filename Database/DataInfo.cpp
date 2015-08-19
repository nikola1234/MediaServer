// DataInfo.cpp: implementation of the CamDataInfo class.
//////////////////////////////////////////////////////////////////////

#include "boost/thread.hpp"
#include "boost/filesystem.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
//#include <string>
namespace fs = boost::filesystem;

#include "DataInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CamDataInfo::CamDataInfo()
{
	m_log.InitLog("./log/RecvDB ");
}

CamDataInfo::~CamDataInfo()
{
//	StopRecvCommand((int)this);
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
void CamDataInfo::AreaToDB(char **areaStr, vector < VIDEO_DRAW >  &WatchRegion )
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

        char buf[1024] = {0};		//����  ��ʼX���ꡢY���꣬����
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
     *areaStr = str;
}

void CamDataInfo::DBToArea(char *m_areaNode,vector < VIDEO_DRAW >  &WatchRegion)
{
//    char *p=str.c_str();
    if ( strlen( m_areaNode) == 0 )
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

    //���ж� �ܹ��������ٸ� "|"���ȹ��ж��ٸ�
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

}
int CamDataInfo::ConvertToTime(char *time,char *t)
{
    char *p1 = NULL;//23:59
    char p2[3];
    char hour;
    char minute;

    if((p1 = strstr(time,":"))!=NULL)
    {
        p2[0] = time[0];
        p2[1] = time[1];
//        memcpy(p2,time,2);
        hour = atoi(p2);
        if(hour < 0 || hour > 24)
        {
            printf("hour error");
            return -1;
        }
    }

    if((p1 = strstr(time,":"))!=NULL)
    {
         p1++;
         minute = atoi(p1);
          if(minute < 0 || minute > 60)
          {
             printf("minute error");
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
//    char t[3]    ={0};
    char en[2]      = {0};
    int index = 0;
   // char *str =NULL;
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
    printf("check db exist: %s%s\n",DB_PATH,db_name);

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
    int mkdir_return = 0 ;
    sprintf(db_tpath,"%s%s",DB_PATH,db_name);

    db_exist_result = exist(db_name);

    if(DB_NOT_EXIST != db_exist_result)
    {
        return DB_OK;
    }
    printf("create db-------------------------- :%s%s\n",DB_PATH,db_name);
    printf("make dir:%s",DB_PATH);
    mkdir_return = mkdir(DB_PATH, 0755);

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
    char open_db_result = 0;
    char db_tpath[DB_PATH_LEN] = {0};

    sprintf(db_tpath,"%s%s",DB_PATH,db_name);
    open_db_result = sqlite3_open(db_tpath,&db);
    const char *SQL1="create table CameraInfo (CameraID INTEGER PRIMARY KEY AUTOINCREMENT,IP varchar(16),Port INTEGETER,"
                     "frameRate INTEGETER,CamUrl varchar(128),RtspUrl varchar(128),CameraFunc INTEGETER,"
                     "AnalyzeNUM INTEGETER, AnalyzeType INTEGETER, CamStatus INTEGETER);";
//ִ�н���
    ret = sqlite3_exec(db,SQL1,0,0,&ErrMsg);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr,"SQL1 Error:%s\n",ErrMsg);
        sqlite3_free(ErrMsg);
    }
    const char *SQL2="create table CameraFuncParam (CameraID INTEGER PRIMARY KEY AUTOINCREMENT,IP varchar(16),AnalyzeNUM INTEGETER,"
                     "AnalyzeType INTEGETER,MaxHumanNum INTEGETER,ChangeRate REAL,AnalyzeType1 INTEGETER,"
                     "AnalyzeEn1 INTEGETER,AlarmTime1 VARCHAR2(295), PkgNum1 INTEGETER, WatchRegion1 varchar(1024),"
                     " AnalyzeType2 INTEGETER,"
                     "AnalyzeEn2 INTEGETER,AlarmTime2 VARCHAR2(295), PkgNum2 INTEGETER, WatchRegion2 varchar(1024));";
//ִ�н���
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
    char open_db_result = 0;
    char db_tpath[DB_PATH_LEN] = {0};

    sprintf(db_tpath,"%s%s",DB_PATH,db_name);
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));

    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

        open_db_result = sqlite3_open(db_tpath,&db);

        const char *SQL1 = "SELECT COUNT (*) as CNT FROM sqlitemaster where type = 'table' and name = 'CameraInfo'";
        ret = sqlite3_exec(db,SQL1,0,0,&ErrMsg);
        if(ret == 0)
        {
            fprintf(stderr,"SQL Error:%s\n",ErrMsg);
            sqlite3_free(ErrMsg);
            sqlite3_close(db);
            return 0;

        }
    }
    catch (cppsqlite3::CppSQLite3Exception& e)
    {
//		m_log.Add("CDataInfo::GetMaxCameraID: %d : %s ", e.errorCode(), e.errorMessage());
        return -1;
    }
}

int CamDataInfo::getCameraInfo(int iCameraID, char chRemark)
{
    return 0;
}
int CamDataInfo::setCameraInfo(int iCameraID, char chRemark)
{
    return 0;
}
int CamDataInfo::AddCameraInfo(int iCameraID, char chRemark)
{
    return 0;
}
int select_callback(void *data,int col_count,char **col_values,char **col_name)
{
    //ÿ����¼�ص�һ�θú������ж������ͻص����ٴ�
    int i;
    for(i=0;i<col_count;i++)
    {
        printf("%s=%s\n",col_name[i],col_values[i]==0?"NULL":col_values[i]);
//        if(col_name[i]=="CameraID")
//            camera->CameraID = atoi(col_values[i]);
//        else if(col_name[i]=="CameraID")
//            strcpy(camera->ip, col_values[i]);
//        else if(col_name[i]=="Port")
//            camera->Port = atoi(col_values[i]);
//        else if(col_name[i]=="frameRate")
//            camera->frameRate = atoi(col_values[i]);
//        else if(col_name[i]=="CameraFunc")
//            camera->CameraFunc = atoi(col_values[i]);
//        else if(col_name[i]=="AnalyzeNUM")
//            camera->AnalyzeNUM = atoi(col_values[i]);
//        else if(col_name[i]=="AnalyzeType")
//            camera->AnalyzeType = atoi(col_values[i]);
//        else if(col_name[i]=="CamStatus")
//            camera->CamStatus = atoi(col_values[i]);
    }
    return 0;
}
int CamDataInfo::getCameraConfig( int cameraid, DBCAMERACONFI *camera )
{
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    char open_db_result = 0;
    char **pszResult = 0;
    int nRow = 0;
    int nColumn = 0;
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));

	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        open_db_result = sqlite3_open("./DataBase/CameraDB.db",&db);

        sprintf_s(SqlBuf,"select CameraID, IP, Port, frameRate, CamUrl,"
            "RtspUrl, CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus "
            "from CameraInfo where CameraID = %d ", cameraid);
//         sprintf_s(SqlBuf,"select * from CameraInfo;");
//		camera->CameraID = cameraid;
//        //��ѯ���ݱ�����
//        printf("query\n");
//        sqlite3_exec(db,SqlBuf,select_callback,data,&ErrMsg);
        // ��ȡ����sql����ָ���ļ�¼��
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
            // �˼�¼���ɹ���ȡ��,��ǰ����nColume��Ԫ��������
            // ����,����ѭ����ȡ��¼�ǣ������Ǳȷ��ص�������һ��,��������
            printf("������%d ��¼!\n", nRow);
            printf("������%d ��!\n", nColumn);
            if((nRow == 0)&&(nColumn == 0))
            {
                sqlite3_close(db);
                printf("DB is empty\n");
                return 0;
            }
            for (int i = 0; i <= nRow; ++i)
            {
                for (int j = 0; j < nColumn; ++j)
                {
                    std::cout << *(pszResult + nColumn * i + j) << "\t";
                }

                camera->CameraID = atoi(*(pszResult + nColumn * i + 0));
                strcpy(camera->ip, *(pszResult + nColumn * i  + 1));
                camera->Port = atoi(*(pszResult + nColumn * i  + 2));
                camera->frameRate = atoi(*(pszResult + nColumn * i + 3));
                camera->CameraFunc = atoi(*(pszResult + nColumn * i + 6));
                camera->AnalyzeNUM = atoi(*(pszResult + nColumn * i + 7));
                camera->AnalyzeType = atoi(*(pszResult + nColumn * i + 8));
                camera->CamStatus = atoi(*(pszResult + nColumn * i + 9));
                std::cout << std::endl;
            }
        }
        if (ErrMsg != NULL) sqlite3_free(ErrMsg);
        ErrMsg = NULL;
        sqlite3_free_table(pszResult);
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
    char open_db_result = 0;
    char **pszResult = 0;
    int nRow = 0;
    int nColumn = 0;
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));

    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        open_db_result = sqlite3_open("./DataBase/CameraDB.db",&db);

//        sprintf_s(SqlBuf,"select CameraID from CameraInfo where CameraID = %d ", cameraid);
        sprintf_s(SqlBuf,"select * from CameraInfo;");
//		camera->CameraID = cameraid;
//        //��ѯ���ݱ�����
//        printf("query\n");
//        sqlite3_exec(db,SqlBuf,select_callback,data,&ErrMsg);
        // ��ȡ����sql����ָ���ļ�¼��
        ret = sqlite3_get_table(db, SqlBuf, &pszResult, &nRow, &nColumn, &ErrMsg);
        if (ret != SQLITE_OK)
        {
            std::cout << "SQL error " << ErrMsg << std::endl;
//            sqlite3_free(ErrMsg);
//            ErrMsg = NULL;
//            sqlite3_free_table(pszResult);
//            sqlite3_close(db);
            return -1;
        }
        else
        {
            printf("������%d ��¼!\n", nRow);
            printf("������%d ��!\n", nColumn);
            if((nRow == 0)&&(nColumn == 0))
            {
                sqlite3_close(db);
                printf("DB is empty\n");
                return 0;
            }
            for (int i = 0; i <= nRow; ++i)
            {
                for (int j = 0; j < nColumn; ++j)
                {
                    std::cout << *(pszResult + nColumn * i + j) << "\t";
                }

                camera->CameraID = atoi(*(pszResult + nColumn * i + 0));
                res.push_back(camera->CameraID);
                std::cout << std::endl;
            }
        }
        if (ErrMsg != NULL) sqlite3_free(ErrMsg);
        ErrMsg = NULL;
        sqlite3_free_table(pszResult);
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

int CamDataInfo::getCameraAlarmInfo( int cameraid, DBCAMERAFUNCPARAM *camera )
{

    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    char open_db_result = 0;
    char **pszResult = 0;
    int nRow = 0;
    int nColumn = 0;
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));

    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        open_db_result = sqlite3_open("./DataBase/CameraDB.db",&db);

//        sprintf_s(SqlBuf,"select CameraID, IP, Port, frameRate, CamUrl,"
//            "RtspUrl, CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus "
//            "from CameraInfo where CameraID = %d ", cameraid);

        sprintf_s(SqlBuf,"select CameraID, IP, AnalyzeNUM, AnalyzeType, MaxHumanNUM,"
            "ChangeRate, AnalyzeType1, AnalyzeEn1, AlarmTime1, PkgNum1, WatchRegion1, "
            "AnalyzeType2, AnalyzeEn2, AlarmTime2, PkgNum2, WatchRegion2 from CameraFuncParam where CameraID = %d ", cameraid);

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
            // �˼�¼���ɹ���ȡ��,��ǰ����nColume��Ԫ��������
            // ����,����ѭ����ȡ��¼�ǣ������Ǳȷ��ص�������һ��,��������
            printf("������%d ��¼!\n", nRow);
            printf("������%d ��!\n", nColumn);
            if((nRow == 0)&&(nColumn == 0))
            {
                sqlite3_close(db);
                printf("DB is empty\n");
                return 0;
            }
            for (int i = 0; i <= nRow; ++i)
            {
                for (int j = 0; j < nColumn; ++j)
                {
                    std::cout << *(pszResult + nColumn * i + j) << "\t";
                }
	
				printf("sssssssssss  is    %s\n",*(pszResult + nColumn * i + 0));
                camera->CameraID = atoi(*(pszResult + nColumn * i + 0));
				printf("ip is %s\n",*(pszResult + nColumn * i  + 1));
                strcpy(camera->ip, *(pszResult + nColumn * i  + 1));
                camera->AnalyzeNUM = atoi(*(pszResult + nColumn * i  + 2));
                camera->AnalyzeType = atoi(*(pszResult + nColumn * i + 3));
                camera->MaxHumanNum = atoi(*(pszResult + nColumn * i + 4));
                camera->ChangeRate = atof(*(pszResult + nColumn * i + 5));
                camera->AnalyzeType1 = atoi(*(pszResult + nColumn * i + 6));
                camera->AnalyzeEn1 = atoi(*(pszResult + nColumn * i + 7));

                memset(camera->AlarmTime1, 0, sizeof(camera->AlarmTime1));
                strcpy(camera->AlarmTime1,*(pszResult + nColumn * i  + 8));
                camera->PkgNum1 = atoi(*(pszResult + nColumn * i + 9));

                memset(&camera->WatchRegion1, 0, sizeof(camera->WatchRegion1));
//                strcpy(&camera->WatchRegion1,*(pszResult + nColumn * i  + 10));
                memcpy(&camera->WatchRegion1, (pszResult + nColumn * i  + 10), sizeof(camera->WatchRegion1));

                camera->AnalyzeType2 = atoi(*(pszResult + nColumn * i + 11));
                camera->AnalyzeEn2 = atoi(*(pszResult + nColumn * i + 12));

                memset(camera->AlarmTime2, 0, sizeof(camera->AlarmTime2));
                strcpy(camera->AlarmTime2,*(pszResult + nColumn * i  + 13));
                camera->PkgNum2 = atoi(*(pszResult + nColumn * i + 14));

                memset(&camera->WatchRegion2, 0, sizeof(camera->WatchRegion2));
//                strcpy(&camera->WatchRegion2,*(pszResult + nColumn * i  + 15));
                memcpy(&camera->WatchRegion2, (pszResult + nColumn * i  + 15), sizeof(camera->WatchRegion2));


                std::cout << std::endl;
            }
        }
        if (ErrMsg != NULL) sqlite3_free(ErrMsg);
        ErrMsg = NULL;
        sqlite3_free_table(pszResult);
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

int CamDataInfo::setCameraConfig( int cameraid, DBCAMERACONFI *camera )
{
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    char open_db_result = 0;
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));

    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        open_db_result = sqlite3_open("./DataBase/CameraDB.db",&db);

        sprintf_s(SqlBuf,"update CameraInfo set IP = '%s', Port = %d,frameRate = %d, CamUrl = '%s',RtspUrl = '%s' ,CameraFunc = %d,"
                         "AnalyzeNUM = %d,AnalyzeType = %d,CamStatus =%d where CameraID = %d;",
            camera->ip, camera->Port,camera->frameRate,camera->CamUrl,camera->RtspUrl,camera->CameraFunc,camera->AnalyzeNUM,camera->AnalyzeType,camera->CamStatus,cameraid);
//        sprintf_s(SqlBuf,"REPLACE INTO CameraInfo (CameraID, IP, Port, frameRate, CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus) VALUES (%d, %d, %d, %d, %d, %d, %d);",
//            camera->CameraID, camera->ip, camera->Port,camera->frameRate,camera->CameraFunc,camera->AnalyzeNUM,camera->CamStatus,);
//        return m_sqlite3db.execDML( SqlBuf );
         ret = sqlite3_exec(db,SqlBuf,0,0,&ErrMsg);
         return ret;
    }
    catch(cppsqlite3::CppSQLite3Exception& e)
    {
//		std::cerr << "CamDataInfo::getCameraInfo" << e.errorCode() << ":" << e.errorMessage() << std::endl;
//		m_log.Add("CamDataInfo::getCameraInfo: %d : %s ", e.errorCode(), e.errorMessage());
        return -1;
    }   
}

int CamDataInfo::setCameraAlarmInfo( int cameraid, DBCAMERAFUNCPARAM *camera )
{
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));
    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
//        ContentValues cv = new ContentValues();
//        cv.put("CameraID","camera->CameraID");
        camera->CameraID = cameraid;
        sprintf_s(SqlBuf,"update CameraFuncParam set CameraIP = '%s', AnalyzeNUM = %d,"
                         "AnalyzeType = %d,MaxHumanNUM =%d ,ChangeRate = %f,"
                         "AnalyzeType1 = %d, AnalyzeEn1 = %d,AlarmTime1 = '%s',PkgNum1 = %d,WatchRegion1 = '%s',"
                         "AnalyzeType2 = %d, AnalyzeEn2 = %d,AlarmTime2 = '%s',PkgNum2 = %d,WatchRegion2 = '%s',where CameraID = %d",
            camera->ip, camera->AnalyzeNUM,camera->AnalyzeType,camera->MaxHumanNum,camera->ChangeRate,
            camera->AnalyzeType1,camera->AnalyzeEn1,camera->AlarmTime1,camera->PkgNum1,camera->WatchRegion1,
            camera->AnalyzeType2,camera->AnalyzeEn2,camera->AlarmTime2,camera->PkgNum2,camera->WatchRegion2,camera->CameraID);
//        sprintf_s(SqlBuf,"REPLACE INTO CameraInfo (CameraID, IP, Port, frameRate, CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus) VALUES (%d, %d, %d, %d, %d, %d, %d);",
//            camera->CameraID, camera->ip, camera->Port,camera->frameRate,camera->CameraFunc,camera->AnalyzeNUM,camera->CamStatus,);
        return m_sqlite3db.execDML( SqlBuf );

        return 1;
    }
    catch(cppsqlite3::CppSQLite3Exception& e)
    {
//		std::cerr << "CamDataInfo::getCameraInfo" << e.errorCode() << ":" << e.errorMessage() << std::endl;
//		m_log.Add("CamDataInfo::getCameraInfo: %d : %s ", e.errorCode(), e.errorMessage());
        return -1;
    }
}

int CamDataInfo::AddCameraConfig( int cameraid, DBCAMERACONFI *camera )
{
   // /*
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    char open_db_result = 0;
    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        open_db_result = sqlite3_open("./DataBase/CameraDB.db",&db);
//        SQLiteDatabase db = databaseHelper.getWritableDatabase();
//        ContentValues cv = new ContentValues();
//        String[] args = { String.valueof("id") };
//        long rowid = db.insert("CameraInfo", null, cv);

//        sprintf_s(SqlBuf,"REPLACE INTO CameraInfo (CameraID, IP, Port, frameRate, CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus) VALUES (%d, %d, %d, %d, %d, %d, %d);",
//            camera->CameraID, camera->ip, camera->Port,camera->frameRate,camera->CameraFunc,camera->AnalyzeNUM,camera->CamStatus,);
        camera->CameraID=cameraid;
        if(cameraid == NULL)
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

//           if(ret !=SQLITE_OK)
//           {
//               printf("�������ݳɹ�\n");
//           }
         sqlite3_close(db);
         return ret;
//           printf("1111\n");
    }
    catch(cppsqlite3::CppSQLite3Exception& e)
    {
//		std::cerr << "CamDataInfo::getCameraInfo" << e.errorCode() << ":" << e.errorMessage() << std::endl;
//		m_log.Add("CamDataInfo::getCameraInfo: %d : %s ", e.errorCode(), e.errorMessage());
        return -1;
    }
  //  */
}

int CamDataInfo::AddCameraAlarmInfo( int cameraid, DBCAMERAFUNCPARAM *camera )
{
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    char open_db_result = 0;
    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        open_db_result = sqlite3_open("./DataBase/CameraDB.db",&db);
        camera->CameraID=cameraid;
        if(cameraid == NULL)
        {
            sprintf_s(SqlBuf,"insert into CameraFuncParam (IP, AnalyzeNUM, AnalyzeType, MaxHumanNUM, ChangeRate,"
                             "AnalyzeType1, AnalyzeEn1, AlarmTime1, PkgNum1, WatchRegion1,"
                             "AnalyzeType2, AnalyzeEn2, AlarmTime2, PkgNum2, WatchRegion2) VALUES ('%s', %d, %d, %d, %f, %d, %d,'%s', %d, '%s', %d, %d,'%s',%d,'%s');",
                camera->ip, camera->AnalyzeNUM,camera->AnalyzeType,camera->MaxHumanNum,camera->ChangeRate,
                camera->AnalyzeType1,camera->AnalyzeEn1,camera->AlarmTime1,camera->PkgNum1,camera->WatchRegion1,
                camera->AnalyzeType2,camera->AnalyzeEn2,camera->AlarmTime2,camera->PkgNum2,camera->WatchRegion2);
        }
        else
        {
            sprintf_s(SqlBuf,"insert into CameraFuncParam (CameraID, IP, AnalyzeNUM, AnalyzeType, MaxHumanNUM, ChangeRate,"
                             "AnalyzeType1, AnalyzeEn1, AlarmTime1, PkgNum1, WatchRegion1,"
                             "AnalyzeType2, AnalyzeEn2, AlarmTime2, PkgNum2, WatchRegion2) VALUES (%d, '%s', %d, %d, %d, %f, %d, %d,'%s', %d, '%s', %d, %d,'%s',%d,'%s');",
                camera->CameraID, camera->ip, camera->AnalyzeNUM,camera->AnalyzeType,camera->MaxHumanNum,camera->ChangeRate,
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
//		std::cerr << "CamDataInfo::getCameraInfo" << e.errorCode() << ":" << e.errorMessage() << std::endl;
//		m_log.Add("CamDataInfo::getCameraInfo: %d : %s ", e.errorCode(), e.errorMessage());
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
    char open_db_result = 0;
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        open_db_result = sqlite3_open("./DataBase/CameraDB.db",&db);
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
    char open_db_result = 0;
    memset(SqlBuf,0,sizeof(SqlBuf));
    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        open_db_result = sqlite3_open("./DataBase/CameraDB.db",&db);
        sprintf_s(SqlBuf,"Delete from CameraFuncParam where CameraID = %d;", iCamera);
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


