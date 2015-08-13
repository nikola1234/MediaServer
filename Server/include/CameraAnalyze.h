#ifndef _CAMERA_ANALYZE_H_
#define _CAMERA_ANALYZE_H_

#include "Common.h"

#include "CamReadThread.h"
#include "CamTimeThread.h"
#include "CamAnaThread.h"

class CamAnalyze
{

public:
	CamAnalyze();
	~CamAnalyze();


private:
	uint32 CameraID;
	CamReadThread* ReadThread;
	CamTimeThread* TimeThread;
	CamAnaThread*	Ana1Thread;
	CamAnaThread*	Ana2Thread;

};


#endif
