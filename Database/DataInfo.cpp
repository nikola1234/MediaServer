// DataInfo.cpp: implementation of the CamDataInfo class.
//////////////////////////////////////////////////////////////////////

#include "boost/thread.hpp"
#include "boost/filesystem.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
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

    sqlite3* db = NULL;                             /**<sqlite3 ??±ú*/

    char open_db_result = 0;                        /**<?ò?????????á??*/
    char db_tpath[DB_PATH_LEN] = {0};               /**<???????·??*/
    int db_exist_result = 0;
    int mkdir_return = 0 ;
    sprintf(db_tpath,"%s%s",DB_PATH,db_name);       /**<???????????·??*/

    db_exist_result = exist(db_name);

    if(DB_NOT_EXIST != db_exist_result)
    {
        return DB_OK;
    }
    printf("create db-------------------------- :%s%s\n",DB_PATH,db_name);       /**<???????????·??*/
    printf("make dir:%s",DB_PATH);
    mkdir_return = mkdir(DB_PATH, 0755);

    open_db_result = sqlite3_open(db_tpath,&db);

    if(SQLITE_OK == open_db_result)
    {
        sqlite3_close(db);
        return DB_OK;
    }
    else                                            /**<?ò????????????????????????????????????*/
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
    const char *SQL1="create table CameraInfo (CameraID INTEGETER PRIMARY KEY,IP char(16),Port INTEGETER,"
                     "frameRate INTEGETER,CamUrl char(128),RtspUrl char(128),CameraFunc INTEGETER,"
                     "AnalyzeNUM INTEGETER, AnalyzeType INTEGETER, CamStatus INTEGETER);";
//执行建表
    ret = sqlite3_exec(db,SQL1,0,0,&ErrMsg);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr,"SQL1 Error:%s\n",ErrMsg);
        sqlite3_free(ErrMsg);
    }
    const char *SQL2="create table CameraFuncParam (CameraID INTEGETER PRIMARY KEY,IP char(16),AnalyzeNUM INTEGETER,"
                     "AnalyzeType INTEGETER,MaxHumanNum INTEGETER,ChangeRate REAL,AnalyzeType1 INTEGETER,"
                     "AnalyzeEn1 INTEGETER,AlarmTime1 char(84), PkgNum1 INTEGETER, WatchRegion1 varchar(256),"
                     " AnalyzeType2 INTEGETER,"
                     "AnalyzeEn2 INTEGETER,AlarmTime2 char(84), PkgNum2 INTEGETER, WatchRegion2 varchar(256));";
//执行建表
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
        //执行建表
        ret = sqlite3_exec(db,SQL1,0,0,&ErrMsg);
        printf("MaxCameraID = %d\n",ret);
        if(ret == 0)
        {
            fprintf(stderr,"SQL Error:%s\n",ErrMsg);
            printf("MaxCameraID = %d\n",ret);
            sqlite3_close(db);
            return 0;
            //sqlite3_free(ErrMsg);
        }
//        DataTable dt = SqlLiteHelper.GetDataTabl(strcreatsqlvalue);
//        sprintf_s(SqlBuf,"select MAX(CameraID) as CameraID from CameraInfo");
//        cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);
//        if(!q.eof())
//        {
//            if (!q.fieldIsNull("CameraID"))
//            {
//                return atoi(q.fieldValue("CameraID"));
//            }
//            return -1;
//        }
//        else
//        {
//            return -1;
//        }
    }
    catch (cppsqlite3::CppSQLite3Exception& e)
    {
//		m_log.Add("CDataInfo::GetMaxCameraID: %d : %s ", e.errorCode(), e.errorMessage());
        return -1;
    }
    return -1;
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
    //每条记录回调一次该函数，有多少条就回调多少次
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
    void *data = NULL;
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        open_db_result = sqlite3_open("./CameraDB.db",&db);

        sprintf_s(SqlBuf,"select CameraID, IP, Port, frameRate, CamUrl,"
            "RtspUrl, CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus "
            "from CameraInfo where CameraID = %d ", cameraid);
