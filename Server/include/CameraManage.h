#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "MyLog.h"

#include "Common.h"
#include "NetServer.h"
#include "SingleCamera.h"
#include "CmdDefine.h"
#include "Data.h"
#include "CppSQLite3.h"

class NetServer;
class SingleCamera;
class ManageCamera
{
public:  
	typedef boost::shared_ptr<SingleCamera> SingleCamPtr;	
	typedef boost::shared_lock<boost::shared_mutex> ReadLock;
	typedef boost::unique_lock<boost::shared_mutex> WriteLock;

public:

  ManageCamera();
  ~ManageCamera();

  void AddServer(NetServer *ourServer){Server = ourServer;}
  int InitFromDB();

  int try_to_open(string stream);
  string Create_or_Renew_Camera(ST_VDCS_VIDEO_PUSH_CAM & addCam);

private:
	uint32  get_camera_id();
	int read_CameraID_from_DB();
	SingleCamPtr search_cam_by_id(uint32 ID);
	int reset_camera_param(uint32 ID,ST_VDCS_VIDEO_PUSH_CAM & addCam,string &url);

private:
	std::list<SingleCamPtr> m_SinCamList;
	boost::shared_mutex m_SinCamListMutex_;

	std::list<uint32> CamID;
	boost::shared_mutex m_CamIDListMutex_;

	NetServer *Server;

	CamList m_CamList;

	CMyLog m_log;
};

#endif
