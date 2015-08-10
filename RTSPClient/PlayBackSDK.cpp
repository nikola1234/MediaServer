#include "stdafx.h"
#include "VideoDeviceSDK.h"
#include "ManageLogin.h"
#include "ConnectMC.h"
#include "OpenCamera.h"
#include "PlaySave.h"

#ifndef WIN32
typedef unsigned long       DWORD;
typedef unsigned short      WORD;
#endif

#include "Log.h"
extern CLog g_MSNetSDKlog;

extern CManageLogin::ConnectMCPtr g_connectMCptr_;

VIDDEVSDK int __stdcall video_device_FindPlayBackByTime(
	VIDEODEVICE *videodev, 
	IN TIME_SLICE* pRecordTimeSlice,
	char* pPlayBackInfo,       
	unsigned long PlayBackInfoSize)
{
	CManageLogin::ConnectMCPtr connectMCptr_ = g_connectMCptr_;

	char time_sl[256]; memset(time_sl, 0, 256);
	sprintf(time_sl , "%4d%02d%02dT%02d%02d%02dZ-%4d%02d%02dT%02d%02d%02dZ",
		pRecordTimeSlice->sBeginTime.dwYear, pRecordTimeSlice->sBeginTime.dwMonth,
		pRecordTimeSlice->sBeginTime.dwDay, pRecordTimeSlice->sBeginTime.dwHour,
		pRecordTimeSlice->sBeginTime.dwMinute,pRecordTimeSlice->sBeginTime.dwSecond,
		pRecordTimeSlice->sEndTime.dwYear, pRecordTimeSlice->sEndTime.dwMonth,
		pRecordTimeSlice->sEndTime.dwDay, pRecordTimeSlice->sEndTime.dwHour,
		pRecordTimeSlice->sEndTime.dwMinute, pRecordTimeSlice->sEndTime.dwSecond);

	char resultStr[65536]; memset(resultStr, 0, 65536);
	COpenCamera openCamera_;
	openCamera_.InitOpenCamera(videodev->channel, 2);
	openCamera_.GetSaveFileInfo(time_sl, resultStr);

	PlayBackInfoSize = strlen(resultStr);
	memcpy(pPlayBackInfo, resultStr, PlayBackInfoSize);

	return 0;
}

VIDDEVSDK int __stdcall video_device_PlayBackByTime(
	VIDEODEVICE *videodev, 
	IN TIME_SLICE* pRecordTimeSlice, 
	VIDEO_PLAYBACK_CALLBACK playback,    
	unsigned long dwUser)
{
	CManageLogin::ConnectMCPtr connectMCptr_ = g_connectMCptr_;

	CConnectMC::OpenCameraPtr oldOpenCameraPtr_ = connectMCptr_->FindOpenCameraByUser( dwUser );
	if( oldOpenCameraPtr_ != NULL ) {
		oldOpenCameraPtr_->StopCamera();
		connectMCptr_->RemoveOpenCameraByMediaID(oldOpenCameraPtr_->GetMediaID());
		g_MSNetSDKlog.Add("video_device_PlayBackByTime dwUser=%d", dwUser);
	}

	CConnectMC::OpenCameraPtr openCameraPtr_
		= connectMCptr_->CreatePlaySave();

	g_MSNetSDKlog.Add("video_device_DownloadByTime url=%s MediaID = %d", videodev->channel, openCameraPtr_->GetMediaID());

	char time_sl[256]; memset(time_sl, 0, 256);
	sprintf(time_sl , "%4d%02d%02dT%02d%02d%02dZ-%4d%02d%02dT%02d%02d%02dZ",
		pRecordTimeSlice->sBeginTime.dwYear, pRecordTimeSlice->sBeginTime.dwMonth,
		pRecordTimeSlice->sBeginTime.dwDay, pRecordTimeSlice->sBeginTime.dwHour,
		pRecordTimeSlice->sBeginTime.dwMinute,pRecordTimeSlice->sBeginTime.dwSecond,
		pRecordTimeSlice->sEndTime.dwYear, pRecordTimeSlice->sEndTime.dwMonth,
		pRecordTimeSlice->sEndTime.dwDay, pRecordTimeSlice->sEndTime.dwHour,
		pRecordTimeSlice->sEndTime.dwMinute, pRecordTimeSlice->sEndTime.dwSecond);

	int transport_type = 2;
	if( videodev->reuse >= VNMP_TRANSPORT_UDP && videodev->reuse <= VNMP_TRANSPORT_RAW )
		transport_type = (videodev->reuse % VNMP_TRANSPORT_UDP);
	openCameraPtr_->InitOpenCamera(videodev->channel, transport_type);

	int nret = openCameraPtr_->StartCamera((void *)playback, (void *)dwUser, false, time_sl, 1.0);

	if ( nret >= 0 ){

		memset(videodev->channel, 0, sizeof(videodev->channel));
		sprintf(videodev->channel, "%d", openCameraPtr_->GetCameraID());
		strcpy(videodev->reserve, openCameraPtr_->GetCameraName().c_str());

		videodev->devnum = openCameraPtr_->GetMediaID();
		return videodev->devnum;
	}else{
		connectMCptr_->RemoveOpenCameraByMediaID( openCameraPtr_->GetMediaID() );
		return nret;
	}
	return 0;
}

