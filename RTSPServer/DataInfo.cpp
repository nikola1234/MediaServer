// DataInfo.cpp: implementation of the CDataInfo class.
//////////////////////////////////////////////////////////////////////

#include "boost/thread.hpp"
#include "boost/filesystem.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
namespace fs = boost::filesystem;

#include "DataInfo.h"

#ifdef WIN32
#include "codeconverter.h"
#else
#include "codeconverter_unix.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDataInfo::CDataInfo()
{
	m_log.InitLog("./log/RecvDB ");
}

CDataInfo::~CDataInfo()
{
//	StopRecvCommand((int)this);
}

int CDataInfo::Open(const char* szfilepath)
{
	try
	{
		{
			boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
			m_szFileDB = szfilepath;
			m_sqlite3db.open(szfilepath);
		}

		//int port = 0;
		//char configValue[256]; memset(configValue, 0, 256);
		//int i = GetConfig("PORT_TO_DB", configValue);
		//if( i > 0 ) port = atoi( configValue );
		//if( port > 0 && port < 65535 )
		//	StartRecvCommand(port, (int)this, funHandleRecvCommand);
	}
	catch (cppsqlite3::CppSQLite3Exception& e)
	{
		//std::cerr << "CDataInfo::Open: " << e.errorCode() << ":" << e.errorMessage() << std::endl;
		m_log.Add("CDataInfo::Open: %d : %s ", e.errorCode(), e.errorMessage());
	}
	return 0;
}

int CDataInfo::getCameraInfo( int cameraid, DBCAMERAINFO *camera )
{
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		char SqlBuf[SQL_STRING_MAX];
		memset(SqlBuf,0,sizeof(SqlBuf));

		sprintf_s(SqlBuf,"select CameraID, CameraName, Input, a.Remark as Remark,"
			"a.DevID as DevID, DevType, c.UnitName as UnitName, b.isMatrix as isMatrix "
			"from vnmp_CameraInfo a, vnmp_DevType b, vnmp_UnitInfo c "
			"where CameraID='%d' and a.DevID = b.DevID and a.UnitID = c.UnitID", cameraid);	

		camera->CameraID = cameraid;
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);
		
        if(!q.eof())
		{
			if (!q.fieldIsNull("CameraID"))
			{
				camera->CameraID = atoi(q.fieldValue("CameraID"));
			}
			if(!q.fieldIsNull("CameraName"))						
			{
				char *gb2312;
				size_t size = CodeConverter::Utf8ToGB2312((char *)q.fieldValue("CameraName"),
					(size_t)strlen(q.fieldValue("CameraName")) + 1, &gb2312);
				strcpy(camera->CameraName,gb2312);
				delete gb2312;
			}
			if(!q.fieldIsNull("UnitName"))						
			{				
				char *gb2312;
				size_t size = CodeConverter::Utf8ToGB2312((char *)q.fieldValue("UnitName"),
					(size_t)strlen(q.fieldValue("UnitName")) + 1, &gb2312);
				strcpy(camera->UnitName,gb2312);
				delete gb2312;
			}
			if (!q.fieldIsNull("Input"))
			{
				strcpy(camera->Input,q.fieldValue("Input"));
			}
			if(!q.fieldIsNull("Remark"))						
			{			
				char *gb2312;
				size_t size = CodeConverter::Utf8ToGB2312((char *)q.fieldValue("Remark"),
					(size_t)strlen(q.fieldValue("Remark")) + 1, &gb2312);
				strcpy(camera->Remark,gb2312);
				delete gb2312;
			}
			if (!q.fieldIsNull("DevID"))
			{
				camera->DevID = atoi(q.fieldValue("DevID"));
			}
			if (!q.fieldIsNull("DevType"))
			{
				camera->DevType = atoi(q.fieldValue("DevType"));
			}
			//if (!q.fieldIsNull("CameraType"))
			//{
			//	camera->CameraType = atoi(q.fieldValue("CameraType"));
			//}
			camera->CameraType = 0;
			if (!q.fieldIsNull("isMatrix"))
			{
				camera->isMatrix = atoi(q.fieldValue("isMatrix"));
			}
			return 1;
		}
		else
		{
			m_log.Add("CDataInfo::getCameraInfo: no camera[Cameraid:%d].", cameraid);
			return -1;
		}
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		//std::cerr << "CDataInfo::getCameraInfo" << e.errorCode() << ":" << e.errorMessage() << std::endl;
		m_log.Add("CDataInfo::getCameraInfo: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
}

