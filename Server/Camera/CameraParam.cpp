#include "CameraParam.h"
#include "FileOpr.h"

extern T_ServerParam SerParam;

CamParam::CamParam()
{
  	CameraID = 0;
}

CamParam::~CamParam()
{

}
string CamParam::get_camera_rtsp_url()
{
	string url;
	url = RtspUrl;
	return url;
}

void CamParam::release()
{
	CameraID = 0;
	memset(CameraIP,0,IP_LEN_16);
	frameRate = 0;
	memset(CamUrl,0,SINGLE_URL_LEN_128);
	memset(RtspUrl,0,SINGLE_URL_LEN_128);
	CameraFunc = 0;
	AnalyzeNUM = 0;
	AnalyzeType = 0;
	AnalyzeType1 = 0;
	AnalyzeType2 = 0;
}:

int CamParam::generate_url()
{
	char rtsp[128] ={0};
	uint32 port =558;
	if(strlen(SerParam.Serverip) == 0) return -1;
	sprintf(rtsp,"%s%s:%d/%d","rtsp://",SerParam.Serverip,port,CameraID);
	memcpy(RtspUrl,rtspL,SINGLE_URL_LEN_128);
	return 0;
}

uint16 check_analyzetype(uint16 &type)
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

int CamParam::parse_type(uint16 & type)
{
	if(type &HumanDetect)  {
		AnalyzeType1 = HumanDetect;
		AnalyzeType2=check_analyzetype(type&(uint16)(~HumanDetect));
		return 0;
	}
	if(type &SmokeDetect)  {
		AnalyzeType1 = SmokeDetect;
		AnalyzeType2=check_analyzetype(type&(uint16)(~SmokeDetect));
		return 0;
	}
	if(type &RegionDetect)  {
		AnalyzeType1 = RegionDetect;
		AnalyzeType2=check_analyzetype(type&(uint16)(~RegionDetect));
		return 0;
	}
	if(type &FixedObjDetect)  {
		AnalyzeType1 = FixedObjDetect;
		AnalyzeType2=check_analyzetype(type&(uint16)(~FixedObjDetect));
		return 0;
	}
	if(type &FireDetect)  {
		AnalyzeType1 = FireDetect;
		AnalyzeType2=check_analyzetype(type&(uint16)(~FireDetect));
		return 0;
	}

	dbgprint("%s(%d),parse type error,type is %x!\n", DEBUGARGS,type);
	return -1;
}

void CamParam::reset(ST_VDCS_VIDEO_PUSH_CAM & addCam,uint32 ID)
{
	CameraID =ID;
	memset(CameraIP,addCam.ip,IP_LEN_16);
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
}


