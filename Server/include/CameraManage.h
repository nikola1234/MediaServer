#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

#include "MyLog.h"

#include "Common.h"
#include "NetServer.h"
#include "SingleCamera.h"
#include "CmdDefine.h"
#include "Data.h"
#include "CppSQLite3.h"
#include "DataInfo.h"
#include "CameraList.h"

class NetServer;
class SingleCamera;

typedef boost::shared_ptr <SingleCamera> SingleCamPtr;

class ManageCamera
{
public:
	typedef boost::shared_lock<boost::shared_mutex> ReadLock;
	typedef boost::unique_lock<boost::shared_mutex> WriteLock;

public:

  ManageCamera();
  ~ManageCamera();

  void AddServer(NetServer *ourServer){Server = ourServer;}
  int InitFromDB();

  int try_to_open(char * ip ,char* stream);

  int Set_or_Renew_Camera_Param(T_VDCS_VIDEO_CAMERA_PARAM* pt_CameraParam,vector <VIDEO_DRAW> & Pkg);
  string Create_or_Renew_Camera(ST_VDCS_VIDEO_PUSH_CAM & addCam);

public:
	
	void erase_id_from_CamIDList(uint32 ID);
	void fill_push_cam(DBCAMERACONFI *pt_caminfo,ST_VDCS_VIDEO_PUSH_CAM*pt_addCam);
	void fill_var_param(DBCAMERAFUNCPARAM* pt_Camfuncparam,T_VDCS_VIDEO_CAMERA_PARAM*pt_varParam,vector <VIDEO_DRAW>   &PKG,uint8 num);


	void Add_Camerainfo_DB(ST_VDCS_VIDEO_PUSH_CAM* pt_addCam,string url,uint32 ID);
	void parse_time_for_db(T_AlarmTime *pt_time ,char *time_c);
	void Renew_camerainfo_DB(ST_VDCS_VIDEO_PUSH_CAM* pt_addCam,string url,uint32 ID);

	int parser_type(DBCAMERAFUNCPARAM*  pt_FuncParam,uint16 type);
	int set_camerafunc_DB(DBCAMERACONFI* pt_Caminfo);
	int  Renew_camerafunc_DB(T_VDCS_VIDEO_CAMERA_PARAM* pt_CameraParam,vector <VIDEO_DRAW> & PKG,int ID);


	int init_camera_from_DB();
	int read_CameraID_from_DB(char * path);
	SingleCamPtr search_camera_by_id(uint32 ID);
	SingleCamPtr search_camera_by_url(char *url);

	int add_camID_list(char *url ,uint32 ID);
	int remove_camera_by_id(uint32 ID);
	int resume_cameraID_in_list(uint32 ID);
	int remove_camera_from_db_by_id(uint32 ID);
	int reset_camera_param(uint32 ID,ST_VDCS_VIDEO_PUSH_CAM & addCam,string &url);


	uint32  get_camera_id();
	void  Get_Rest_Camlist();
	void  Get_Rest_SingleCamlist();

public:

	NetServer *Server;
	CamList m_CamList;

private:
	std::list<SingleCamPtr> m_SinCamList;
	boost::shared_mutex m_SinCamListMutex_;

	std::list<uint32> CamIDList;
	boost::shared_mutex m_CamIDListMutex_;

	CMyLog m_log;

	CamDataInfo CDataInfo;
};

#endif