int CDataInfo::getCoderInfo( int CoderID, DBCODERINFO *coder )
{
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		char SqlBuf[SQL_STRING_MAX];
		memset(SqlBuf,0,sizeof(SqlBuf));
		sprintf_s(SqlBuf,"select CoderID, IPAddr, Port, Input, Output, LoginUser, LoginPwd, MatrixID from vnmp_CoderInfo where CoderID = %d ", CoderID);
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);
		if(!q.eof())
		{
			if (!q.fieldIsNull("CoderID"))
			{
				coder->CoderID = atoi(q.fieldValue("CoderID"));
			}
			if(!q.fieldIsNull("IPAddr"))						
			{					   
				strcpy_s(coder->IPAddr, q.fieldValue("IPAddr"));
			}
			if (!q.fieldIsNull("Port"))
			{
				coder->Port = atoi(q.fieldValue("Port"));
			}
			if(!q.fieldIsNull("LoginUser"))						
			{					   
				strcpy_s(coder->User, q.fieldValue("LoginUser"));
			}
			if(!q.fieldIsNull("LoginPwd"))						
			{					   
				strcpy_s(coder->Pwd, q.fieldValue("LoginPwd"));
			}
			if (!q.fieldIsNull("Input"))
			{
				coder->Input = atoi(q.fieldValue("Input"));
			}
			if (!q.fieldIsNull("Output"))
			{
				coder->OutPut = atoi(q.fieldValue("Output"));
			}
			if (!q.fieldIsNull("MatrixID"))
			{
				coder->MatrixID = atoi(q.fieldValue("MatrixID"));
			}
			return 1;
		}
		else
		{
			return -1;
		}
	}
	catch (cppsqlite3::CppSQLite3Exception& e)
	{
		//std::cerr  << "CDataInfo::getCoderInfo" << e.errorCode() << ":" << e.errorMessage() << std::endl;
		m_log.Add("CDataInfo::getCoderInfo: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}	
	return 0;
}

int CDataInfo::getDevInfo( int DevID, DBDEVINFO *dev )
{
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		sprintf_s(SqlBuf,"select DevID as DevID, IPAddr, Port, LoginUser, LoginPwd, DevType from vnmp_DevInfo where DevID = %d ", DevID);	

		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);
		if(!q.eof())
		{
			if (!q.fieldIsNull("DevID"))
			{
				dev->DevID = atoi(q.fieldValue("DevID"));
			}
			if(!q.fieldIsNull("IPAddr"))						
			{					   
				strcpy(dev->IPAddr, q.fieldValue("IPAddr"));
			}
			if (!q.fieldIsNull("Port"))
			{
				dev->Port = atoi(q.fieldValue("Port"));
			}
			if(!q.fieldIsNull("LoginUser"))						
			{					   
				strcpy(dev->User, q.fieldValue("LoginUser"));
			}
			if(!q.fieldIsNull("LoginPwd"))						
			{					   
				strcpy(dev->Pwd, q.fieldValue("LoginPwd"));
			}
			if (!q.fieldIsNull("DevType"))
			{
				dev->DevType = atoi(q.fieldValue("DevType"));
			}
			return 1;
		}
		else
		{
			m_log.Add("CDataInfo::getDevInfo: no dev[DevID:%d].", DevID);
			return 0;
		}
	}
	catch (cppsqlite3::CppSQLite3Exception& e)
	{
		//std::cerr  << "CDataInfo::getDevInfo" << e.errorCode() << ":" << e.errorMessage() << std::endl;
		m_log.Add("CDataInfo::getDevInfo: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return 0;
}

int CDataInfo::getMatrixInfo(int MatrixID, DBMATRIXINFO *MatrixInfo)
{
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		sprintf_s(SqlBuf,"select a.*, b.*,a.remark as softcontrip from vnmp_MatrixInfo a, vnmp_CoderInfo b where a.MatrixID = %d and a.ControlID = b.CoderID ", MatrixID);	

		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);
		if(!q.eof())
		{
			if (!q.fieldIsNull("MatrixID"))
			{
				MatrixInfo->MatrixID = atoi(q.fieldValue("MatrixID"));
			}
			if (!q.fieldIsNull("MatrixType"))
			{
				MatrixInfo->MatrixType = atoi(q.fieldValue("MatrixType"));
			}
			if (!q.fieldIsNull("Baud"))
			{
				MatrixInfo->Baud = atoi(q.fieldValue("Baud"));
			}
			if (!q.fieldIsNull("DataBit"))
			{
				MatrixInfo->DataBit = atoi(q.fieldValue("DataBit"));
			}
			if (!q.fieldIsNull("StartBit"))
			{
				MatrixInfo->StartBit = atoi(q.fieldValue("StartBit"));
			}
			if (!q.fieldIsNull("StopBit"))
			{
				MatrixInfo->StopBit = atoi(q.fieldValue("StopBit"));
			}
			if (!q.fieldIsNull("Parity"))
			{
				MatrixInfo->Parity = atoi(q.fieldValue("Parity"));
			}
			if (!q.fieldIsNull("FlowControl"))
			{
				MatrixInfo->FlowControl = atoi(q.fieldValue("FlowControl"));
			}
			if (!q.fieldIsNull("CoderID"))
			{
				MatrixInfo->ControlCoder.CoderID = atoi(q.fieldValue("CoderID"));
			}
			if(!q.fieldIsNull("IPAddr"))						
			{					   
				strcpy_s(MatrixInfo->ControlCoder.IPAddr, q.fieldValue("IPAddr"));
			}
			if (!q.fieldIsNull("Port"))
			{
				MatrixInfo->ControlCoder.Port = atoi(q.fieldValue("Port"));
			}
			if(!q.fieldIsNull("LoginUser"))						
			{					   
				strcpy_s(MatrixInfo->ControlCoder.User, q.fieldValue("LoginUser"));
			}
			if(!q.fieldIsNull("LoginPwd"))						
			{					   
				strcpy_s(MatrixInfo->ControlCoder.Pwd, q.fieldValue("LoginPwd"));
			}
			if (!q.fieldIsNull("softcontrip"))
			{
				strcpy_s(MatrixInfo->SoftConIPAddr,q.fieldValue("softcontrip"));
			}
			return 1;
		}
		else
		{
			m_log.Add("CDataInfo::getMatrixInfo: no MatrixID[MatrixID:%d].", MatrixID);
			return -1;
		}
	}
	catch (cppsqlite3::CppSQLite3Exception& e)
	{
		//std::cerr  << "CDataInfo::getMatrixInfo" << e.errorCode() << ":" << e.errorMessage() << std::endl;
		m_log.Add("CDataInfo::getMatrixInfo: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return 0;
}

int CDataInfo::getMatrixInfoByControlID(int UnitID, int& ControlID, DBMATRIXINFO *MatrixInfo)
{
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		memset(MatrixInfo, 0, sizeof(DBMATRIXINFO));

		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf,"select * from vnmp_MatrixInfo where ControlID = %d and MatrixID/100000 = %d", ControlID, UnitID);	
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		if( q.eof() ) {
			sprintf_s(SqlBuf,"select * from vnmp_MatrixInfo where ControlID = 0 and MatrixID/100000 = %d", UnitID);	
			q = m_sqlite3db.execQuery(SqlBuf);
			ControlID = 0;
		}

		if(!q.eof())
		{
			if (!q.fieldIsNull("MatrixID"))
			{
				MatrixInfo->MatrixID = atoi(q.fieldValue("MatrixID"));
			}
			if (!q.fieldIsNull("MatrixType"))
			{
				MatrixInfo->MatrixType = atoi(q.fieldValue("MatrixType"));
			}
			if (!q.fieldIsNull("Remark"))
			{
				strcpy_s(MatrixInfo->SoftConIPAddr,q.fieldValue("Remark"));
			}
			return 1;
		}
		else
			return -1;
	}
	catch (cppsqlite3::CppSQLite3Exception& e)
	{
		//std::cerr  << "CDataInfo::getMatrixInfoByControlID" << e.errorCode() << ":" << e.errorMessage() << std::endl;
		m_log.Add("CDataInfo::getMatrixInfoByControlID: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return 0;
}

void CDataInfo::getCoderidVector(std::vector<int> &coderid_list, int matrixid)
{
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf, 0, sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		sprintf_s(SqlBuf, "SELECT CoderID from vnmp_CoderInfo where MatrixID = %d ",matrixid);
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		coderid_list.clear();
		while (!q.eof())
		{  
			if(q.fieldIsNull("CoderID")) continue;
			int coderid = atoi(q.fieldValue("CoderID"));
			coderid_list.push_back(coderid);
			q.nextRow();
		}
		return;
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		//std::cerr  << "CDataInfo::getCoderidVector" << e.errorCode() << ":" << e.errorMessage() << std::endl;
		m_log.Add("CDataInfo::getCoderidVector: %d : %s ", e.errorCode(), e.errorMessage());
		return;
	}
}

