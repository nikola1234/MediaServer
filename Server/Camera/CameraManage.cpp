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

int ManageCamera::InitFromDB()
{
  	return 0;
}

int ManageCamera::try_to_open(string stream)
{
	VideoCapture  vcap;
	 if(!vcap.open(stream)) {
	        return -1;
	}
	return 0;
}

int ManageCamera::create_camera_id(ST_VDCS_VIDEO_PUSH_CAM & addCam)
{
	uint num = 0;
	int  iRet = 0;
	do{
		srand((unsigned)time(NULL));
		num =600300000 +(uint32 )((rand()%500)+1);
		ManCamReadLock readlock_(m_SinCamListMutex_);
		std::list<SingleCamPtr>::iterator it = m_SinCamList.begin();
		for ( ; it != m_SinCamList.end() ; it++ )
		{
			if ((*it)->GetCameraID() == num)
			{
				iRet = 1;
				break;
			}
			iRet = 0;
		}
    		readlock_.unlock();
	}while(iRet);

}

SingleCamPtr ManageCamera::search_cam_by_id(uint32 ID)
{
	SingleCamPtr tmpcamptr=NULL;
	ManCamReadLock readlock_(m_SinCamListMutex_);
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

int  ManageCamera::reset_camera_param(uint32 ID,ST_VDCS_VIDEO_PUSH_CAM & addCam,string &url)
{
	SingleCamPtr camptr =NULL;
	camptr = search_cam_by_id(ID);
	if(camptr == NULL)return -1;
	ManCamWriteLock writelock_(m_SinCamListMutex_);
	camptr->set_camera_param(addCam);
	writelock_.unlock();
	ManCamReadLock readlock_(m_SinCamListMutex_);
	url =camptr->get_rtsp_url();
	if(url.length() == 0){
		dbgprint("%s(%d),wrong rtsp url !\n", DEBUGARGS);
		return -1;
	}
	return 0;
}

string ManageCamera::Create_or_Renew_Camera(ST_VDCS_VIDEO_PUSH_CAM & addCam)
{
	int iRet = -1;
	int ID = -1;
	string url = "TT";
	
	iRet = m_CamList.search_cam_by_url(addCam.CameUrL,ID);
	if(iRet = -1){
		create_camera_id(addCam);
		set_param();
		set_or_change_analyze();
		url =getUrl();
		Add_or_Renew_DB();
		return url;
	}
	
	if(iRet == 0){
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

