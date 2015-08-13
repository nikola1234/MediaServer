
#include "NetClientSession.h"
#include "NetServer.h"
#include "FileOpr.h"
#include "RtspCamera.h"
#include "CameraManage.h"

T_ServerParam SerParam;
NetServer NetWorkServer;
CFileOpr fileOpr;
CRtspCamera rtspCamera;
ManageCamera ManCamera;

int main(int argc ,char **argv)
{

  fileOpr.read_server_config(SerParam);

  NetWorkServer.InitNetServer(SerParam.port);
  NetWorkServer.Start();
  NetWorkServer.Run();

  ManCamera.AddServer(&NetWorkServer);
  ManCamera.InitFromDB();

  NetWorkServer.AddManaCamera(&ManCamera);

  StartRTSPServer(&rtspCamera);

  cout<<"everthing is already"<<endl;
  while(1){
	    	sleep(2);
  }

  return 0;
}
