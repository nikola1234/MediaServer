#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Common.h"
#include "CameraAnalyze.h"
#include "CameraParam.h"
#include "NetServer.h"

class NetServer;
class ManageCamera
{
public:

  ManageCamera();
  ~ManageCamera();

  int generate_cam_num();

  void AddServer(NetServer *ourServer){Server = ourServer;}
  int InitFromDB();

  int try_to_open(string stream);

private:

  vector < uint32 > CameraIDVector;
  NetServer *Server;
};

#endif
