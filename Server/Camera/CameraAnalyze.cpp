#include "CameraAnalyze.h"

CamAnalyze::CamAnalyze(uint32 ID)
{
	CameraID = ID;
}

CamAnalyze::~CamAnalyze()
{

}

int CamAnalyze::start_all_thread(T_CAM_PARAM *pt_cam_param);
{
	
}

void CamAnalyze::StopAnalyze()
{
	TimeThread->pause();
	Ana1Thread->pause();
	Ana2Thread->pause();
}


