#include "CameraList.h"

CamList::CamList()
{

}

CamList::~CamList()
{
	camlist_.clear();
}

void CamList::Get_All_Camera()
{
	CamreadLock readlock_(m_CamListMutex_);
	std::list<T_CAM_LIST>::iterator it = camlist_.begin();
	for ( ; it != camlist_.end() ; it++ )
	{
		printf("Camlist ID is %d\n", (int)(*it).CameraID);
	}
}

int CamList::add_cam_list(char * url,uint32 ID)
{
	T_CAM_LIST tmp;
	CamwriteLock writelock_(m_CamListMutex_);
	tmp.CameraID = ID;
	memcpy(tmp.url ,url,SINGLE_URL_LEN_128);
	camlist_.push_back(tmp);
	return 0;
}

int CamList::search_cam_by_url(char *url,int ID)
{
	CamreadLock readlock_(m_CamListMutex_);

	std::list<T_CAM_LIST>::iterator it = camlist_.begin();
	for ( ; it != camlist_.end() ; it++ )
	{
		if(0 ==  memcmp((*it).url,url,SINGLE_URL_LEN_128))
		{
			ID = (int)(*it).CameraID;
			return 0;
		}
	}
	return -1;
}

int CamList::delete_cam_by_id(uint32 ID)
{
	CamreadLock writelock_(m_CamListMutex_);

	std::list<T_CAM_LIST>::iterator it = camlist_.begin();
	for ( ; it != camlist_.end() ; it++ )
	{
		if((*it).CameraID == ID)
		{
			camlist_.erase(it);
			return 0;
		}
	}
	return -1;
}
