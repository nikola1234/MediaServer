#pragma once

#include <list>
#include "boost/shared_ptr.hpp"
#include <boost/thread.hpp>
#include "boost/atomic.hpp"

//支持旧平台结构
struct _SCAMERLIST_
{
	int iCount; //摄像机列表长度
	int *CameraID; //摄像机列表
};
typedef struct _SCAMERLIST_ SCAMERALIST;

class COpenCamera;
class CConnectMC
{
public:
	typedef boost::shared_ptr<COpenCamera> OpenCameraPtr;

	typedef boost::shared_lock<boost::shared_mutex> readLock;
	typedef boost::unique_lock<boost::shared_mutex> writeLock;

public:
	CConnectMC(void);
	~CConnectMC(void);

	OpenCameraPtr CreateOpenCamera();
	OpenCameraPtr CreatePlaySave();

	OpenCameraPtr FindOpenCameraByMediaID(int mediaid);
	OpenCameraPtr FindOpenCameraByCameraID(int cameraid);
	OpenCameraPtr FindOpenCameraByUser(int user);

	int FindNoOpenCameraID(std::list<int>& cameraidList, int& cameraidNO);

	void RemoveOpenCameraByMediaID(int mediaid);

	int GetLoginHandle(){ return m_loginHandle; };

	bool IsExistConnectMC(char *ip, int port, char *username);

	void StopAllCamera();

	void MonitorOpenCameraThread();

	int m_clientType;
private:
	std::list<OpenCameraPtr> m_openCameraList;
	boost::shared_mutex m_openCameraMutex;

	static boost::atomic<int> s_loginHandle;
	int m_loginHandle;

	char m_mcip[256];
	int m_mcport;
	char m_username[256];
	char m_password[256];

	boost::shared_ptr< boost::thread > m_pMonitorThread;
	bool m_bMonitorRun;
};
