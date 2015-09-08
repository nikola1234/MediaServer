#include "SingleCamera.h"
#include "FileOpr.h"

extern T_ServerParam SerParam;

SingleCamera::SingleCamera(ManageCamera*cam,uint32 ID)
{
	CameraID = ID;
	ManaCam = cam;
	ReadThread = new CamReadThread(this,ManaCam->Server);
	TimeThread = new CamTimeThread(this);
	Ana1Thread = new CamAnaThread(this,ManaCam->Server);
	Ana2Thread = new CamAnaThread(this,ManaCam->Server);

	AnalyzeType1 = 0;
	AnalyzeType2 = 0;
}

SingleCamera::~SingleCamera()
{
	delete ReadThread;
	ReadThread = NULL;
	delete TimeThread;
	TimeThread =NULL;
	delete Ana1Thread;
	Ana1Thread =NULL;
	delete Ana2Thread;
	Ana2Thread =NULL;
}

string  SingleCamera:: get_rtsp_url()
{
	string url =RtspUrl;
	return url;
}

int SingleCamera::generate_url()
{
	char rtsp[128] ={0};
	uint32 port =556;
	if(strlen(SerParam.ServerIp) == 0) return -1;
	memset(RtspUrl, 0 ,SINGLE_URL_LEN_128);
	sprintf(rtsp,"%s%s:%d/%d","rtsp://",SerParam.ServerIp,port,CameraID);
	memcpy(RtspUrl,rtsp,SINGLE_URL_LEN_128);
	printf("RtspUrl is %s\n",RtspUrl);
	return 0;
}

uint16 check_analyzetype(uint16 type)
{
	uint16 uType = 0;
	switch(type){
	case HumanDetect: 
		uType = HumanDetect;
		break;
	case SmokeDetect :
		uType = SmokeDetect;
		break;
	case RegionDetect :
		uType = RegionDetect;
		break;
	case FixedObjDetect:
		uType = FixedObjDetect;
		break;
	case FireDetect :
		uType = FireDetect;
		break;
	case ResidueDetect :
		uType = ResidueDetect;
		break;
	default: break;
	}
	if(uType == 0)	
		dbgprint("%s(%d),push camera one analyze but none,type is %x!\n", DEBUGARGS,type);
	return uType;
}

int SingleCamera::parse_type(uint16  type)
{
	if(type & HumanDetect)  {
		AnalyzeType1 = HumanDetect;
		AnalyzeType2= check_analyzetype(type &( (uint16)(~HumanDetect)));
		return 0;
	}
	if(type & SmokeDetect)  {
		AnalyzeType1 = SmokeDetect;
		AnalyzeType2=check_analyzetype(type & ((uint16)(~SmokeDetect)));
		return 0;
	}
	if(type & RegionDetect)  {
		AnalyzeType1 = RegionDetect;
		AnalyzeType2=check_analyzetype(type& ((uint16)(~RegionDetect)));
		return 0;
	}
	if(type & FixedObjDetect)  {
		AnalyzeType1 = FixedObjDetect;
		AnalyzeType2=check_analyzetype(type&((uint16)(~FixedObjDetect)));
		return 0;
	}
	if(type & FireDetect)  {
		AnalyzeType1 = FireDetect;
		AnalyzeType2=check_analyzetype(type& ((uint16)(~FireDetect)));
		return 0;
	}

	dbgprint("%s(%d),parse type error,type is %x!\n", DEBUGARGS,type);
	return -1;
}

int SingleCamera:: set_camera_fix_param(ST_VDCS_VIDEO_PUSH_CAM & addCam)
{
	printf("set_camera_fix_param \n");
	memcpy(CameraIP,addCam.ip,IP_LEN_16);
	memcpy(CamUrl,addCam.CameUrL,SINGLE_URL_LEN_128);
	
	frameRate 	= addCam.frameRate;
	CameraFunc 	= addCam.CameraFunc;
	AnalyzeNUM    = addCam.AnalyzeNUM;
	AnalyzeType   = addCam.AnalyzeType;
	generate_url();
	switch (AnalyzeNUM){
		case 1:
			AnalyzeType1 =check_analyzetype(AnalyzeType);
			if(AnalyzeType1 == 0) AnalyzeNUM = 0;
			break;
		case 2:
			parse_type(AnalyzeType);
			break;
		default :
			dbgprint("%s(%d),wrong AnalyzeNUM and AnalyzeNUM is %d!\n", DEBUGARGS,AnalyzeNUM);
		 	break;
	}
	//start thread
	string url =CamUrl;
	ReadThread->SetCamera_StartThread(url);
	TimeThread->SetCamera_StartThread(AnalyzeNUM);
	Ana1Thread->SetCamera_StartThread(AnalyzeType1,1);
	Ana2Thread->SetCamera_StartThread(AnalyzeType2,2);
	return 0;

}

