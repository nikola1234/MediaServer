#include "CameraAnalyze.h"

CamAnalyze::CamAnalyze()
{
}

CamAnalyze::~CamAnalyze()
{

}

void CamAnalyze::StopAnalyze()
{
	TimeThread->pause();
	Ana1Thread->pause();
	Ana2Thread->pause();
}


