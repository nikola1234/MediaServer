#ifndef _CAMERA_LIST_H_
#define  _CAMERA_LIST_H_

#include "Common.h"
#include "Data.h"

class CamList
{
public:
	typedef boost::shared_lock<boost::shared_mutex> CamreadLock;
	typedef boost::unique_lock<boost::shared_mutex> CamwriteLock;
public:
	CamList();
	~CamList();

	
	int add_cam_list(char * &url,uint32 ID);
	int search_cam_by_url(char *url,int & ID);
private:
	std::list<T_CAM_LIST> camlist_;  /* url and ID */
	
	boost::shared_mutex m_CamListMutex_;
};
#endif