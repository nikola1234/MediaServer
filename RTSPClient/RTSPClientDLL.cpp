// VNMPNETSDK.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "VideoDeviceSDK.h"
#include "ManageLogin.h"
#include "ConnectMC.h"
#include "OpenCamera.h"
#include "Log.h"
#include "UDP_RTSPClient.h"

#include "boost/filesystem.hpp"

CManageLogin g_ManageLogin;
CManageLogin::ConnectMCPtr g_connectMCptr_;
CLog g_MSNetSDKlog;
SCAMERALIST g_FullCamList;

void InitLog()
{
	boost::filesystem::create_directory("log");
	g_MSNetSDKlog.InitLog("./log/RTSPClient-");
}

VIDDEVSDK int __stdcall  video_device_init()
{
	if(g_connectMCptr_ == NULL) {
		InitLog();
		g_connectMCptr_ = g_ManageLogin.CreateConnectMC();
		g_MSNetSDKlog.Add("video_device_init g_connectMCptr_ == NULL");
	}
	else
	{
		g_connectMCptr_->StopAllCamera();
		g_MSNetSDKlog.Add("video_device_init g_connectMCptr_ != NULL");
	}
	memset(&g_FullCamList, 0, sizeof(SCAMERALIST));
	return 0;
}

VIDDEVSDK int __stdcall  video_device_destroy()
{
	g_MSNetSDKlog.Add("video_device_destroy");
	if(g_connectMCptr_ != NULL){
		g_connectMCptr_->StopAllCamera();
		CManageLogin::ConnectMCPtr pNULL;
		g_connectMCptr_ = pNULL;
	}
	//CUDP_RTSPClient::s_udpportvector.StopKeepLiveThread();
	return 0;
}

VIDDEVSDK int __stdcall video_device_login(VIDEODEVICE *videodev)
{
	if(g_connectMCptr_ != NULL){
		g_connectMCptr_->m_clientType = videodev->reuse;
	}
	return 0;
}

VIDDEVSDK int __stdcall video_device_logout(VIDEODEVICE *videodev)
{
	return 0;
}

typedef struct tagConnectMS{
int MediaID;
void* callback_fun;
void* user;
bool isStartEx;
int nret;
}ConnectMSST;

unsigned __stdcall ConnectMSThread(void * param)
{
	try{
		ConnectMSST* pConnectMCInfo = (ConnectMSST*)param;
		CConnectMC::OpenCameraPtr openCameraPtr_
			= g_connectMCptr_->FindOpenCameraByMediaID( pConnectMCInfo->MediaID );
		if( openCameraPtr_ != NULL )
			pConnectMCInfo->nret = openCameraPtr_->StartCamera(pConnectMCInfo->callback_fun,
			pConnectMCInfo->user, 
			pConnectMCInfo->isStartEx);
	}catch(...){}
	return 0;
}

