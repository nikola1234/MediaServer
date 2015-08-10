
#include "NetClientSession.h"
#include "NetServer.h"
#include "Camera.h"

const string videoStreamAddress1 = "rtsp://admin:123456@192.168.1.252:554/mpeg4cif";
const string videoStreamAddress2 = "rtsp://admin:admin@192.168.1.169:554/cam/realmonitor?channel=1&subtype=1";
int main()
{
  /*
  NetServer server;

  server.InitNetServer();
  server.Start();
  server.Run();
*/

  CamThread cam(videoStreamAddress1,1);
  cam.GetCamParam();
  cam.CreateCamThread();

  while(1){
    sleep(5);
  }

  return 0;
}
