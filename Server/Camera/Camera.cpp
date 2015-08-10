#include "Camera.h"
#include "MyLog.h"

CamThread::CamThread(string url,uint8 index)
	:m_videoStream(url),m_index(index)
{
	m_CameraFlag = true;
}

CamThread::~CamThread()
{
	m_CameraFlag = false;
}

void CamThread::releaseEncode()
{
	avcodec_close(c);
	av_free(c);
	av_free_packet(&pkt);
	av_frame_free(&m_pRGBFrame);
	av_freep(&m_pYUVFrame->data[0]);
	av_frame_free(&m_pYUVFrame);
	av_free(fmtctx);
	pYUV_buffer =NULL;
}

int CamThread::GetCamParam()
{
	Mat tmpframe;
	uint32  num  = 0;
	m_vcap.open(CV_CAP_FIREWARE);
	if(!m_vcap.open(m_videoStream)) {

		dbgprint("%s(%d),%d CAM opening video stream failed!\n",DEBUGARGS,m_index);
		return -1;
	}

	m_cols    =	 m_vcap.get(CV_CAP_PROP_FRAME_WIDTH);
	m_rows    =  m_vcap.get(CV_CAP_PROP_FRAME_HEIGHT);
	m_fps     =  m_vcap.get(CV_CAP_PROP_FPS);

	while(!(m_vcap.read(tmpframe))) {
		num++;
		if(num > 500){
				dbgprint("%s(%d),%d CAM read video stream failed!\n",DEBUGARGS,m_index);
				return -1;
		}

	}

	ReadFrame.create(m_rows,m_cols,tmpframe.type());
	EncodeInit();

	StartRTSPServer(&AnalyzeCamera);
	return 0;
}

int CamThread::EncodeInit()
{
	c= NULL;

	int yuv420_bytes = 0;
	m_pRGBFrame =  new AVFrame[1];  //RGB帧数据
	m_pYUVFrame =  new AVFrame[1];  //YUV帧数据

	av_register_all();
	avcodec_register_all();
	avformat_network_init();

	fmtctx = avformat_alloc_context();

  codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  if (!codec) {
      fprintf(stderr, "Codec not found\n");
      exit(1);
  }

	video_st = avformat_new_stream(fmtctx, codec);

	c = video_st->codec;

	avcodec_get_context_defaults3(c, codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }
	c->codec_id = AV_CODEC_ID_H264;

	/* resolution must be a multiple of two */
	c->width  = m_cols;
	c->height = m_rows;
	/* frames per second */
	c->time_base = (AVRational){1,m_fps};
	c->gop_size = 10;
	c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P;

	 /* open it */
	if (avcodec_open2(c, codec, NULL) < 0) {
	    fprintf(stderr, "Could not open codec\n");
	    exit(1);
	}

	//yuv
	m_pYUVFrame = av_frame_alloc();
	yuv420_bytes = avpicture_get_size( AV_PIX_FMT_YUV420P, m_cols, m_rows);
	pYUV_buffer = (uint8_t *)av_malloc(yuv420_bytes*sizeof(uint8_t));
	m_pYUVFrame->format = c->pix_fmt;
	m_pYUVFrame->width  = c->width;
	m_pYUVFrame->height = c->height;
	avpicture_fill((AVPicture*)m_pYUVFrame, pYUV_buffer, AV_PIX_FMT_YUV420P, m_cols, m_rows);

	scxt = sws_getContext(m_cols, m_rows,AV_PIX_FMT_RGB24, m_cols, m_rows,AV_PIX_FMT_YUV420P, 0, 0, 0, 0);
	return 0;
}

int CamThread::Encode(Mat &frame)
{
	int iRet = -1;
	int got_output = 0;
	//将BGR 转为RGB
	cvtColor( frame , rgb_frame, CV_BGR2RGB ) ;
  //将RGBdata填充  (AVFrame*)m_pRGBFrame
	avpicture_fill((AVPicture*)m_pRGBFrame,(uint8_t *)rgb_frame.data,AV_PIX_FMT_RGB24,m_cols,m_rows);

  //将RGB转化为YUV
	sws_scale(scxt,m_pRGBFrame->data,m_pRGBFrame->linesize,0,m_rows,m_pYUVFrame->data,m_pYUVFrame->linesize);

	av_init_packet(&pkt);
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	pkt.pts = AV_NOPTS_VALUE;
	pkt.dts = AV_NOPTS_VALUE;

	/* encode the image */
	iRet = avcodec_encode_video2(c, &pkt, m_pYUVFrame, &got_output);
	if (iRet < 0) {
		fprintf(stderr, "Error encoding frame\n");
		exit(1);
	}

	//If size is zero, it means the image was buffered.

	if (got_output)
	{
		if (c->coded_frame->key_frame)pkt.flags |= AV_PKT_FLAG_KEY;
		pkt.stream_index = video_st->index;
		if (pkt.pts != AV_NOPTS_VALUE )
		{
			pkt.pts = av_rescale_q(pkt.pts,video_st->codec->time_base, video_st->time_base);
		}
		if(pkt.dts !=AV_NOPTS_VALUE )
		{
			pkt.dts = av_rescale_q(pkt.dts,video_st->codec->time_base, video_st->time_base);
		}
		/* Write the compressed frame to the media file. */
		if( AnalyzeCamera.m_capture_callback != NULL )
				AnalyzeCamera.m_capture_callback(0, pkt.size, pkt.data, 600300001);
	}
	else {
		iRet = 0;
	}

	return 0;
}

void CamThread::run()
{
	int iRet = -1;
	while(m_CameraFlag)
	{
			if(!(m_vcap.read(ReadFrame)))
			{
					dbgprint("%s(%d),%d CAM no frame!\n",DEBUGARGS,m_index);
					waitKey(40);
					continue;
			}

			if(ReadFrame.empty())
			{
				waitKey(40);
				continue;
			}

	    ReadFrame.copyTo(EncodeFrame);

      Encode(EncodeFrame);
	}

	dbgprint("%s(%d),%d CamThread exit!\n",DEBUGARGS,m_index);
	pthread_exit(NULL);
}

int CamThread::CreateCamThread()
{
	int iRet = -1;
	pthread_t CamThread;
	iRet = pthread_create(&CamThread,NULL,RunCamThread,this);
	if(iRet != 0){
		 dbgprint("%s(%d),create %d CamThread failed!\n",DEBUGARGS,m_index);
		 return -1;
	}
	pthread_detach(CamThread);
	return 0;
}
