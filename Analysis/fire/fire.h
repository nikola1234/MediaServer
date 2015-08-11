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
  int SetRectangle(vector< Rect > & rectangle);
  int FireDetectRun(Mat& displayFrame,void* videoHandler);
};

#endif
