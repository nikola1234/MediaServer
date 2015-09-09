#ifndef _CAM_READ_THREAD_H_
#define _CAM_READ_THREAD_H_

#include "Common.h"
#include "CmdDefine.h"
#include "CamAnaThread.h"
#include "SingleCamera.h"
#include "NetServer.h"

class CamAnaThread;
class NetServer;
class NetClient;
class CamReadThread
{
public:
	CamReadThread(SingleCamera *sincam,NetServer *Server,NetClient*clt);
	~CamReadThread();

	Mat  anaframe;

	int SetCamera_StartThread(string url);

	int InitCamera();

	void run();
	int  CreateCamReadThread();
	static void* RunCamThread(void*  param){
		CamReadThread* p = (CamReadThread*)param;
		p->run();
		return NULL;
	}
	void quit(){m_CameraFlag = false;}
	void resume();
	void pause();

	pthread_mutex_t m_frameMut;
	
private:

	bool  m_CameraFlag;
	bool  m_Status;
	pthread_mutex_t mut;
	pthread_cond_t cond;


private:
	uint32 CameraID;
	string m_videoStream;
	
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

	uint32 errReadNum;
	void releaseEncode();
	int EncodeInit();
	int GetCamParam();
	int Encode(Mat &frame);
	void draw(CamAnaThread * Anathread,Mat &frame);
	void draw_encode_frame(Mat & frame);
	
	void report_camera_break();
	void check_camera_status();

	SingleCamera *cam;
	NetServer* server;
	NetClient* client;
};


#endif