void CDataInfo::getVirtualCoderidVector(int UnitID, std::vector<int> &coderid_vector, int& MatrixID)
{
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf,"SELECT CoderID from vnmp_CoderInfo where MatrixID = %d and CoderID/100000 = %d order by CoderID", MatrixID, UnitID);	
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		if( q.eof() ) {
			sprintf_s(SqlBuf,"SELECT CoderID from vnmp_CoderInfo where MatrixID = 0 and CoderID/100000 = %d order by CoderID", UnitID);
			q = m_sqlite3db.execQuery(SqlBuf);
			MatrixID = 0;
		}

		coderid_vector.clear();
		while ( !q.eof() ) {  
			if(q.fieldIsNull("CoderID")) continue;
			int coderid = atoi(q.fieldValue("CoderID"));
			coderid_vector.push_back(coderid);
			q.nextRow();
		}
	} catch (cppsqlite3::CppSQLite3Exception& e) {
		m_log.Add("CDataInfo::getVirtualCoderidVector: %d : %s ", e.errorCode(), e.errorMessage());
	}
}

int CDataInfo::GetCameraIDByLowerCameraID(int iDevID, int iLowerCameraID)
{
	int cameraid = -1;
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf,"SELECT CameraID FROM vnmp_CameraInfo WHERE DevID = %d AND (Input like '%%%d') ", iDevID, iLowerCameraID);

		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		if(!q.eof()){
			if (!q.fieldIsNull("CameraID"))
			{
				cameraid = atoi(q.fieldValue("CameraID"));
				if(cameraid > 0)
					return cameraid;
			}
			else
				return -1;
			return -1;
		} else {
			m_log.Add("CDataInfo::GetCameraIDByLowerCameraID:devid=(%d)[iLowerCameraID:%d].", iDevID, iLowerCameraID);
			return -1;
		}
	} catch (cppsqlite3::CppSQLite3Exception& e) {
		m_log.Add("CDataInfo::GetCameraIDByLowerCameraID: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return -1;
}

int CDataInfo::GetConfig(const char* configName, char* configValue)
{
#ifdef OLD_ITEMBASECONFIG_DB
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf, 0, sizeof(SqlBuf));

	int modleID = 0;
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		sprintf_s(SqlBuf, "select ItemName from vnmp_ItemBase where ItemType = 100 and ItemId = 1");
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);
		if(!q.eof())
		{
			if (!q.fieldIsNull("ItemName"))
			{
				modleID = atoi(q.fieldValue("ItemName"));
			}
		}else return -1;

		if( modleID <= 0 )return -1;

		sprintf_s(SqlBuf, "select Remark from vnmp_ItemBase where ItemType = %d and ItemName = '%s'", 200 + modleID, configName);
		q = m_sqlite3db.execQuery(SqlBuf);
		if(!q.eof())
		{
			if (!q.fieldIsNull("Remark"))
			{
				strcpy(configValue, q.fieldValue("Remark"));
				return 1;
			}
		}return -1;
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		//std::cerr  << "CDataInfo::getCoderidVector" << e.errorCode() << ":" << e.errorMessage() << std::endl;
		m_log.Add("CDataInfo::GetConfig: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
#endif
#ifdef OLD_SUBSYSTEMINFO_CONFIG_DB
	try
	{
		char SqlBuf[SQL_STRING_MAX]; memset(SqlBuf, 0, sizeof(SqlBuf));
		char chSubsysName[256]; memset(chSubsysName, 0, 256);

		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf, "select SubsysID, SubsysName as sysname, SubsysIPAddr as msip, RTSPPort as rtspport, CameraStatePort as camstateport, StatePort as mcstateport "
			" from vnmp_subsysteminfo where SubsysID in (select cast(ConfigureValue as integer) from vnmp_Configure where ConfigureType = 'MSSQLITE' and ConfigureName = 'MSSUBSYSID')");
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		if(!q.eof())
		{
			if ( (strcmp(configName, "MSIP") == 0) && !q.fieldIsNull("msip")) {
				strcpy(configValue, q.fieldValue("msip"));
				return 1;
			}
			if ( (strcmp(configName, "PORT_TOClient") == 0) && !q.fieldIsNull("rtspport")) {
				strcpy(configValue, q.fieldValue("rtspport"));
				return 1;
			}
			if ( (strcmp(configName, "PORT_TO_MC_CameraStuts") == 0) && !q.fieldIsNull("camstateport")) {
				strcpy(configValue, q.fieldValue("camstateport"));
				return 1;
			}
			if ( (strcmp(configName, "PORT_TO_MC") == 0) && !q.fieldIsNull("mcstateport")) {
				strcpy(configValue, q.fieldValue("mcstateport"));
				return 1;
			}
			if  (!q.fieldIsNull("sysname")) {
				strcpy(chSubsysName, q.fieldValue("sysname"));
			}
		}else return -1;

		sprintf_s(SqlBuf, "SELECT ConfigureName as name, ConfigureType as type, ConfigureValue as value, Remark"
			" FROM vnmp_configure WHERE ConfigureType = '%s' and ConfigureName = '%s'", chSubsysName, configName);
		q = m_sqlite3db.execQuery(SqlBuf);

		if ( !q.eof() && !q.fieldIsNull("value")) {
			strcpy(configValue, q.fieldValue("value"));
			return 1;
		}
		return -1;
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::GetConfig configName = %s: %d : %s ", configName, e.errorCode(), e.errorMessage());
		return -1;
	}
