
#include "NetClientSession.h"
#include "NetServer.h"
#include "FileOpr.h"
#include "RtspCamera.h"
//#include "tmp/CamTmp.h"

//const string videoStreamAddress1 = "rtsp://admin:123456@192.168.1.252:554/mpeg4cif";
const string videoStreamAddress2 = "rtsp://admin:admin@192.168.1.169:554/cam/realmonitor?channel=1&subtype=1";

T_ServerParam SerParam;
NetServer server;
CFileOpr fileOpr;
CRtspCamera rtspCamera;

int main(int argc ,char **argv)
{

  fileOpr.read_server_config(SerParam);

  server.InitNetServer(SerParam.port);
  server.Start();
  server.Run();

  StartRTSPServer(&rtspCamera);
/*
  CamTmpThread cam(videoStreamAddress2,1);
  cam.GetCamParam();
  cam.CreateCamThread();
*/

  while(1){
	    	sleep(2);
  }

  return 0;
}
