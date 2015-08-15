#ifndef _CAMERA_ANALYZE_H_
#define _CAMERA_ANALYZE_H_

#include "Common.h"
#include "Data.h"
#include "CamReadThread.h"
#include "CamTimeThread.h"
#include "CamAnaThread.h"

class CamAnalyze
{

public:
	CamAnalyze(uint32 ID);
	~CamAnalyze();


	int start_all_thread(T_CAM_PARAM *pt_cam_param); 

	void StopAnalyze();

private:
	uint32 CameraID;
	CamReadThread* ReadThread;
	CamTimeThread* TimeThread;
	CamAnaThread*	Ana1Thread;
	CamAnaThread*	Ana2Thread;

};


#endif
