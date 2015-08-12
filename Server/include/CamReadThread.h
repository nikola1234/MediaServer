#ifndef _CAM_READ_THREAD_H_
#define _CAM_READ_THREAD_H_

#include "Common.h"

class CamReadThread
{
public:
  CamReadThread(string url,uint32 ID);
  ~CamReadThread();

  Mat   frame1;
  Mat   frame2;

  int InitCamera();

  void run();
	int  CreateCamReadThread();
	static void* RunCamThread(void*  param){
		CamReadThread* p = (CamReadThread*)param;
		p->run();
		return NULL;
	}
	void quit(){m_CameraFlag = false;}

private:
  uint32 CameraID;
  string m_videoStream;
  bool  m_CameraFlag;
  Mat ReadFrame;
  Mat EncodeFrame;

  void releaseEncode();
  int EncodeInit();
  int GetCamParam();
  int Encode(Mat &frame);

};


#endif
