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

	VideoCapture  	m_vcap;
	int 	 	m_rows; 	 /* high */
	int 	 	m_cols; 	  /* width */
	int	  	m_fps;
	AVFormatContext *fmtctx;
	AVCodec * codec;
	AVCodecContext *c;
	AVStream *video_st;

	AVPacket pkt;
	Mat rgb_frame;

	AVFrame *m_pRGBFrame ;  //RGB帧数据
	AVFrame *m_pYUVFrame ;  //YUV帧数据
	uint8   *pYUV_buffer;

	struct SwsContext *scxt;

	void releaseEncode();
	int EncodeInit();
	int GetCamParam();
	int Encode(Mat &frame);

};


#endif
