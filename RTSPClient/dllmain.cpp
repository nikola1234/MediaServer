// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ManageLogin.h"
#include "ConnectMC.h"
#include "Log.h"
#include "VideoDeviceSDK.h"

extern CManageLogin::ConnectMCPtr g_connectMCptr_;

#ifdef WIN32

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//m_log.Add("DllMain::DLL_PROCESS_ATTACH");
		break;
	case DLL_THREAD_ATTACH:
		//m_log.Add("DllMain::DLL_THREAD_ATTACH");
		break;
	case DLL_THREAD_DETACH:
		//m_log.Add("DllMain::DLL_THREAD_DETACH");
		break;
	case DLL_PROCESS_DETACH:
		VIDEODEVICE dev;
		video_device_set_camera_switch_update(&dev,NULL,-1);
		//exit(0);
		//m_log.Add("DllMain::DLL_PROCESS_DETACH");
		break;
	}
	return TRUE;
}

#endif