#endif

#ifdef NEW_CONFIG_DB_500
	try
	{
		char SqlBuf[SQL_STRING_MAX]; memset(SqlBuf, 0, sizeof(SqlBuf));
		char chSubsysName[256]; memset(chSubsysName, 0, 256);

		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf, "SELECT ConfigureName as name, ConfigureType as type, ConfigureValue as value, Remark"
			" FROM vnmp_configure WHERE ConfigureName = '%s'", configName);
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		if ( !q.eof() && !q.fieldIsNull("value")) {
			strcpy(configValue, q.fieldValue("value"));
			return 1;
		}
		return -1;
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::GetConfig configName = %s: %d : %s ", configName, e.errorCode(), e.errorMessage());
		return -1;
	}
#endif
	return 1;
}

int CDataInfo::GetSaveVideoUsers(std::vector<std::string>& vectorUsers)
{
	try
	{
		vectorUsers.clear();

		char SqlBuf[SQL_STRING_MAX]; memset(SqlBuf, 0, sizeof(SqlBuf));
		char chSubsysName[256]; memset(chSubsysName, 0, 256);

		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf, "SELECT ConfigureValue as value FROM vnmp_configure WHERE ConfigureType = 'UserRecordConfig'");
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		while (!q.eof())
		{
			if ( !q.fieldIsNull("value")) {
				std::string tUserName =  q.fieldValue("value");
				vectorUsers.push_back( tUserName );
			}
			q.nextRow();
		}

		return 1;
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::GetSaveVideoUsers error =  %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return 1;
}

bool CDataInfo::GetUserLevelAndUserType(char* szUserID, int CameraID, int& Level, int& userType)
{
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf," select Levels  from ( select max(Levels) as Levels from vnmp_RoleCameraInfo where ROLEID in "
			"(select distinct ROLEID from USERROLE where UserID = '%s') "
			" and CameraTeam in (select distinct CameraTeam from vnmp_CameraTeam where CameraID = %d));",
			szUserID, CameraID);

		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);
		if(!q.eof()){
			if (!q.fieldIsNull("Levels"))
			{
				Level = atoi(q.fieldValue("Levels"));
			}
			else
				return false;
			//if (!q.fieldIsNull("usertype"))
			//{
			//	userType = atoi(q.fieldValue("usertype"));
			//}
			//else
			//	return false;
			userType = 3;
			return true;
		} else {
			m_log.Add("CDataInfo::GetUserLevelAndUserType: no UserID[UserID:%d].", szUserID);
			return false;
		}
	} catch (cppsqlite3::CppSQLite3Exception& e) {
		m_log.Add("CDataInfo::GetUserLevelAndUserType: %d : %s ", e.errorCode(), e.errorMessage());
		return false;
	}
	return false;
}

