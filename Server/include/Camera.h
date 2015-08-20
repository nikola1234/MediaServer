#ifndef _CAM_THREAD_H_
#define _CAM_THREAD_H_

#include "Common.h"
#include "MediaServerDLL.h"
#include "ICamera.h"

class CAnaCamera : public ICamera
{
public:
	CAnaCamera(){ m_capture_callback = NULL; };
	virtual ~CAnaCamera(){ };

	virtual int openCamera(int CameraID, int ChannelID, int user, VIDEO_CAPTURE_CALLBACKEX capture_callback, RESULT_PARAM_S *resultParam)
	{
		if( m_capture_callback == NULL ) m_capture_callback = capture_callback;
		return 0;
	};

	virtual int closeCamera(int CameraID) { return 0; };
	virtual int switchCamera(int CameraID, int oldCameraID, int ChannelID, int user, VIDEO_CAPTURE_CALLBACKEX capture_callback) { return -1; };
	virtual int controlCamera(int CameraID, int ChannelID, int ControlType, int ControlSpeed) { return -1; };

	VIDEO_CAPTURE_CALLBACKEX m_capture_callback;
};

class CamThread
{
public:
	CamThread(string url,uint8 index);
	~CamThread();

	Mat   ReadFrame;
  Mat   EncodeFrame;

	VideoCapture  	m_vcap;
	string m_videoStream;
	uint8  m_index;
	bool  m_CameraFlag;

	void releaseEncode();
	int GetCamParam();
	int EncodeInit();
  int Encode(Mat &frame);

	void run();
	int  CreateCamThread();
	static void* RunCamThread(void*  param){
		CamThread* p = (CamThread*)param;
		p->run();
		return NULL;
	}

	void quit()
	{
		m_CameraFlag = false;
	}

private:
	int 	 	m_rows; 	 /* high */
	int 	 	m_cols; 	  /* width */
	int	  	m_fps;


	CAnaCamera AnalyzeCamera;

private:
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

};

#endif
