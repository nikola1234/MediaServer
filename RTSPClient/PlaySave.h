#pragma once

#include "OpenCamera.h"
#include "boost/atomic.hpp"

class CRTSPClient;
class CPlaySave :
	public COpenCamera
{

public:
	CPlaySave(void);
	~CPlaySave(void);

public:	
	int ChangePlaySpeed(float scale );
	int ChangePlayRangeClock(const char* Range_clock);

	int RePlayCommend();
	int PauseCommend();

	std::string GetCurPlayBackTime();

	virtual int RecvData(int timestamp, unsigned long dwDataType, unsigned char *data, unsigned long len);
};