//         sprintf_s(SqlBuf,"select * from CameraInfo;");
//		camera->CameraID = cameraid;
//        //查询数据表内容
//        printf("query\n");
//        sqlite3_exec(db,SqlBuf,select_callback,data,&ErrMsg);
        // 获取上面sql语句指定的记录集
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
            // 此记录集成功获取后,最前面的nColume个元素是列名
            // 因此,在做循环读取记录是，行数是比返回的行数多一行,存放列名
            printf("%d record!\n", nRow);
            printf("%d line!\n", nColumn);
			if((nRow == 0)&&(nColumn == 0))
			{
				   sqlite3_close(db);
				   return 0;
			}
				
            int index = nColumn;
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
//        camera = (DBCAMERACONFI *)data;

        //memcpy(camera,data,sizeof(data));

//        cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);
//		ret = sqlite3_exec(db,SqlBuf,0,0,&ErrMsg);
        /*
        if(!q.eof())//end=1,else=0
        {
            if (!q.fieldIsNull("CameraID"))
            {
                camera->CameraID = atoi(q.fieldValue("CameraID"));
            }

            memset(camera->ip, 0, 256);
            if (!q.fieldIsNull("IP"))
            {
                strcpy(camera->ip, q.fieldValue("IP"));
            }


           if (!q.fieldIsNull("Port"))
           {
                camera->Port = atoi(q.fieldValue("Port"));
                printf("ok\n");
           }
//            char URL[256]; memset(URL, 0, 256);
//            sprintf( URL, "rtsp://%s:%d/%d", ip, port, cameraid );
//            std::string strurl = URL;
//            CameraURLArray.push_back(strurl);

//            q.nextRow();
            if (!q.fieldIsNull("frameRate"))
            {
                camera->frameRate = atoi(q.fieldValue("frameRate"));
            }

            if (!q.fieldIsNull("CameraFunc"))
            {
                camera->CameraFunc = atoi(q.fieldValue("CameraFunc"));
            }

            if (!q.fieldIsNull("AnalyzeNUM"))
            {
                camera->AnalyzeNUM = atoi(q.fieldValue("AnalyzeNUM"));
            }

            if (!q.fieldIsNull("AnalyzeType"))
            {
                camera->AnalyzeType = atoi(q.fieldValue("AnalyzeType"));
            }

            if (!q.fieldIsNull("CamStatus"))
            {
                camera->CamStatus = atoi(q.fieldValue("CamStatus"));
            }
            return 1;
        }
        else
        {
            printf("err\n");
//			m_log.Add("CamDataInfo::getCameraInfo: no camera[Cameraid:%d].", cameraid);
            return -1;
        }
        */
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
    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

        char SqlBuf[SQL_STRING_MAX];
        memset(SqlBuf,0,sizeof(SqlBuf));