void SingleCamera::reset_camera_fix_param(ST_VDCS_VIDEO_PUSH_CAM & addCam)
{
	TimeThread->pause();
	Ana1Thread->pause();
	Ana2Thread->pause();
	
	memset(CameraIP,0,IP_LEN_16);
	frameRate = 0;
	memset(CamUrl,0,SINGLE_URL_LEN_128);
	memset(RtspUrl,0,SINGLE_URL_LEN_128);
	CameraFunc = 0;
	AnalyzeNUM = 0;
	AnalyzeType = 0;
	AnalyzeType1 = 0;
	AnalyzeType2 = 0;

	memcpy(CameraIP,addCam.ip,IP_LEN_16);
	memcpy(CamUrl,addCam.CameUrL,SINGLE_URL_LEN_128);
	frameRate	= addCam.frameRate;
	CameraFunc	= addCam.CameraFunc;
	AnalyzeNUM	  = addCam.AnalyzeNUM;
	AnalyzeType   = addCam.AnalyzeType;
	generate_url();
	switch (AnalyzeNUM){
		case 1:
			AnalyzeType1 =check_analyzetype(AnalyzeType);
			if(AnalyzeType1 == 0) AnalyzeNUM = 0;
			break;
		case 2:
			parse_type(AnalyzeType);
			break;
		default :
			dbgprint("%s(%d),wrong AnalyzeNUM and AnalyzeNUM is %d!\n", DEBUGARGS,AnalyzeNUM);
			break;
	}

}

int SingleCamera::reset_camera_var_param_from_db(T_VDCS_VIDEO_CAMERA_PARAM* pt_CameraParam,vector <VIDEO_DRAW> & Pkg,int num)
{
	printf("reset_camera_var_param_from_db\n");
	if(num== 1)
	{	
		AnalyzeType1 =  pt_CameraParam->AnalyzeType;
		reset_camera_var_param(pt_CameraParam,Pkg);
		return 0;
	}
	
	if(num ==2)
	{
		AnalyzeType2 =  pt_CameraParam->AnalyzeType;
		reset_camera_var_param(pt_CameraParam,Pkg);
		return 0;
	}
	return -1;
}

int SingleCamera::reset_camera_var_param(T_VDCS_VIDEO_CAMERA_PARAM* pt_CameraParam,vector <VIDEO_DRAW> & Pkg)
{
	printf("reset_camera_var_param\n");
	printf("pt_CameraParam->AnalyzeType is %x\n", pt_CameraParam->AnalyzeType );
	if((pt_CameraParam->PkgNum != Pkg.size())||(Pkg.size() == 0))
	{
		dbgprint("%s(%d),reset_camera_var_param wrong PkgNum  is %d, pkg.size() is %d!\n", DEBUGARGS,pt_CameraParam->PkgNum,Pkg.size());
		return -1;
	}

	//ReadThread->set_video_draw(Pkg);
	if(pt_CameraParam->AnalyzeType == AnalyzeType1)
	{
		Ana1Thread->pause();
		TimeThread->reset_time1_param(pt_CameraParam->AlarmTime);
		Ana1Thread->AnaIndex = 1;
		Ana1Thread->AnalyzeType = pt_CameraParam->AnalyzeType;
		Ana1Thread->human->MaxNum = pt_CameraParam->MaxHumanNum;
		Ana1Thread->region->ChangRate = pt_CameraParam->ChangRate;
		Ana1Thread->set_video_draw(Pkg);
		Ana1Thread->resume();
		return 0;
	}
	if(pt_CameraParam->AnalyzeType == AnalyzeType2)
	{
		Ana2Thread->pause();
		TimeThread->reset_time2_param(pt_CameraParam->AlarmTime);
		Ana2Thread->AnaIndex = 2;
		Ana2Thread->AnalyzeType = pt_CameraParam->AnalyzeType;
		Ana2Thread->human->MaxNum = pt_CameraParam->MaxHumanNum;
		Ana2Thread->region->ChangRate = pt_CameraParam->ChangRate;
		Ana2Thread->set_video_draw(Pkg);
		Ana2Thread->resume();
		return 0;
	}

	dbgprint("%s(%d),reset_camera_var_param wrong AnalyzeType  is %d!\n", DEBUGARGS,pt_CameraParam->AnalyzeType);
	return -1;
}


