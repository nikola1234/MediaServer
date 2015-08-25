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

//#include "VideoDevice.h"
#include "CppSQLite3/CppSQLite3.h"
#include "MyLog.h"
#include "ICamera.h"

#define SQL_STRING_MAX 4096
#define STRATEGYID_ITSSELF  913156789

#define NEW_CONFIG_DB_500 //5.0

//#define OLD_SUBSYSTEMINFO_CONFIG_DB
//#define OLD_ITEMBASECONFIG_DB
//#define OLD_VIRTUALCHANNEL

#define sprintf_s sprintf
#define strcpy_s strcpy

class CDataInfo:
	public IData
{

public:
	CDataInfo();
	virtual ~CDataInfo();

	virtual int Open(const char* szfilepath); //PG

	virtual int getCameraInfo(int cameraid, DBCAMERAINFO* camera); //PG
	virtual int getCoderInfo(int CoderID, DBCODERINFO* coder); //PG
	virtual int getMatrixInfo(int MatrixID, DBMATRIXINFO* MatrixInfo); //PG
	virtual int getDevInfo(int DevID, DBDEVINFO* dev); //PG

	virtual int getUnitNameFormDict(int cameraid, char* chUnitName){ return -1; }; //PG

	//OLD_VIRTUALCHANNEL
	int getMatrixInfoByControlID(int UnitID, int& ControlID, DBMATRIXINFO *MatrixInfo);

	//VIRTUALCHANNEL 5.0
	virtual void getCoderidVector(std::vector<int> &coderid_list,int matrixid);  //PG
	virtual void getVirtualCoderidVector(int UnitID, std::vector<int> &coderid_vector, int& MatrixID); //PG

	virtual int GetCameraIDByLowerCameraID(int iDevID, int iLowerCameraID);  //PG

	//配置相关
	virtual int GetConfig(const char* configName, char* configValue);  //PG
	virtual void SetConfig(const char* configName, const char* configValue){ }; //PG
	virtual int GetSaveVideoUsers(std::vector<std::string>& vectorUsers); //PG

	//用户认证相关 OLD_VIRTUALCHANNEL
	bool GetUserLevelAndUserType(char* szUserID, int CameraID, int& Level, int& userType);
	bool GetUserIDAndPassWD(const char* username, char* passwd_md5, char* szUserID);

	//存储相关
	virtual int GetPTZBackInfoByCameraID(int cameraID, int& iMinute,int &Preset); //PG
	virtual int GetRecordCameraURLList(std::vector<std::string>& CameraURLArray); //PG
	virtual bool GetIsNeedToRecordCamera(int iCameraID); //PG
	//事件录像相关
	virtual int ExecuteSQL(std::string& strSQL){ return 0; };   //PG
	virtual int GetNewEventID(int& newEventID){ return 0; };   //PG
	virtual int InsertEventRecordInfo(int iCameraID, std::string& strStartTime, std::string& strRecordPath,
		std::string& strRecordUrl, int iRecordType, int iEventRecordID, std::string& strTableName){ return 0; };   //PG

	//巡检相关 MS自动巡检 后四个函数需要重新打开新SQLite
	virtual bool CreateCheckVideoStrategy(int rtsp_port, int iInspectType, int iInspectCycle); //PG
	int GetInspectStrategyID();
	bool UpdateStartTimeByStrategyID(int strategyid);
	void GetCameraListByStrategyID(int strategyid, std::list<int>& cameralist);
	bool GetMSURLAndTypeByStrategyID(int strategyid, char* ms_url, int& InspectType);

	//MS DevNetStatus
	virtual int GetDevInfoList(std::vector<DBDEVINFO>& devInfoVector, bool bNeedNetState = false); //PG
	virtual int GetCameraIDsByDevID(int devID, std::vector<int>& cameraIDVector);  //PG

	virtual int GetCameraID(const char* devname, const char* input);  //PG
	virtual int GetCameraIDWithInput(int iDevID, const char* input);   //PG

	virtual int GetDevNetStatus(int iDevID){ return 0; };   //PG

	//MSDLL 更新数据库摄像机列表
	virtual void UpdateAllCameraList(int iDevID, unsigned long lCameraNum, STCAMERAINFO* CameraInfoArray); //PG

	//从MS接入单元，获得摄像机列表
	virtual void GetAllCameraList(std::vector<STCAMERAINFO>& vtCameraList); //PG

	//直接更新至Postgre数据库 PG
	typedef struct _MSLBINFO_
	{
		int	 iCameraID;
		int	 iClientNum;
		_MSLBINFO_()
		{
			iCameraID = 0;
			iClientNum = 0;
		}
	} MSLBINFO;

	//virtual int UpdateCameraStatus(int iCameraID, int iOnline, int iStatus){ return 0; };
	virtual int UpdateCameraStatus(int iCameraID, int iOnline, int iStatus);
	virtual int UpdateCameraGPS(int iCameraID, const char* chSmX, const char* chSmY, int iDirection, int iSpeed){ return 0; };
	virtual int InsertCameraCommand(const char* chUserName, int iCameraID, const char* chCommand,
		const char* chParam1, const char* chParam2, const char* chRemark){ return 0; };
	virtual int UpdateMSLoadBalance(std::vector<MSLBINFO>& vMSLoadBalance){ return 0; };

	//virtual int UpdateDevNetStatus(int iDevID, int iNetStatus){ return 0; };
	virtual int UpdateDevNetStatus(int iDevID, int iNetStatus);
	virtual int InsertAlarmInfo(int iAlarmType, int iAlarmNO, const char* chAlarmParam, const char* chAlarmData, const char* chAlarmMessage){ return 0; };
	virtual int InsertSubSystemException(const char* chExceptionMsg){ return 0; };

//private:
public:

	//static int __stdcall funHandleRecvCommand(int user, int comtype, const char* comstring);
	//UpdateAllCameraList 内部调用
	int GetMaxCameraID(int iDevID, int iUnitID);
	int AddCameraInfo(DBCAMERAINFO* camera);
	int UpdateCameraInfo(int iCameraID, const char* chCameraName, const char* chRemark);
	int DelCameraInfo(int iCamera);

public:
	CMyLog m_log;

protected:
	boost::mutex m_db_mutex_;
	std::string m_szFileDB;
	cppsqlite3::CppSQLite3DB m_sqlite3db;
};

#endif // !defined(AFX_DATAINFO_H__45D6DFB6_DE4F_4A45_A1AB_694F51FE2074__INCLUDED_)
