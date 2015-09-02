
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

void string2char(string str, char*buf)
{
	memcpy(buf, str.c_str(),str.length());
}

int main(int argc ,char **argv)
{
	string a ;
	int num= 0;
	char buffer[64] ={0};

	fileOpr.read_server_config(SerParam);

	NetWorkServer.InitNetServer(SerParam.port);
	NetWorkServer.Start();
	NetWorkServer.Run();

	ManCamera.AddServer(&NetWorkServer);
	ManCamera.InitFromDB();

	NetWorkServer.AddManaCamera(&ManCamera);

	StartRTSPServer(&rtspCamera);

	cout<<"everthing is already"<<endl;
/*	
	while(1)  //²¿Êð×÷ÓÃ
	{
		sleep(5);
	}
*/

	while(cin>> a){ // ²âÊÔ
			memset(buffer, 0 ,64);
	    	string2char(a,buffer);
	    	num =  atoi(buffer);
	    	switch(num){
	    	case 1:
	    		NetWorkServer.GetClient();
	    		break;
	    	case 2:
	    		ManCamera.m_CamList.Get_All_Camera();
	    		break;
	    	case 3:
	    		ManCamera.Get_Rest_Camlist();
	    		break;
	    	case 4:
	    		ManCamera.Get_Rest_SingleCamlist();
	    		break;
			case 11:
				ManCamera.Server->SendBufferToMCUClient(600300001,1);
				break;
			case 12:
				ManCamera.Server->SendBufferToMCUClient(600300002,1);
				break;
			case 13:
				ManCamera.Server->SendBufferToMCUClient(600300003,1);
				break;
			case 14:
				ManCamera.Server->SendBufferToMCUClient(600300004,1);
				break;

			default :break;
	    	}
	}

	return 0;
}
