
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
	while(cin>> a){
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
		default :break;
	    	}
	}

	return 0;
}