bool CDataInfo::GetUserIDAndPassWD(const char* username, char* passwd_md5, char* szUserID)
{
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf,"select systemuserid as UserID,password as Password from systemuser where loginusername = '%s'; ", username);	
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		if(!q.eof()){
			if (!q.fieldIsNull("UserID"))
			{
				strcpy(szUserID , q.fieldValue("UserID"));
			}
			else
				return false;

			if (!q.fieldIsNull("Password"))
			{
				strcpy(passwd_md5, q.fieldValue("Password"));
			}
			else
				return false;

			return true;
		} else {
			m_log.Add("CDataInfo::GetUserID: no User[username:%s].", username);
			return false;
		}
	} catch (cppsqlite3::CppSQLite3Exception& e) {
		//std::cerr  << "CDataInfo::getDevInfo" << e.errorCode() << ":" << e.errorMessage() << std::endl;
		m_log.Add("CDataInfo::GetUserID: %d : %s ", e.errorCode(), e.errorMessage());
		return false;
	}
	return false;
}

int CDataInfo::GetCameraID(const char* devname, const char* input)
{
	int cameraid = -1;
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf,"SELECT CameraID FROM vnmp_CameraInfo WHERE  "
			"(DevID IN (SELECT  DevID  FROM vnmp_DevInfo  WHERE (DevName = '%s'))) AND (Input = '%s') ", devname, input);

		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		if(!q.eof()){
			if (!q.fieldIsNull("CameraID"))
			{
				 cameraid = atoi(q.fieldValue("CameraID"));
				 if(cameraid > 0)
					 return cameraid;
			}
			else
				return -1;
			return -1;
		} else {
			m_log.Add("CDataInfo::GetCameraID: no Camera in (%s)[input:%s].", devname, input);
			return -1;
		}
	} catch (cppsqlite3::CppSQLite3Exception& e) {
		//std::cerr  << "CDataInfo::getDevInfo" << e.errorCode() << ":" << e.errorMessage() << std::endl;
		m_log.Add("CDataInfo::GetCameraID: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return -1;
}

int CDataInfo::GetCameraIDWithInput(int iDevID, const char* input)
{
	int cameraid = -1;
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf,"SELECT CameraID FROM vnmp_CameraInfo WHERE  "
			"(DevID = %d ) AND (Input = '%s') ", iDevID, input);

		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		if(!q.eof()){
			if (!q.fieldIsNull("CameraID"))
			{
				cameraid = atoi(q.fieldValue("CameraID"));
				if(cameraid > 0)
					return cameraid;
			}
			else
				return -1;
			return -1;
		} else {
			m_log.Add("CDataInfo::GetCameraIDWithInput: [iDevID:%d][devid:%s].", iDevID, input);
			return -1;
		}
	} catch (cppsqlite3::CppSQLite3Exception& e) {
		//std::cerr  << "CDataInfo::getDevInfo" << e.errorCode() << ":" << e.errorMessage() << std::endl;
		m_log.Add("CDataInfo::GetCameraIDWithInput: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return -1;
}

int CDataInfo::GetPTZBackInfoByCameraID(int cameraID, int& iMinute,int &Preset)
{
	char SqlBuf[1200];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf,"select * from vnmp_RecordCamera where CameraID ='%d' ", cameraID);	
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);
		if(!q.eof())
		{
			iMinute = Preset = -1;
			if (!q.fieldIsNull("PTZBackMinute"))
			{
				iMinute = atoi(q.fieldValue("PTZBackMinute"));
			}
			if(!q.fieldIsNull("PTZPreset"))						
			{					   
				Preset = atoi(q.fieldValue("PTZPreset"));
			}
			if( iMinute < 1200 && iMinute >= 1 && Preset >= 1 && Preset < 256 )
				return 1;
			else
				return -1;
		}
		else
		{
			m_log.Add("CDataInfo::GetPTZBackInfoByCameraID: no cameraID[cameraID:%d].", cameraID);
			return -1;
		}
	}
	catch (cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::GetPTZBackInfoByCameraID: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return 0;
}

int CDataInfo::GetRecordCameraURLList(std::vector<std::string>& CameraURLArray)
{
	char SqlBuf[4096];
	memset(SqlBuf, 0, sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		sprintf_s(SqlBuf, "SELECT CameraID, IP, Port from vnmp_RecordCamera");
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		char ip[256]; 
		int port = 0;
		while (!q.eof())
		{  
			int cameraid = atoi(q.fieldValue("CameraID"));

			memset(ip, 0, 256);
			if (!q.fieldIsNull("IP"))
				strcpy(ip, q.fieldValue("IP"));

			if (!q.fieldIsNull("Port"))
				port = atoi(q.fieldValue("Port"));

			char URL[256]; memset(URL, 0, 256);
			sprintf( URL, "rtsp://%s:%d/%d", ip, port, cameraid );
			std::string strurl = URL;
			CameraURLArray.push_back(strurl);

			q.nextRow();
		}
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::GetRecordCameraURLList error = %d errorinfo = %s", e.errorCode(), e.errorMessage());
		return -1;
	}
	return 0;
}

int CDataInfo::GetInspectStrategyID()
{
	char SqlBuf[4096];
	memset(SqlBuf, 0, sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		sprintf_s(SqlBuf, "SELECT * FROM vnmp_InspectStrategy ORDER BY InspectLevel DESC");
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		while (!q.eof())
		{  
			if ( q.fieldIsNull("StrategyID") ){
				q.nextRow(); continue;
			}

			int stategyid = atoi(q.fieldValue("StrategyID"));

			char starttime[256]; memset(starttime, 0, 256);
			if (!q.fieldIsNull("StartTime"))
				strcpy(starttime, q.fieldValue("StartTime"));

			try
			{
				boost::posix_time::ptime starttime_ = boost::posix_time::from_iso_string(starttime);
				boost::posix_time::ptime nowtime_(boost::posix_time::second_clock::local_time());
				if( nowtime_ >= starttime_ )
					return stategyid;
			}catch (...){ }
			q.nextRow();
		}
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::GetInspectStrategyID error = %d errorinfo = %s", e.errorCode(), e.errorMessage());
		return -1;
	}

	return -1;
}

