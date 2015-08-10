#include "fire.h"
#include "Analyze.h"

extern T_SINGLE_CAMERA t_camera[CAM_MAX_LEN] ;

CFire::CFire(uint8 index,VideoHandler** videoHandler)
{
	m_index =index;
	handler = new VideoHandler();
	*videoHandler  =  handler;
}


CFire::~CFire()
{
	delete handler;
	handler =NULL;
}

int CFire::FireAlarmRun(Mat& displayFrame,void* videoHandler)
{
  int  iRet = -1;
	alarm = 0;
	FlameRect.clear();
	for(unsigned int i = 0;i< t_camera[m_index].t_Camvarparam.t_CamFireAlarm.Rects.size();i++){
		Rect rt = t_camera[m_index].t_Camvarparam.t_CamFireAlarm.Rects[i];
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
