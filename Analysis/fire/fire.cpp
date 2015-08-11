#include "fire.h"

CFire::CFire(uint8 index,VideoHandler** videoHandler)
{
	m_index =index;
	handler = new VideoHandler();
	*videoHandler  =  handler;
	Rects.clear();
}

CFire::~CFire()
{
	delete handler;
	handler =NULL;
	Rects.clear();
}

int CFire::SetRectangle(vector< Rect > & rectangle)
{
	uint16 i = 0;
	Rects.clear();
	if(0 == rectangle.size())
	{
  	cout<<"camera "<<m_index<<" CFire::SetRectangle size is 0"<<endl;
		return -1;
	}

	for(i = 0; i < rectangle.size();i++)
	{
		Rect  tmp;
		tmp = rectangle[i];
		Rects.push_back(tmp);
	}
	return 0;
}

int CFire::FireDetectRun(Mat& displayFrame,void* videoHandler)
{
  int  iRet = -1;
	alarm = 0;
	FlameRect.clear();
	for(unsigned int i = 0;i< Rects.size();i++){
		Rect rt = Rects[i];
		iRet =  (int)handler->handle(displayFrame ,rt, videoHandler);
		if(iRet  == 0)
		{
			FlameRect.clear();
			vector<Rect>  & tmpRect =handler->getDetector().getDecider().alarmRect;
			for(uint16 k=0 ; k <tmpRect.size();k++)
      {
				Rect  tmp;
				tmp = tmpRect[k];
				FlameRect.push_back(tmp);
			}

			for(uint16 j = 0; j< FlameRect.size(); j++)
      {
				FlameRect[j].x   =FlameRect[j].x +rt .x;
				FlameRect[j].y   =FlameRect[j].y +rt .y;
			}
			alarm = 1;
		}
		if(iRet == 2) alarm = 0;
	}

	return 0;
}
