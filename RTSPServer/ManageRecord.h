#pragma once

#ifndef MANAGERECORD_H
#define MANAGERECORD_H

#include <vector>
#include "CameraRecord.h"
#include <boost/shared_ptr.hpp>
#include "CppSQLite3/CppSQLite3.h"
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include "http/server.hpp"
#include "DataInfo.h"

namespace MediaSave
{

class CPreAllocateDisk;
class CManageRecord
{
public:
#define CAMERA_COUNT_PRE_THREAD     10
#define RECORD_TIMEOUT_MAX_SECONDS  20
#define MANUAL_RECORD_TIME          300 //5*60=300
typedef boost::shared_ptr<CCameraRecord> CameraRecordPtr;

public:
	CManageRecord(CPreAllocateDisk *pall_disk);
	~CManageRecord(void);

public:
	int InitManageRecord(CDataInfo* pDataInfo);
	int StartRecordThread(int iManualPort = -1);
	void StopRecordThread();

	int AddManualCameraRecord( char* url, int recordType = CCameraRecord::RECORD_MANUAL );
	CPreAllocateDisk* GetPreAllocateDisk(){ return m_pre_allocate_disk; };

	CameraRecordPtr GetCameraRecordPtr(int CameraID);

private:
	void ManageRecordThread(int ibegin);
	//int GetPTZBackInfoByCameraID(int cameraID, int& iMinute, int &Preset);
	bool IsNeedToSave(int CameraID);
	//static int __stdcall funHandleRecvCommand(int user, int comtype, const char* comstring);
	//int GetConfig(const char* configName, char* configValue);

	void ManageManualRecordThread();

public:
	CDataInfo* m_pDataInfo;
	//cppsqlite3::CppSQLite3DB m_sqlite3db;
	//std::string m_szFileDB;
	//boost::mutex m_db_mutex_;

	CPreAllocateDisk* m_pre_allocate_disk;

private:
	std::vector< CameraRecordPtr > m_camera_recode_array;
	std::vector< boost::shared_ptr<boost::thread> > m_threadptr_array;

	std::list<CameraRecordPtr> m_ManualCamReclist;
	boost::mutex m_ManualCamArrayMutex;

	boost::shared_ptr<http_wl::server3::server> m_httpServerPtr;
	boost::shared_ptr<boost::thread> m_ManualThreadPtr;

	bool m_brun;
};

}// end namespace

#endif // MANAGERECORD_H