bool CDataInfo::UpdateStartTimeByStrategyID(int strategyid)
{
	char SqlBuf[4096];
	memset(SqlBuf, 0, sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf, "SELECT * FROM vnmp_InspectStrategy WHERE StrategyID = %d", strategyid);
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		if (!q.eof())
		{  
			if ( q.fieldIsNull("StrategyID") ){
				return false;
			}

			int stategyid = atoi(q.fieldValue("StrategyID"));
			int inspectcycle = 1;
			if (!q.fieldIsNull("InspectCycle"))
				inspectcycle = atoi( q.fieldValue("InspectCycle") );
 
			if( inspectcycle <= 0 )//hours
				inspectcycle = 24;
			//inspectcycle = 0; //测试使用 一般肯定不能用，负责会循环巡检

			boost::posix_time::time_duration td(inspectcycle, 0, 0);
			boost::posix_time::ptime nowtime_(boost::posix_time::second_clock::local_time());
			boost::posix_time::ptime starttime_ = nowtime_ + td;

			sprintf_s(SqlBuf, "update vnmp_InspectStrategy set StartTime = '%s' WHERE StrategyID = %d",
				boost::posix_time::to_iso_string(starttime_).c_str(), stategyid);
			if( m_sqlite3db.execDML(SqlBuf) > 0)
				return true;
		}
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::UpdateStartTimeByStrategyID error = %d errorinfo = %s", e.errorCode(), e.errorMessage());
		return false;
	}
	return false;
}

void CDataInfo::GetCameraListByStrategyID(int strategyid, std::list<int>& cameralist)
{
	char SqlBuf[4096];
	memset(SqlBuf, 0, sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		cameralist.clear();
		sprintf_s(SqlBuf, "SELECT * FROM vnmp_Camerateam WHERE CameraTeam = %d ORDER BY CameraID ", strategyid);
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		while (!q.eof())
		{  
			int cameraid = 0;

			if (!q.fieldIsNull("CameraID"))
				cameraid = atoi( q.fieldValue("CameraID") );
			cameralist.push_back(cameraid);

			q.nextRow();
		}
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::GetCameraListByStrategyID error = %d errorinfo = %s", e.errorCode(), e.errorMessage());
	}
}

bool CDataInfo::GetMSURLAndTypeByStrategyID(int strategyid, char* ms_url, int& InspectType)
{
	if(ms_url == NULL )return false;

	char SqlBuf[4096];
	memset(SqlBuf, 0, sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		sprintf_s(SqlBuf, "SELECT * FROM vnmp_InspectStrategy WHERE StrategyID = %d", strategyid);
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		if (!q.eof())
		{  
			if ( q.fieldIsNull("StrategyID") ){
				return false;
			}
			if (!q.fieldIsNull("MS_URL")){
				strcpy(ms_url, q.fieldValue("MS_URL"));
			}
			if (!q.fieldIsNull("InspectType")){
				InspectType = atoi( q.fieldValue("InspectType") );
			}
			return true;
		}
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::GetMSURLAndTypeByStrategyID error = %d errorinfo = %s", e.errorCode(), e.errorMessage());
		return false;
	}
	return false;
}

bool CDataInfo::CreateCheckVideoStrategy(int rtsp_port, int iInspectType, int iInspectCycle)
{
	char SqlBuf[4096];
	memset(SqlBuf, 0, sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		int strategyid = STRATEGYID_ITSSELF;
		boost::posix_time::ptime nowtime_(boost::posix_time::second_clock::local_time());
		sprintf_s(SqlBuf, "REPLACE INTO vnmp_InspectStrategy (StrategyID, InspectName, InspectLevel, InspectCycle, StartTime, InspectType, MS_URL) "
			" VALUES (%d, 'itself', 3, %d, '%s', %d, 'rtsp://127.0.0.1:%d/')", strategyid, iInspectCycle, boost::posix_time::to_iso_string(nowtime_).c_str(),
			iInspectType, rtsp_port);

		m_sqlite3db.execDML(SqlBuf);

		sprintf_s(SqlBuf, "Delete from vnmp_CameraTeam Where CameraTeam = %d;", strategyid);
		m_sqlite3db.execDML(SqlBuf);

		sprintf_s(SqlBuf, "select CameraID from vnmp_CameraInfo;" );
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		std::string strSQL;
		strSQL += "BEGIN; ";
		while (!q.eof())
		{
			if(q.fieldIsNull("CameraID")) continue;
			int cameraid = atoi(q.fieldValue("CameraID"));
			strSQL += " INSERT INTO vnmp_CameraTeam ( CameraTeam, CameraID ) VALUES ( " + boost::lexical_cast<std::string>(strategyid)
				+ ", " + boost::lexical_cast<std::string>(cameraid) + " ); ";
			q.nextRow();
		}
		strSQL += " COMMIT; ";

		m_sqlite3db.execDML( strSQL.c_str() );

		return true;
	} catch(...){ }
	return false;
}