VIDDEVSDK int __stdcall video_device_PlayBackControl(
	VIDEODEVICE *videodev,
	unsigned long dwControlCode,
	unsigned long dwParam )
{
	CManageLogin::ConnectMCPtr connectMCptr_ = g_connectMCptr_;

	if( dwControlCode == PLAY_BACK_FAST || dwControlCode == PLAY_BACK_SLOW  )
		if( dwParam > 8 || dwParam < 1 )
			return ERROR_PARAMETER_INVALID;

	CConnectMC::OpenCameraPtr openCameraPtr_
		= connectMCptr_->FindOpenCameraByMediaID(videodev->devnum);
	if( openCameraPtr_ != NULL ){

		CPlaySave* pPlaySave = (CPlaySave *)openCameraPtr_.get();
		if(dwControlCode == PLAY_BACK_SKIPTIME)
			return pPlaySave->ChangePlayRangeClock( videodev->reserve );
		if(dwControlCode == PLAY_BACK_FAST )
			return pPlaySave->ChangePlaySpeed( (float)dwParam );
		if(dwControlCode == PLAY_BACK_SLOW)
			return pPlaySave->ChangePlaySpeed( 1.0/(float)dwParam );
		if(dwControlCode == PLAY_BACK_PAUSE)
			return pPlaySave->PauseCommend();
		if(dwControlCode == PLAY_BACK_RESTART)
			return pPlaySave->RePlayCommend();
		if(dwControlCode == PLAY_BACK_GETTIME)
		{
			std::string strRet = pPlaySave->GetCurPlayBackTime();
			strcpy( videodev->reserve, strRet.c_str() );
			if( strRet.size() > 0 )
				return ERROR_PROCESS_SUCCESS;
			else
				return ERROR_SYSTEM_ERROR;
		}

		return ERROR_PARAMETER_INVALID;
	} else { return ERROR_CAMERA_NOOPEN; }
	return 0;
}

VIDDEVSDK int __stdcall video_device_StopPlayBack(VIDEODEVICE *videodev)
{	
	g_MSNetSDKlog.Add("video_device_StopPlayBack devnum=%d", videodev->devnum);

	CManageLogin::ConnectMCPtr connectMCptr_ = g_connectMCptr_;
	if( connectMCptr_ == NULL )return NULL;
	CConnectMC::OpenCameraPtr openCameraPtr_
		= connectMCptr_->FindOpenCameraByMediaID(videodev->devnum);
	if( openCameraPtr_ != NULL){
		openCameraPtr_->StopCamera();
		connectMCptr_->RemoveOpenCameraByMediaID(videodev->devnum);
	}
	return 0;
}

VIDDEVSDK int __stdcall video_device_DownloadByTime(
	VIDEODEVICE *videodev, 
	IN TIME_SLICE* pRecordTimeSlice,
	VIDEO_PLAYBACK_CALLBACK playback,       
	unsigned long dwUser)
{

	CManageLogin::ConnectMCPtr connectMCptr_ = g_connectMCptr_;
	CConnectMC::OpenCameraPtr openCameraPtr_
		= connectMCptr_->CreatePlaySave();

	g_MSNetSDKlog.Add("video_device_DownloadByTime url=%s MediaID = %d", videodev->channel, openCameraPtr_->GetMediaID());

	char time_sl[256]; memset(time_sl, 0, 256);
	sprintf(time_sl , "%4d%02d%02dT%02d%02d%02dZ-%4d%02d%02dT%02d%02d%02dZ",
		pRecordTimeSlice->sBeginTime.dwYear, pRecordTimeSlice->sBeginTime.dwMonth,
		pRecordTimeSlice->sBeginTime.dwDay, pRecordTimeSlice->sBeginTime.dwHour,
		pRecordTimeSlice->sBeginTime.dwMinute,pRecordTimeSlice->sBeginTime.dwSecond,
		pRecordTimeSlice->sEndTime.dwYear, pRecordTimeSlice->sEndTime.dwMonth,
		pRecordTimeSlice->sEndTime.dwDay, pRecordTimeSlice->sEndTime.dwHour,
		pRecordTimeSlice->sEndTime.dwMinute, pRecordTimeSlice->sEndTime.dwSecond);

	openCameraPtr_->InitOpenCamera(videodev->channel, 2);
	int nret = openCameraPtr_->StartCamera((void *)playback, (void *)dwUser, false, time_sl, 0.0);

	if ( nret >= 0 ){
		videodev->devnum = openCameraPtr_->GetMediaID();
		return videodev->devnum;
	}else{
		connectMCptr_->RemoveOpenCameraByMediaID( openCameraPtr_->GetMediaID() );
		return nret;
	}
	return 0;
}

VIDDEVSDK int __stdcall video_device_StopDownload(VIDEODEVICE *videodev)
{
	g_MSNetSDKlog.Add("video_device_StopDownload url=%s user = %d", videodev->channel, videodev->devnum);

	CManageLogin::ConnectMCPtr connectMCptr_ = g_connectMCptr_;

	CConnectMC::OpenCameraPtr openCameraPtr_
		= connectMCptr_->FindOpenCameraByMediaID(videodev->devnum);
	if( openCameraPtr_ != NULL){
		openCameraPtr_->StopCamera();
		connectMCptr_->RemoveOpenCameraByMediaID(videodev->devnum);
	}
	return 0;
}
