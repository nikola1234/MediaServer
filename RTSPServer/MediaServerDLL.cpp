#include "stdafx.h"
#include "MediaServerDLL.h"

#include "RTSPServer.h"
#include "PreAllocateDisk.h"
#include "ManageRecord.h"

live555RTSP::RTSPServer* g_pRTSPServer = NULL;
MediaSave::CPreAllocateDisk* g_pPreAllocateDisk = NULL;
MediaSave::CManageRecord* g_pManageRecord = NULL;

IDevice* g_pIDevice = NULL;
int StartMediaSave()
{
	char configValue[256]; memset(configValue, 0, 256);
	int i = g_pRTSPServer->GetDevInfo().GetDataInfoPtr()->GetConfig("MediaSave", configValue);
	if( strcmp(configValue,"on") != 0 )return 0;

	if(g_pPreAllocateDisk == NULL)
	{
		memset(configValue, 0, 256);
		i = g_pRTSPServer->GetDevInfo().GetDataInfoPtr()->GetConfig("VIDEOPATH", configValue);
		if( i < 0 )return ERROR_VIDEOPATH_CONFIG_FAILED;

		g_pPreAllocateDisk = new MediaSave::CPreAllocateDisk();
		if(!g_pPreAllocateDisk->InitStorageDirArray( configValue ))
			return ERROR_MEDIASAVE_INIT_FAILED;
		g_pRTSPServer->SetPreAllocateDisk(g_pPreAllocateDisk);

		i = g_pRTSPServer->GetDevInfo().GetDataInfoPtr()->GetConfig("PTZAutoBack", configValue);
		if( i >= 0 && strcmp(configValue,"on") == 0 ){ g_pPreAllocateDisk->SetIsPTZAutoBack(true); };

		g_pPreAllocateDisk->PrintPackageArray();
		g_pPreAllocateDisk->PrintFileIndecList();
	}
	g_pManageRecord = new MediaSave::CManageRecord(g_pPreAllocateDisk);
	int iret = g_pManageRecord->InitManageRecord( g_pRTSPServer->GetDevInfo().GetDataInfoPtr() );
	if( iret < 0 )return ERROR_LOADMRDB_FAILED;

	int RecordCMDPort = 0;
	i = g_pRTSPServer->GetDevInfo().GetDataInfoPtr()->GetConfig("RecordCMDPort", configValue);
	if( i >= 0 ) RecordCMDPort = atoi(configValue);
	iret = g_pManageRecord->StartRecordThread( RecordCMDPort );

	return iret;
}

int StartRTSP(ICamera* pICamera)
{
	if(g_pRTSPServer != NULL) return 0;
	g_pRTSPServer = new live555RTSP::RTSPServer();
	int i = g_pRTSPServer->InitRTSPServer(pICamera, "MS.sqlite");
	if( i < 0)return ERROR_LOADMSDB_FAILED;

	if( g_pIDevice != NULL )
		g_pRTSPServer->SetIDeviceInRTSPServer( g_pIDevice );

	char configValue[256]; memset(configValue, 0, 256);
	i = g_pRTSPServer->GetDevInfo().GetDataInfoPtr()->GetConfig("PORT_TOClient", configValue);
	if( i < 0 )return ERROR_RTSPPORT_CONFIG_FAILED;
	int clientPort = atoi(configValue); 
	if( clientPort <= 0 || clientPort > 65535)return ERROR_RTSPPORT_CONFIG_FAILED;

	i = g_pRTSPServer->Start(clientPort, 10);
	if( i < 0)return ERROR_STARTLISTEN_RTSPPORT_FAILED;
	g_pRTSPServer->Run();

	if( g_pRTSPServer->GetDevInfo().GetDBType() >= DB_POSTGRE )
	{
		memset(configValue, 0, 256);
		i = g_pRTSPServer->GetDevInfo().GetDataInfoPtr()->GetConfig("ConnectDBStr", configValue);
		if( i >= 0 ) g_pRTSPServer->GetDevInfo().GetDataInfoPtr()->Open(configValue);
		return 0;
	}

	return StartMediaSave();
}

MSDLL int __stdcall StartRTSPServer(ICamera* pICamera)
{
	int i = StartRTSP( pICamera );
	if( i >= 0 )return i;
	switch( i )
	{
	case ERROR_LOADMSDB_FAILED:
		g_pRTSPServer->m_log.Add("¼ÓÔØMSÊý¾Ý¿âÊ§°Ü");
		break;
	case ERROR_RTSPPORT_CONFIG_FAILED:
		g_pRTSPServer->m_log.Add("ÅäÖÃrtsp¶Ë¿Ú ´íÎó");
		break;
	case ERROR_STARTLISTEN_RTSPPORT_FAILED:
		g_pRTSPServer->m_log.Add("¼àÌýrtsp¶Ë¿ÚÊ§°Ü");
		break;
	case ERROR_VIDEOPATH_CONFIG_FAILED:
		g_pRTSPServer->m_log.Add("Â¼ÏñÂ·¾¶ ÉèÖÃ´íÎó");
		break;
	case ERROR_MEDIASAVE_INIT_FAILED:
		g_pRTSPServer->m_log.Add("Â¼ÏñÄ£¿éÆô¶¯Ê§°Ü");
		break;
	case ERROR_LOADMRDB_FAILED:
		g_pRTSPServer->m_log.Add("¼ÓÔØ´æ´¢Êý¾Ý¿âÊ§°Ü");
		break;
	default:
		g_pRTSPServer->m_log.Add( "Î´Öª´íÎó error = %d", i );
		break;
	}
	return i;
}

MSDLL void __stdcall StopRTSPServer()
{
	if (g_pManageRecord != NULL){
		g_pManageRecord->StopRecordThread();
		delete g_pManageRecord;
		g_pManageRecord = NULL;
	}

	if(g_pRTSPServer != NULL){
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		g_pRTSPServer->Stop();
		delete g_pRTSPServer;
		g_pRTSPServer = NULL;
	}
}

MSDLL int  __stdcall SendCameraError(int user,const char* chErrorInfo)
{
	if(g_pRTSPServer == NULL) return -1;
	g_pRTSPServer->SendMSGUpdateCameraName(user, 1, chErrorInfo);
	return 0;
}

MSDLL int  __stdcall UpdateSwitchCameraInfo(int oldCameraID, int iNewCameraID)
{
	if(g_pRTSPServer == NULL) return -1;
	return g_pRTSPServer->SendUpdateSwitchCameraInfo(oldCameraID, iNewCameraID);
}

MSDLL void __stdcall SetIDevice(IDevice* pIDevice)
{
	g_pIDevice = pIDevice;
	if( g_pRTSPServer != NULL )
		g_pRTSPServer->SetIDeviceInRTSPServer( pIDevice );
}
