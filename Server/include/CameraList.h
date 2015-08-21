#ifndef _CAMERA_LIST_H_
#define  _CAMERA_LIST_H_

#include "Common.h"
#include "Data.h"

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread.hpp>

class CamList
{
public:
	typedef boost::shared_lock<boost::shared_mutex> CamreadLock;
	typedef boost::unique_lock<boost::shared_mutex> CamwriteLock;
public:
	CamList();
	~CamList();

	void Get_All_Camera();
	int add_cam_list(char * url,uint32 ID);
	int search_cam_by_url(char *url,uint32* ID);
	int delete_cam_by_id(uint32 ID);
private:
	std::list<T_CAM_LIST> camlist_;  /* url and ID */

	boost::shared_mutex m_CamListMutex_;
};
#endif
