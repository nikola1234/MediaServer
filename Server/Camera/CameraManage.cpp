#include "boost/filesystem.hpp"
#include "CameraManage.h"

ManageCamera::ManageCamera()
{
	boost::filesystem::create_directory("log");
	m_log.InitLog("./log/Cam-");
	m_log.Add("Manage Camera Start !");

}

ManageCamera::~ManageCamera()
{

}

int ManageCamera::read_CameraID_from_DB(char * path)
{
	int ret = 0;
	sqlite3* db = NULL;
	char sqlbuf[1024];
	char *ErrMsg=NULL;
	char **pszResult = NULL;
	char open_db_result = 0;

	int nRow = 0;
	int nColumn = 0;

	memset(sqlbuf,0,sizeof(sqlbuf));

	open_db_result = sqlite3_open(path,&db);
	sprintf(sqlbuf,"select CameraID from vnmp_CameraInfo;");
	ret = sqlite3_get_table(db, sqlbuf, &pszResult, &nRow, &nColumn, &ErrMsg);
	if (ret != SQLITE_OK)
	{
		 m_log.Add("%s(%d),SQL error! %s" ,DEBUGARGS,ErrMsg);
		sqlite3_free(ErrMsg);
		ErrMsg = NULL;
		sqlite3_free_table(pszResult);
		sqlite3_close(db);
		return -1;
	}
	else
	{
		int num = 0;
		for (int i = 0; i <= nRow; ++i)
		{
			for (int j = 0; j < nColumn; ++j)
			{
				int n= atoi(*(pszResult + nColumn * i + j));	
				if( n != 0){
					CamID.push_back((uint32)n);
					num++;
				}
			}
		}
	}
	if (ErrMsg != NULL) sqlite3_free(ErrMsg);
	ErrMsg = NULL;
	sqlite3_free_table(pszResult);
	sqlite3_close(db);
	return 0;

}

int ManageCamera::InitFromDB()
{
	char DB_file[20] ="MS.sqlite";
	read_CameraID_from_DB(DB_file);
  	return 0;
}

int ManageCamera::try_to_open(string stream)
{
	VideoCapture  vcap;
	 if(!vcap.open(stream)) {
		 m_log.Add("%s(%d),try to open failed!" ,DEBUGARGS);
	        return -1;
	}
	return 0;
}

uint32  ManageCamera::get_camera_id()
{	uint32 ID = 0;
	ReadLock read_lock (m_CamIDListMutex_);
	if(CamID.size() >0)
		ID = CamID.front();
	read_lock.unlock();
	WriteLock write_lock(m_CamIDListMutex_);
	CamID.pop_front(); 
	return ID;
}

SingleCamPtr ManageCamera::search_camera_by_id(uint32 ID)
{
	SingleCamPtr tmpcamptr=NULL;
	ReadLock readlock_(m_SinCamListMutex_);
	std::list<SingleCamPtr>::iterator it = m_SinCamList.begin();
	for ( ; it != m_SinCamList.end() ; it++ )
	{
		if ((*it)->GetCameraID() == ID)
		{
			tmpcamptr = *it;
			return tmpcamptr;
		}
	}
	return tmpcamptr;
}

SingleCamPtr ManageCamera::search_camera_by_url(char *url )
{
	uint32 ID= 0;
	SingleCamPtr tmpcamptr=NULL;
	m_CamList.search_cam_by_url(url, ID);
	if(ID > 0)
	{
		tmpcamptr = search_camera_by_id(ID);
	}else{
		m_log.Add("%s(%d),get camer ptr fail!" ,DEBUGARGS);
	}
	return tmpcamptr;
}	

int ManageCamera::remove_camera_by_id(uint32 ID)
{
	WriteLock writelock_(m_SinCamListMutex_);
	std::list<SingleCamPtr>::iterator it = m_SinCamList.begin();
	for ( ; it != m_SinCamList.end() ; it++ )
	{
		if ((*it)->GetCameraID() == ID)
		{
			m_SinCamList.erase(it);
			return 0;
		}
	}
	m_log.Add("%s(%d),remove camera by id  fail!" ,DEBUGARGS);
	return -1;
}

