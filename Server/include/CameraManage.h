#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Common.h"
#include "NetServer.h"
#include "SingleCamera.h"

class NetServer;
class SingleCamera;
class ManageCamera
{
public:  
	typedef boost::shared_ptr<SingleCamera> SingleCamPtr;	
	typedef boost::shared_lock<boost::shared_mutex> readLock;
	typedef boost::unique_lock<boost::shared_mutex> writeLock;

public:

  ManageCamera();
  ~ManageCamera();

  int generate_cam_num();

  void AddServer(NetServer *ourServer){Server = ourServer;}
  int InitFromDB();

  int try_to_open(string stream);

private:
	std::list<SingleCamPtr> m_SinCamList;
	boost::shared_mutex m_SinCamList;

	vector < uint32 > CameraIDVector;
	NetServer *Server;
	
};

#endif