bool CDataInfo::GetIsNeedToRecordCamera(int iCameraID)
{
	char SqlBuf[4096];
	memset(SqlBuf, 0, sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		sprintf_s(SqlBuf, "SELECT CameraID, RecordMode FROM vnmp_RecordCamera where CameraID = %d", iCameraID);
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);
		if ( q.eof() )return false;

		int iRecordMode = 0;
		if (!q.fieldIsNull("RecordMode")){
			iRecordMode = atoi(q.fieldValue("RecordMode"));
			if( iRecordMode == 0 )
				return true;
		}

		boost::posix_time::time_duration time_zero = boost::posix_time::time_duration(99,59,59);
		boost::posix_time::ptime nowtime_(boost::posix_time::second_clock::local_time());
		boost::posix_time::time_duration time_du[9] = { time_zero,time_zero,time_zero,time_zero,time_zero,time_zero,time_zero,time_zero,time_zero  };
		time_du[0] = nowtime_.time_of_day();

		sprintf_s(SqlBuf, "SELECT * FROM vnmp_TempleteByDay where TemplateID = %d", iRecordMode);
		q = m_sqlite3db.execQuery(SqlBuf);
		if ( q.eof() )return false;

		char time_tmp[256]; 
		std::string string_time_temp;
		for( int i = 1; i  <= 8; i++ ) {
			if ( !q.fieldIsNull(i) ) {
				memset(time_tmp, 0, 256);
				strcpy(time_tmp, q.fieldValue(i));
				if( strlen(time_tmp) < 6 )continue;
				string_time_temp = time_tmp;
				try
				{
					time_du[i] = boost::posix_time::duration_from_string(string_time_temp);
				}catch (...){
					m_log.Add( "CDataInfo::GetIsNeedToRecordCamera duration_from_string string_time_temp = %s", time_tmp );
				}
			}
		}

		std::vector<std::string> strtime;
		for( int j = 0;  j < 9; j++ ){
			strtime.push_back(boost::posix_time::to_simple_string(time_du[j]));
		}

		for ( int i = 1 ; i < 9; i += 2 )
		{
			if( time_du[i] != time_zero && time_du[i + 1] != time_zero && time_du[i] < time_du[i + 1] )
			{
				if( time_du[0] >= time_du[i] && time_du[0] <= time_du[i + 1] )
					return true;
			}
		}
	}
	catch( cppsqlite3::CppSQLite3Exception& e )
	{
		m_log.Add("CDataInfo::GetIsNeedToRecordCamera error = %d errorinfo = %s", e.errorCode(), e.errorMessage());
		return false;
	}
	return false;
}

int CDataInfo::GetDevInfoList(std::vector<DBDEVINFO>& devInfoVector, bool bNeedNetState)
{
	char SqlBuf[4096];
	memset(SqlBuf, 0, sizeof(SqlBuf));
	devInfoVector.clear();
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		if( bNeedNetState )
			sprintf_s(SqlBuf, "SELECT DevID as DevID, DevName, IPAddr, Port as Port, LoginUser as LoginUser,"
			" NetState as NetState, UpdateTime as UpdateTime from vnmp_Devinfo");
		else
			sprintf_s(SqlBuf, "SELECT DevID as DevID, DevName, IPAddr, Port as Port from vnmp_Devinfo");

		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		int port = 0;
		while (!q.eof())
		{  
			DBDEVINFO devinfo;
			devinfo.DevID = atoi(q.fieldValue("DevID"));

			if (!q.fieldIsNull("DevName"))
			{
				//strcpy(devinfo.User, q.fieldValue("DevName"));
				char *gb2312;
				size_t size = CodeConverter::Utf8ToGB2312((char *)q.fieldValue("DevName"),
					(size_t)strlen(q.fieldValue("DevName")) + 1, &gb2312);
				strcpy(devinfo.User,gb2312);
				delete gb2312;
			}

			if (!q.fieldIsNull("IPAddr"))
				strcpy(devinfo.IPAddr, q.fieldValue("IPAddr"));
			if (!q.fieldIsNull("Port"))
				devinfo.Port = atoi(q.fieldValue("Port"));

			if( bNeedNetState ) {
				if (!q.fieldIsNull("NetState"))
					devinfo.DevType = atoi(q.fieldValue("NetState"));
				if (!q.fieldIsNull("UpdateTime"))
					strcpy(devinfo.Pwd, q.fieldValue("UpdateTime"));
			}

			devInfoVector.push_back(devinfo);
			q.nextRow();
		}
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::GetDevInfoList error = %d errorinfo = %s", e.errorCode(), e.errorMessage());
		return -1;
	}
	return 0;
}

int CDataInfo::UpdateDevNetStatus(int iDevID, int iNetStatus)
{
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf,"update vnmp_DevInfo set NetState = %d, updatetime = '%sZ, where DevID = %d",
			iNetStatus, boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time()).c_str(), iDevID);
		return m_sqlite3db.execDML( SqlBuf );
	}
	catch (cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::UpdateDevNetStatus: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return -1;
}

int CDataInfo::UpdateCameraStatus(int iCameraID, int iOnline, int iStatus)
{
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf,"REPLACE INTO vnmp_CameraState (CameraID, vnmpOnline, CameraID, LastUpdateTime) VALUES (%d, %d, %d, '%sZ');",
			iCameraID, iOnline, iStatus, boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time()).c_str());
		return m_sqlite3db.execDML( SqlBuf );
	}
	catch (cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::UpdateCameraStatus: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return -1;
}