int ManageCamera::resume_cameraID_in_list(uint32 ID)
{	
	int iRet = -1;
	iRet = m_CamList.delete_cam_by_id(ID);
	if(iRet < 0){
		m_log.Add("%s(%d),resume_cameraID_in_list  fail!" ,DEBUGARGS);
		return -1;
	}

	WriteLock writelock_(m_CamIDListMutex_);
	CamID.push_back(ID);	
	return 0;
}

int  ManageCamera::reset_camera_param(uint32 ID,ST_VDCS_VIDEO_PUSH_CAM & addCam,string &url)
{
	SingleCamPtr camptr =NULL;
	camptr = search_camera_by_id(ID);
	if(camptr == NULL)return -1;
	
	WriteLock writelock_(m_SinCamListMutex_);
	camptr->reset_camera_fix_param(addCam);
	writelock_.unlock();
	
	ReadLock readlock_(m_SinCamListMutex_);
	url =camptr->get_rtsp_url();
	if(url.length() == 0){
		m_log.Add("%s(%d),wrong rtsp url!" ,DEBUGARGS);
		return -1;
	}
	return 0;
}
int ManageCamera::add_camID_list(char *&url ,uint32 ID)
{
	int iRet =  m_CamList.add_cam_list(url,ID);
	return iRet;
}


int ManageCamera::Set_or_Renew_Camera_Param(T_VDCS_VIDEO_CAMERA_PARAM* pt_CameraParam,vector <VIDEO_DRAW> & Pkg)
{
	int iRet = -1;
	int ID = -1;
	iRet = m_CamList.search_cam_by_url(pt_CameraParam->CameUrL,ID);
	if(iRet = -1)
	{
		m_log.Add("%s(%d),get camer id failure!" ,DEBUGARGS);
		return -1;
	}
	
	SingleCamPtr camptr =NULL;
	camptr = search_camera_by_id(ID);
	if(camptr == NULL)
	{
		m_log.Add("%s(%d),search camera by ID failure!" ,DEBUGARGS);
		return -1;
	}

	WriteLock writelock_(m_SinCamListMutex_);
	camptr->reset_camera_var_param(pt_CameraParam,Pkg);
	writelock_.unlock();
	
	return 0;
}

string ManageCamera::Create_or_Renew_Camera(ST_VDCS_VIDEO_PUSH_CAM & addCam)
{
	int iRet = -1;
	int ID = -1;
	string url = "TT";
	
	iRet = m_CamList.search_cam_by_url(addCam.CameUrL,ID);
	if(iRet = -1){ /* have no camera */
		ID = get_camera_id();
		if(ID == 0){
			m_log.Add("%s(%d),get camer id failure!" ,DEBUGARGS);
			return url;
		}
		SingleCamPtr camptr = SingleCamPtr(new SingleCamera(this,(uint32)ID));
		WriteLock writelock_(m_SinCamListMutex_);
		m_SinCamList.push_front(camptr);
		writelock_.unlock();
		camptr->set_camera_fix_param(addCam);
		url =camptr->get_rtsp_url();
		if(url.length() == 0){
			m_log.Add("%s(%d),wrong rtsp url!" ,DEBUGARGS);
			return url;
		}
		//add list
		iRet =add_camID_list(addCam.CameUrL,(uint32)ID);
		if(iRet < 0){
			string tmpurl ="TT";
			m_log.Add("%s(%d),add_camID_list failure!" ,DEBUGARGS);
			return tmpurl;
		}
		//Add_or_Renew_DB();
		return url;
	}
	
	if(iRet == 0){ /* have exist camera */
		if(ID <= 0){
			m_log.Add("push exist camera %s and ID= %d return failure !" ,addCam.CameUrL,ID);
			return url;
		}
		reset_camera_param((uint32)ID,addCam,url); /*stop analyze*/
		//Add_or_Renew_DB();
		return url;
	}

	return url;
}