//		sprintf_s(SqlBuf,"select CameraID, CameraName, Input, a.Remark as Remark,"
//			"a.DevID as DevID, DevType, c.UnitName as UnitName, b.isMatrix as isMatrix "
//			"from vnmp_CameraInfo a, vnmp_DevType b, vnmp_UnitInfo c "
//			"where CameraID='%d' and a.DevID = b.DevID and a.UnitID = c.UnitID", cameraid);

        sprintf_s(SqlBuf,"select CameraID, CameraIP, AnalyzeNUM, AnalyzeType, MaxHumanNUM,"
            "ChangeRate, AnalyzeType1, AnalyzeEn1, AlarmTime1, PkgNum1, WatchRegion1, "
            "AnalyzeType2, AnalyzeEn2, AlarmTime2, PkgNum2, WatchRegion2, "
            "from CameraFuncParam"
            "where CameraID='%d' ", cameraid);

        camera->CameraID = cameraid;
        cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

        if(!q.eof())//end=1,else=0
        {
            if (!q.fieldIsNull("CameraID"))
            {
                camera->CameraID = atoi(q.fieldValue("CameraID"));
            }

            memset(camera->ip, 0, 256);
            if (!q.fieldIsNull("CameraIP"))
                strcpy(camera->ip, q.fieldValue("CameraIP"));

           if (!q.fieldIsNull("AnalyzeNUM"))
                camera->AnalyzeNUM = atoi(q.fieldValue("AnalyzeNUM"));

           if (!q.fieldIsNull("AnalyzeType"))
                camera->AnalyzeType = atoi(q.fieldValue("AnalyzeType"));

           if (!q.fieldIsNull("MaxHumanNum"))
                camera->MaxHumanNum = atoi(q.fieldValue("MaxHumanNum"));

           if (!q.fieldIsNull("ChangeRate"))
                camera->ChangeRate = atoi(q.fieldValue("ChangeRate"));

           if (!q.fieldIsNull("AnalyzeType1"))
                camera->AnalyzeType1 = atoi(q.fieldValue("AnalyzeType1"));

           if (!q.fieldIsNull("AnalyzeEn1"))
                camera->AnalyzeEn1 = atoi(q.fieldValue("AnalyzeEn1"));

//           char time_tmp[84];
//           memset(time_tmp, 0, 84);
            memset(camera->AlarmTime1, 0, 84);
//           if (!q.fieldIsNull("AlarmTime1"))
//               memcpy(camera->AlarmTime1,atoi(q.fieldValue("AlarmTime1")),84);

           if (!q.fieldIsNull("PkgNum1"))
           {
               camera->PkgNum1 = atoi(q.fieldValue("PkgNum1"));
           }

//             memset(camera->WatchRegion1, 0, sizeof(camera->WatchRegion1));
//           if (!q.fieldIsNull("WatchRegion1"))
//               memcpy(camera->WatchRegion1,atoi(q.fieldValue("WatchRegion1")),sizeof(camera->WatchRegion1));

           if (!q.fieldIsNull("AnalyzeType2"))
                camera->AnalyzeType2 = atoi(q.fieldValue("AnalyzeType2"));

           if (!q.fieldIsNull("AnalyzeEn2"))
                camera->AnalyzeEn2 = atoi(q.fieldValue("AnalyzeEn2"));

//           char time_tmp[84];
//           memset(time_tmp, 0, 84);
           memset(camera->AlarmTime2, 0, 84);
//           if (!q.fieldIsNull("AlarmTime2"))
//               memcpy(camera->AlarmTime2,atoi(q.fieldValue("AlarmTime2")),84);

           if (!q.fieldIsNull("PkgNum2"))
           {
               camera->PkgNum2 = atoi(q.fieldValue("PkgNum2"));
           }

            return 1;
        }
        else
        {
//			m_log.Add("CamDataInfo::getCameraInfo: no camera[Cameraid:%d].", cameraid);
            return -1;
        }
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
    ///*
    char *ErrMsg=0;
    int  ret = 0;
    sqlite3* db = NULL;
    char open_db_result = 0;
    char **pszResult = 0;
    int nRow = 0;
    int nColumn = 0;
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));
    void *data = NULL;
    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        open_db_result = sqlite3_open("./DataBase/CameraDB.db",&db);

        sprintf_s(SqlBuf,"update CameraInfo set IP = %s, Port = %d,frameRate = %d, CamUrl = %s,RtspUrl = %s ,CameraFunc = %d,"
                         "AnalyzeNUM = %d,AnalyzeType = %d,CamStatus =%d where CameraID = %d;",
            camera->ip, camera->Port,camera->frameRate,camera->CamUrl,camera->RtspUrl,camera->CameraFunc,camera->AnalyzeNUM,camera->AnalyzeType,camera->CamStatus,cameraid);