int start_capturing(VIDEODEVICE *videodev, void* user, void* callback_fun, bool isStartEx)
{
	if(g_connectMCptr_ == NULL) {
		InitLog();
		g_connectMCptr_ = g_ManageLogin.CreateConnectMC();
	}
	g_MSNetSDKlog.Add("start_capturing begin rtsp_url = %s user = %d", videodev->channel, user);

	CManageLogin::ConnectMCPtr connectMCptr_ = g_connectMCptr_;
	int inewCameraid = COpenCamera::GetCameraIDFromURL(videodev->channel);
	if( inewCameraid < 0 )return ERROR_OPEN_CAMERA;

	CConnectMC::OpenCameraPtr openCameraPtr_
		= connectMCptr_->FindOpenCameraByCameraID(inewCameraid);

	if( openCameraPtr_ != NULL){
		openCameraPtr_->StopCamera();
		connectMCptr_->RemoveOpenCameraByMediaID(openCameraPtr_->GetMediaID());
		openCameraPtr_.reset();
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}

	CConnectMC::OpenCameraPtr oldOpenCameraPtr_ = connectMCptr_->FindOpenCameraByUser( ( int )user );
	if(oldOpenCameraPtr_ != NULL){
		g_MSNetSDKlog.Add("SwitchCamera need to Close oldCameraId = %d", oldOpenCameraPtr_->GetCameraID());
		oldOpenCameraPtr_->StopCamera();
		connectMCptr_->RemoveOpenCameraByMediaID(oldOpenCameraPtr_->GetMediaID());
	}

	openCameraPtr_ = connectMCptr_->CreateOpenCamera();

	std::string strURLall = videodev->channel;
	std::string strURLProtocol = strURLall.substr(0, 5);
	if( strURLProtocol.compare("onvif") == 0 ) {
		std::string strRTSPUrl;
		int iret = openCameraPtr_->GetRTSPUrlOnvif(videodev->channel, strRTSPUrl);
		if( iret < 0 ){
			connectMCptr_->RemoveOpenCameraByMediaID(openCameraPtr_->GetMediaID());
			return ERROR_OPEN_CAMERA;
		} else {
			strcpy(videodev->channel, strRTSPUrl.c_str());
		}
	}

	int transport_type = 2;
	if( videodev->reuse >= VNMP_TRANSPORT_UDP && videodev->reuse <= VNMP_TRANSPORT_RAW )
		transport_type = (videodev->reuse % VNMP_TRANSPORT_UDP);
	openCameraPtr_->InitOpenCamera(videodev->channel, transport_type);

#ifdef WIN32
	ConnectMSST connectMSST;
	connectMSST.MediaID = openCameraPtr_->GetMediaID();
	connectMSST.callback_fun = callback_fun;
	connectMSST.user = user;
	connectMSST.isStartEx = isStartEx;
	connectMSST.nret = -1;
	HANDLE hServerHandle = (HANDLE)_beginthreadex(NULL, 0, 
		ConnectMSThread, (LPVOID)&connectMSST, 0, NULL);
	MSG  msg;
	while(WAIT_TIMEOUT == WaitForSingleObject(hServerHandle,1)){	
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	CloseHandle( hServerHandle );
	int nret = connectMSST.nret;
#else
	int nret = openCameraPtr_->StartCamera(callback_fun, user, isStartEx);
#endif	

	g_MSNetSDKlog.Add("start_capturing end rtsp_url = %s user = %d nret = %d", videodev->channel, user, nret);
	memset(videodev->reserve, 0, 128);
	if ( nret >= 0 )
	{
		videodev->devnum = openCameraPtr_->GetUser();
		memset(videodev->channel, 0, sizeof(videodev->channel));
		sprintf(videodev->channel, "%d", openCameraPtr_->GetCameraID());
		strcpy(videodev->reserve, openCameraPtr_->GetCameraName().c_str());
		return videodev->devnum;
	}
	else
	{
		connectMCptr_->RemoveOpenCameraByMediaID(openCameraPtr_->GetMediaID());

		char chErrorInfo[256];memset(chErrorInfo, 0, 256);
		if( strlen(openCameraPtr_->GetOpenErrorInfo()) > 0 ){
			strcpy(chErrorInfo, openCameraPtr_->GetOpenErrorInfo());
			chErrorInfo[127] = chErrorInfo[126] = 0;
			strcpy(videodev->reserve, chErrorInfo);
		}

		//方便外部关闭摄像机--内部关闭
		videodev->devnum = openCameraPtr_->GetUser();
		video_device_capturing_stop(videodev);
		//////////////////////////////////////////////////

		if( nret <= -50 && nret >= -60 )
			return nret; //与服务器网络错误
		return ERROR_OPEN_CAMERA;
	}
	return 0;
}

VIDDEVSDK int __stdcall video_device_capturing_start(VIDEODEVICE *videodev,
	unsigned long user, VIDEO_CAPTURE_CALLBACK capture_callback)
{
	return start_capturing(videodev, (void *)user, (void *)capture_callback, false);
}

VIDDEVSDK int __stdcall video_device_capturing_startEx(VIDEODEVICE *videodev,
	void* user, VIDEO_CAPTURE_CALLBACKEx capture_callback)
{
	return start_capturing(videodev,user,(void *)capture_callback,true);
}

VIDDEVSDK int __stdcall video_device_capturing_stop(VIDEODEVICE *videodev)
{
	if(g_connectMCptr_ == NULL) return 0;
	CManageLogin::ConnectMCPtr connectMCptr_ = g_connectMCptr_;

	g_MSNetSDKlog.Add("video_device_capturing_stop videodev->devnum = %d ", videodev->devnum);

	CConnectMC::OpenCameraPtr openCameraPtr_
		= connectMCptr_->FindOpenCameraByUser(videodev->devnum);
	if( openCameraPtr_ != NULL){
		openCameraPtr_->StopCamera();
		connectMCptr_->RemoveOpenCameraByMediaID(openCameraPtr_->GetMediaID());
	}
	return 0;
}

VIDDEVSDK int __stdcall video_device_cameracontrol(VIDEODEVICE *videodev, int ControlType, int ControlSpeed)
{
	if(g_connectMCptr_ == NULL) return 0;
	CManageLogin::ConnectMCPtr connectMCptr_ = g_connectMCptr_;
	CConnectMC::OpenCameraPtr openCameraPtr_
		= connectMCptr_->FindOpenCameraByUser(videodev->devnum);
	if( openCameraPtr_ != NULL){
		openCameraPtr_->CameraPTZCtrl(ControlType, ControlSpeed, 0);
		g_MSNetSDKlog.Add("video_device_cameracontrol videodev->devnum=%d ControlType=%d ControlSpeed=%d ",
			videodev->devnum, ControlType, ControlSpeed);
	}
	return 0;
}

VIDDEVSDK void __stdcall video_device_set_camera_switch_update(VIDEODEVICE *videodev,
	VIDEO_SWITCH_CAMERA_UPDATE_CALLBACK
	camera_switch_update_callback,
	unsigned long user)
{
	CRTSPClient::s_UpdateCameraNameCallBack = camera_switch_update_callback;
	CRTSPClient::s_UpdateCameraCallBackUser = user;
}

