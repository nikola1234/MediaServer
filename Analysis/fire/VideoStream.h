#ifndef _VIDEOSTREAM_H_
#define _VIDEOSTREAM_H_

#include "Common.h"

class VideoStream
{
private:

	Mat ReadFrame;

	String m_str_url ;
	int videoWidth;
	int videoHeight;
	int m_i_frameFinished;

	uint8 videoStreamIndex;
	pthread_mutex_t   BuffMutex;
	AVPicture  pAVPicture;
	AVFormatContext *pAVFormatContext;
	AVCodecContext *pAVCodecContext;
	AVFrame *pAVFrame;
	SwsContext * pSwsContext;
	AVPacket pAVPacket;

public:

	VideoStream();
	~VideoStream();

	void setUrl(String url);
	void startStream();
	void stopStream();
	bool Init();
	void play();
	Mat & GetReadFrame() {return ReadFrame;}

};

#endif