int CDataInfo::GetCameraIDsByDevID(int devID, std::vector<int>& cameraIDVector)
{
	char SqlBuf[4096];
	memset(SqlBuf, 0, sizeof(SqlBuf));
	cameraIDVector.clear();
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		sprintf_s(SqlBuf, "SELECT CameraID from vnmp_CameraInfo where DevID = %d", devID);
		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);

		int port = 0;
		while (!q.eof())
		{  
			if ( !q.fieldIsNull("CameraID") )
				cameraIDVector.push_back( atoi(q.fieldValue("CameraID")) );
			q.nextRow();
		}
		return 0;
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::GetCameraIDsByDevID error = %d errorinfo = %s", e.errorCode(), e.errorMessage());
		return -1;
	}
	return 0;
}

void CDataInfo::GetAllCameraList(std::vector<STCAMERAINFO>& vtCameraList)
{
	char msip[256];memset(msip, 0, 256);
	char msport[256];memset(msport, 0, 256);
	GetConfig( "MSIP", msip );
	GetConfig("PORT_TOClient", msport);

	char SqlBuf[4096];
	memset(SqlBuf, 0, sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		sprintf_s(SqlBuf, "SELECT vnmp_Camerainfo.CameraID, CameraName, vnmpOnline as vnmpOnline,"
			" Status as Status from vnmp_Camerainfo left join vnmp_CameraState on vnmp_Camerainfo.CameraID = vnmp_CameraState.CameraID;");

		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);
		int port = 0;
		while (!q.eof())
		{  
			STCAMERAINFO camerainfo;

			if (!q.fieldIsNull("CameraID"))
				strcpy(camerainfo.chRemark, q.fieldValue("CameraID"));

			if (!q.fieldIsNull("CameraName"))
			{
				strcpy(camerainfo.chCameraName, q.fieldValue("CameraName"));
				//char *gb2312;
				//size_t size = CodeConverter::Utf8ToGB2312((char *)q.fieldValue("CameraName"),
				//	(size_t)strlen(q.fieldValue("CameraName")) + 1, &gb2312);
				//strcpy(camerainfo.chCameraName, gb2312);
				//delete gb2312;
			}

			if (!q.fieldIsNull("vnmpOnline"))
				camerainfo.iOnline = atoi(q.fieldValue("vnmpOnline"));
			else
				camerainfo.iOnline = 1;
			if (!q.fieldIsNull("Status"))
				camerainfo.iStatus = atoi(q.fieldValue("Status"));
			else
				camerainfo.iStatus = 0;

			strcpy(camerainfo.chCameraInput, "rtsp://");
			strcat(camerainfo.chCameraInput, msip);
			strcat(camerainfo.chCameraInput, ":");
			strcat(camerainfo.chCameraInput, msport);
			strcat(camerainfo.chCameraInput, "/");
			strcat(camerainfo.chCameraInput, camerainfo.chRemark);

			vtCameraList.push_back(camerainfo);
			q.nextRow();
		}
	}
	catch(cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::GetAllCameraList error = %d errorinfo = %s", e.errorCode(), e.errorMessage());
	}
}

int CDataInfo::GetMaxCameraID(int iDevID, int iUnitID)
{
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);

		if( iUnitID >= 1000 && iUnitID <= 9999 )
			sprintf_s(SqlBuf,"select MAX(CameraID) as CameraID from vnmp_CameraInfo where UnitID = %d", iUnitID);
		else
			sprintf_s(SqlBuf,"select MAX(CameraID) as CameraID from vnmp_CameraInfo where UnitID = %d", iDevID/100000);

		cppsqlite3::CppSQLite3Query q = m_sqlite3db.execQuery(SqlBuf);
		if(!q.eof())
		{
			if (!q.fieldIsNull("CameraID"))
			{
				return atoi(q.fieldValue("CameraID"));
			}
			return -1;
		}
		else
		{
			return -1;
		}
	}
	catch (cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::GetMaxCameraID: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return -1;
}

int CDataInfo::AddCameraInfo(DBCAMERAINFO* camera)
{
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf,"insert into vnmp_CameraInfo (CameraID, CameraName, Input, DevID, Remark, UnitID) values (%d, '%s', '%s', %d, '%s', %s)",
			camera->CameraID, camera->CameraName, camera->Input, camera->DevID, camera->Remark, camera->UnitName);
		return m_sqlite3db.execDML( SqlBuf );
	}
	catch (cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::AddCameraInfo: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return -1;
}

int CDataInfo::UpdateCameraInfo(int iCameraID, const char* chCameraName, const char* chRemark)
{
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf,"update vnmp_CameraInfo set CameraName = '%s',Remark = '%s' where CameraID = %d ", chCameraName, chRemark, iCameraID);
		return m_sqlite3db.execDML( SqlBuf );
	}
	catch (cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::UpdateCameraInfo: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return -1;
}

int CDataInfo::DelCameraInfo(int iCamera)
{
	char SqlBuf[SQL_STRING_MAX];
	memset(SqlBuf,0,sizeof(SqlBuf));
	try
	{
		boost::lock_guard<boost::mutex> lock_(m_db_mutex_);
		sprintf_s(SqlBuf,"Delete from vnmp_CameraInfo where CameraID = %d ", iCamera);
		return m_sqlite3db.execDML( SqlBuf );
	}
	catch (cppsqlite3::CppSQLite3Exception& e)
	{
		m_log.Add("CDataInfo::DelCameraInfo: %d : %s ", e.errorCode(), e.errorMessage());
		return -1;
	}
	return -1;
}

void CDataInfo::UpdateAllCameraList(int iDevID, unsigned long lCameraNum, STCAMERAINFO* CameraInfoArray)
{
}
