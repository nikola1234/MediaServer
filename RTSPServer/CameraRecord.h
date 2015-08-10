#pragma once

#ifndef CAMERARECORD_H
#define CAMERARECORD_H

#include "time.h"
#include "FileWrite.h"

//#define OLD_MANUAL_SAVA

#ifndef WIN32
#ifndef __stdcall
#define __stdcall __attribute__((__stdcall__))
#endif
#endif

namespace MediaSave
{

class CManageRecord;
class CRAW_RTSPClient;
class CCameraRecord
{
public:
	enum RECORDTYPE 
	{
		RECORD_TIME = 0,
		RECORD_ALARM ,
		RECORD_MANUAL,
		RECORD_USER
	};

public:
//	CCameraRecord(int camid, CManageRecord* pmanagerecord);
	CCameraRecord(CManageRecord* pmanagerecord, int recordType = RECORD_TIME);
	~CCameraRecord(void);

public:
	int InitCameraRecordByURL(char* url, bool bRealInit = true);
	int SwitchCameraByCameraID(int iNewCamera);
	int StartRecord();
	int RecordData(unsigned char *data, int len, unsigned int dwdatatype = 0);
	void StopRecord();

	bool GetCameraRecordIsOpen(){ return m_isopen_camera; };
	//void SetCameraRecordIsOpen(bool bIsOpen){ m_isopen_camera = bIsOpen; };
	time_t GetCurRecordTime(){ return m_cur_record_time; };

	int CameraPTZCtrl(int ptzCMD, int param1, int param2);

	time_t GetLastPtzAutoBackTime(){ return m_lastPtzAutoBackTime; };
	void SetLastPtzAutoBackTime(time_t t_){ m_lastPtzAutoBackTime = t_; };

	int GetCameraID(){ return m_cameraid; };

	void ResetStartRecordTime(){ m_startRecordTime = time(NULL);}
	time_t GetStartRecordTime(){ return m_startRecordTime; }

	int GetRecordType(){ return m_CameraRecordType; };

	static void __stdcall SVideoSwitchCameraUpdateCallback(unsigned long dwOldCameraID,
		unsigned long dwNewCameraID, unsigned char *NewCameraName, unsigned long user);

	void SetIsBadSocket( bool isBad ) { m_isBadSocket = isBad; };
	bool GetIsBadSocket(){ return m_isBadSocket; };

	static bool IsSaveDirectory(const char * str_save_path);
	bool CreateNewManualSaveFilePath();
	int GetEventRecordID(){ return m_EventRecordID; };
private:
	CRAW_RTSPClient* m_prtsp_client;
	int m_cameraid;

	time_t m_cur_record_time;
	bool m_filestate;
	bool m_isopen_camera;

	CManageRecord* m_manage_record;
	CFileWrite m_file_write;

	char m_ip[256];
	int m_port;
	char m_url[256];
	time_t m_lastPtzAutoBackTime;
	time_t m_startRecordTime;

	time_t m_startManualFileTime;
	time_t m_curFileIndexTime_t;

	int m_CameraRecordType;
	bool m_isBadSocket;

	char m_userName[256];
	char m_passWord[256];

	char m_chMRURL[256];

	unsigned char m_headData[HEAD_SIZE];
	int m_headLen;
	bool m_isHaveHeadData;

	int m_EventRecordID;
	std::string m_strEventTableName;
#ifndef OLD_MANUAL_SAVA
	char m_manualSavePath[256];
	char m_manualFileIndexPath[256];
#endif
};

}// end namespace

#endif // CAMERARECORD_H