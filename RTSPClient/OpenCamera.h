#pragma once

#include "boost/atomic.hpp"
#include <string>

#include "RTSPClient.h"

#include "Log.h"
extern CLog g_MSNetSDKlog;

class COpenCamera
{
	enum TRANSPORT_TYPE {
		TRANSPORT_UDP = 0,
		TRANSPORT_TCP,
		TRANSPORT_RAW
	};

#define MONITOROPENCAMERA_TIME_SEC 30 //120

public:
	COpenCamera();
	virtual ~COpenCamera(void);

public:
	int InitOpenCamera(char* url, int transport_type = TRANSPORT_UDP);
	int StartCamera(void* capture_callback, void* user,
		bool iscallbackex = false, const char* Range_clock = "-", float scale = 1.0);
	virtual int RecvData(int timestamp, unsigned long dwDataType, unsigned char *data, unsigned long len);
	int SwitchCamera(int ioldCamera, int inewCamera);
	void ChangeCameraIDAndUrl(int inewCamera);
	int CameraPTZCtrl(int ptzCMD, int param1, int param2);
	void StopCamera();

	int GetMediaID(){ return m_mediaid; };
	std::string GetCameraName(){  std::string str; if(m_prtsp_client != NULL)str = m_prtsp_client->GetCurCameraName(); return str; }
	int GetCameraID(){ return m_cameraid; };
	int GetUser(){ return (int)m_user; };

	int GetSaveFileInfo(const char* Range_clock, char* resultStr);

	static int GetCameraIDFromURL(const char* sz_url);
	static int IsInTheSameRoad(int Caid1, int Caid2);

	int GetRTSPUrlOnvif( const char* sz_url, std::string& rtsp_url );

	std::list<int>& GetFullCameraList(){ return m_prtsp_client->GetCameraList(); };
	time_t GetCurRecvDataTime(){ return m_curRecvDataTime; };

	void ResetCurRecvDataTime(){ m_curRecvDataTime = time(NULL) + MONITOROPENCAMERA_TIME_SEC; m_nReConnectTimes = 0; };
	void PutCurRecvDataTime(){
		m_curRecvDataTime = time(NULL) + MONITOROPENCAMERA_TIME_SEC * m_nReConnectTimes++;
		if( m_nReConnectTimes >= 20 )m_nReConnectTimes = 20;
	};

	bool IsPlaySave(){ return m_bPlaySave; };
	int ReStartCamera();

	char* GetOpenErrorInfo(){ return m_openErrorInfo; };

private:
	int GetIPAndAddressByURL(const char* url);
	int GetRealUserNameAndLevels();

protected:
	CRTSPClient* m_prtsp_client;
	void* m_capture_callback;
	void* m_user;
	int m_cameraid;
	char m_url[256];
	float m_scale;
	bool m_bPlaySave;

private:
	char m_ip[256];
	int m_port;
	char m_username[256];
	char m_password[256];

	int m_transport_type;
	bool m_iscallbackex;

	static boost::atomic<int> s_mediaid;
	int m_mediaid;

	time_t m_curRecvDataTime;
	int m_nReConnectTimes;
	char m_openErrorInfo[256];

	int m_UserLevel;
	std::vector<int> m_channleLevels;

	boost::mutex m_mutex;

	std::string m_strOnvifIP;
	int m_iOnvifPort;
	std::string m_strOnvifUser;
	std::string m_strOnvifPwd;
	int m_iOnvifChannel;
};