//        sprintf_s(SqlBuf,"REPLACE INTO CameraInfo (CameraID, IP, Port, frameRate, CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus) VALUES (%d, %d, %d, %d, %d, %d, %d);",
//            camera->CameraID, camera->ip, camera->Port,camera->frameRate,camera->CameraFunc,camera->AnalyzeNUM,camera->CamStatus,);
//        return m_sqlite3db.execDML( SqlBuf );
         ret = sqlite3_exec(db,SqlBuf,0,0,&ErrMsg);
         return ret;
        //

    }
    catch(cppsqlite3::CppSQLite3Exception& e)
    {
//		std::cerr << "CamDataInfo::getCameraInfo" << e.errorCode() << ":" << e.errorMessage() << std::endl;
//		m_log.Add("CamDataInfo::getCameraInfo: %d : %s ", e.errorCode(), e.errorMessage());
        return -1;
    }
    //*/
}

int CamDataInfo::setCameraAlarmInfo( int cameraid, DBCAMERAFUNCPARAM *camera )
{
    /*
    char SqlBuf[SQL_STRING_MAX];
    memset(SqlBuf,0,sizeof(SqlBuf));
    try
    {
        boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
//        ContentValues cv = new ContentValues();

//        cv.put("CameraID","camera->CameraID");
//        cv.put("CameraIP","camera->ip");
//        cv.put("AnalyzeNUM","camera->AnalyzeNUM");
//        cv.put("AnalyzeType","camera->AnalyzeType");
//        cv.put("MaxHumanNUM","camera->MaxHumanNUM");
//        cv.put("ChangeRate","camera->ChangeRate");
//        cv.put("AnalyzeType1","camera->AnalyzeType1");
//        cv.put("AnalyzeEn1","camera->AnalyzeEn1");
//        cv.put("AlarmTime1","camera->AlarmTime1");
//        cv.put("PkgNum1","camera->PkgNum1");
//        cv.put("WatchRegion1","camera->WatchRegion1");
//        cv.put("AnalyzeType2","camera->AnalyzeType2");
//        cv.put("AnalyzeEn2","camera->AnalyzeEn2");
//        cv.put("AlarmTime2","camera->AlarmTime2");
//        cv.put("PkgNum2","camera->PkgNum2");
//        cv.put("WatchRegion2","camera->WatchRegion2");
//        update("CameraFuncParam",cv,"CameraID=?",cameraid);

//        sprintf_s(SqlBuf,"update CameraFuncParam set CameraIP = %d, AnalyzeNUM = %d,"
//                         "AnalyzeType = %d,MaxHumanNUM =%d ,ChangeRate = %d,"
//                         "AnalyzeType1 = %d, AnalyzeEn1 = %d,AlarmTime1 = '%sZ,PkgNum1 = %d,WatchRegion1 = %d,"
//                         "AnalyzeType2 = %d, AnalyzeEn2 = %d,AlarmTime2 = '%sZ,PkgNum2 = %d,WatchRegion2 = %d,where CameraID = %d",
//            camera->ip, camera->AnalyzeNUM,camera->AnalyzeType,camera->MaxHumanNum,camera->ChangeRate,
//            camera->AnalyzeType1,camera->AnalyzeEn1,camera->AlarmTime1,camera->PkgNum1,camera->WatchRegion1,
//            camera->AnalyzeType2,camera->AnalyzeEn2,camera->AlarmTime2,camera->PkgNum2,camera->WatchRegion2,camera->CameraID);
        sprintf_s(SqlBuf,"update CameraFuncParam set CameraIP = %d, AnalyzeNUM = %d,"
                         "AnalyzeType = %d,MaxHumanNUM =%d ,ChangeRate = %d,"
                         "AnalyzeType1 = %d, AnalyzeEn1 = %d,AlarmTime1 = '%sZ,PkgNum1 = %d,"
                         "AnalyzeType2 = %d, AnalyzeEn2 = %d,AlarmTime2 = '%sZ,PkgNum2 = %d,where CameraID = %d",
            camera->ip, camera->AnalyzeNUM,camera->AnalyzeType,camera->MaxHumanNum,camera->ChangeRate,
            camera->AnalyzeType1,camera->AnalyzeEn1,camera->AlarmTime1,camera->PkgNum1,
            camera->AnalyzeType2,camera->AnalyzeEn2,camera->AlarmTime2,camera->PkgNum2,camera->CameraID);
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
    */
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
//        cv.put("CameraID","camera->CameraID");
//        cv.put("IP","camera->ip");
//        cv.put("Port","camera->Port");
//        cv.put("frameRate","camera->frameRate");
//        cv.put("CameraFunc","camera->CameraFunc");
//        cv.put("AnalyzeNUM","camera->AnalyzeNUM");
//        cv.put("AnalyzeType","camera->AnalyzeType");
//        cv.put("CamStatus","camera->CamStatus");
//       update("CameraInfo",cv,"CameraID=?",cameraid);
//        long rowid = db.insert("CameraInfo", null, cv);

//        return 1;
//        sprintf_s(SqlBuf,"REPLACE INTO vnmp_CameraState (CameraID, vnmpOnline, CameraID, LastUpdateTime) VALUES (%d, %d, %d, '%sZ');",
//            iCameraID, iOnline, iStatus, boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time()).c_str());
//        sprintf_s(SqlBuf,"REPLACE INTO CameraInfo (CameraID, IP, Port, frameRate, CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus) VALUES (%d, %d, %d, %d, %d, %d, %d);",
//            camera->CameraID, camera->ip, camera->Port,camera->frameRate,camera->CameraFunc,camera->AnalyzeNUM,camera->CamStatus,);
        camera->CameraID=cameraid;
        sprintf_s(SqlBuf,"insert into CameraInfo (CameraID, IP, Port, frameRate, CamUrl, RtspUrl ,CameraFunc, AnalyzeNUM, AnalyzeType, CamStatus) values (%d, %s, %d, %d,%s,%s, %d, %d, %d, %d)",
            camera->CameraID, camera->ip, camera->Port,camera->frameRate,camera->CamUrl,camera->RtspUrl,camera->CameraFunc,camera->AnalyzeNUM,camera->CamStatus);
//        return m_sqlite3db.execDML( SqlBuf );
  ret = sqlite3_exec(db,SqlBuf,0,0,&ErrMsg);
        //向数据库插入数据
//        camera->CameraID=6;
//           const char *SQL2="insert into CameraInfo values(atoi(camera->CameraID),'123456789012345',3,4,'123456789012345','123456789012345',7,8,9,10);";
//           ret = sqlite3_exec(db,SQL2,0,0,&ErrMsg);
//           if(ret !=SQLITE_OK)
//           {
//               printf("插入数据成功\n");
//           }
  sqlite3_close(db);
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
  //  /*
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

//        cv.put("CameraID","camera->CameraID");
//        cv.put("CameraIP","camera->ip");
//        cv.put("AnalyzeNUM","camera->AnalyzeNUM");
//        cv.put("AnalyzeType","camera->AnalyzeType");
//        cv.put("MaxHumanNUM","camera->MaxHumanNUM");
//        cv.put("ChangeRate","camera->ChangeRate");
//        cv.put("AnalyzeType1","camera->AnalyzeType1");
//        cv.put("AnalyzeEn1","camera->AnalyzeEn1");
//        cv.put("AlarmTime1","camera->AlarmTime1");
//        cv.put("PkgNum1","camera->PkgNum1");
//        cv.put("WatchRegion1","camera->WatchRegion1");
//        cv.put("AnalyzeType2","camera->AnalyzeType2");
//        cv.put("AnalyzeEn2","camera->AnalyzeEn2");
//        cv.put("AlarmTime2","camera->AlarmTime2");
//        cv.put("PkgNum2","camera->PkgNum2");
//        cv.put("WatchRegion2","camera->WatchRegion2");

//        update("CameraFuncParam",cv,"CameraID=?",cameraid);
//        long rowid = db.insert("CameraFuncParam", null, cv);
        camera->CameraID=cameraid;
//        sprintf_s(SqlBuf,"insert into CameraFuncParam (CameraID, IP, AnalyzeNUM, AnalyzeType, MaxHumanNUM, ChangeRate,"
//                         "AnalyzeType1, AnalyzeEn1, AlarmTime1, PkgNum1, WatchRegion1,"
//                         "AnalyzeType2, AnalyzeEn2, AlarmTime2, PkgNum2, WatchRegion2) VALUES (%d, %s, %d, %d, %d, %f, %d, %d,%s, %d, %s, %d, %d,%s,%d,%s);",
//            camera->CameraID, camera->ip, camera->AnalyzeNUM,camera->AnalyzeType,camera->MaxHumanNum,camera->ChangeRate,
//            camera->AnalyzeType1,camera->AnalyzeEn1,camera->AlarmTime1,camera->PkgNum1,camera->WatchRegion1,
//            camera->AnalyzeType2,camera->AnalyzeEn2,camera->AlarmTime2,camera->PkgNum2,camera->WatchRegion2);
        sprintf_s(SqlBuf,"insert into CameraFuncParam (CameraID, IP, AnalyzeNUM, AnalyzeType, MaxHumanNUM, ChangeRate,"
                         "AnalyzeType1, AnalyzeEn1, AlarmTime1, PkgNum1,"
                         "AnalyzeType2, AnalyzeEn2, AlarmTime2, PkgNum2) VALUES (%d, %s, %d, %d, %d, %f, %d, %d,%s, %d, %d, %d,%s,%d);",
            camera->CameraID, camera->ip, camera->AnalyzeNUM,camera->AnalyzeType,camera->MaxHumanNum,camera->ChangeRate,
            camera->AnalyzeType1,camera->AnalyzeEn1,camera->AlarmTime1,camera->PkgNum1,
            camera->AnalyzeType2,camera->AnalyzeEn2,camera->AlarmTime2,camera->PkgNum2);
//        sprintf_s(SqlBuf,"insert into CameraFuncParam (CameraID, IP, AnalyzeNUM, AnalyzeType, MaxHumanNUM, ChangeRate,"
//                         "AnalyzeType1, AnalyzeEn1, AlarmTime1, PkgNum1,"
//                         "AnalyzeType2, AnalyzeEn2, AlarmTime2, PkgNum2) VALUES (%d, %s, %d, %d, %d, %f, %d, %d,%s, %d, %d, %d,%s,%d);",
//            camera->CameraID, camera->ip, camera->AnalyzeNUM,camera->AnalyzeType,camera->MaxHumanNum,camera->ChangeRate,
//            camera->AnalyzeType1,camera->AnalyzeEn1,camera->AlarmTime1,camera->PkgNum1,
//            camera->AnalyzeType2,camera->AnalyzeEn2,camera->AlarmTime2,camera->PkgNum2);
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
  //  */
}



int CamDataInfo::DelCameraInfo(int iCamera)
{
//    SQLiteDatabase db = databaseHelper.getWritableDatabase();
//    db.delete("person", "personid<?", new String[]{"2"});
//    db.close();
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
        sprintf_s(SqlBuf,"Delete from CameraInfo where CameraID = %d ", iCamera);
		return m_sqlite3db.execDML( SqlBuf );
	}
    catch (cppsqlite3::CppSQLite3Exception& e)
	{
//        m_log.Add("CamDataInfo::DelCameraInfo: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return -1;
}

//void CamDataInfo::UpdateAllCameraList(int iDevID, unsigned long lCameraNum, STCAMERAINFO* CameraInfoArray)
//{
//}
