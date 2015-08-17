#ifndef _FIRE_H_
#define _FIRE_H_

#include "Common.h"
#include "VideoHandler.h"
#include "FlameDetector.h"

class  CFire
{

public:
	CFire(uint8 index,VideoHandler** videoHandler);
	~CFire();

	uint8 	m_index;
	VideoHandler *handler ;

public:

	uint8 alarm;
	vector<Rect > Rects;
	vector<Rect > FlameRect;

	void sleep_release();
	void pause_release();

	int set_rectangle(vector <Rect> rect);
	int FireDetectRun(Mat& displayFrame,void* videoHandler);
};

#endif
