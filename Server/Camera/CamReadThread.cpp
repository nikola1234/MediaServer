#include "CamReadThread.h"
#include "RtspCamera.h"
extern CRtspCamera rtspCamera;

CamReadThread::CamReadThread(string url,uint32 ID)
{
  CameraID = ID;
  m_videoStream =url;
  m_CameraFlag = true;
}


int CamReadThread::InitCamera()
{
  int iRet = -1;
  iRet = GetCamParam();
  if(iRet < 0) return -1;

  iRet = EncodeInit();
  if(iRet < 0) return -1;
  return 0;
}

void CamReadThread::releaseEncode()
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

int CamReadThread::EncodeInit()
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
      dbgprint("%s(%d),%d CAM Codec not found!\n",DEBUGARGS,CameraID);
  		return -1;
  }

	video_st = avformat_new_stream(fmtctx, codec);
	c = video_st->codec;
	avcodec_get_context_defaults3(c, codec);
  if (!c) {
    dbgprint("%s(%d),%d CAM Could not allocate video codec context!\n",DEBUGARGS,CameraID);
    return -1;
  }
	c->codec_id = AV_CODEC_ID_H264;
	c->width  = m_cols;
	c->height = m_rows;
	c->time_base = (AVRational){1,m_fps};
	c->gop_size = 10;
	c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P;

	if (avcodec_open2(c, codec, NULL) < 0) {
      dbgprint("%s(%d),%d CAM Could not open codec!\n",DEBUGARGS,CameraID);
      return -1;
	}

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

int CamReadThread::GetCamParam()
{
	Mat tmpframe;
	uint32  num  = 0;
  m_vcap.open(CV_CAP_FIREWARE);
  if(!m_vcap.open(m_videoStream)) {
		dbgprint("%s(%d),%d CAM opening video stream failed!\n",DEBUGARGS,CameraID);
		return -1;
	}

	m_cols    =	 m_vcap.get(CV_CAP_PROP_FRAME_WIDTH);
	m_rows    =  m_vcap.get(CV_CAP_PROP_FRAME_HEIGHT);
	m_fps     =  m_vcap.get(CV_CAP_PROP_FPS);

	while(!(m_vcap.read(tmpframe))) {
		num++;
		if(num > 500){
				dbgprint("%s(%d),%d CAM read video stream failed!\n",DEBUGARGS,CameraID);
				return -1;
		}
	}
	ReadFrame.create(m_rows,m_cols,tmpframe.type());
	return 0;
}

int CamReadThread::Encode(Mat &frame)
{
	int iRet = -1;
	int got_output = 0;

	cvtColor( frame , rgb_frame, CV_BGR2RGB ) ;
	avpicture_fill((AVPicture*)m_pRGBFrame,(uint8_t *)rgb_frame.data,AV_PIX_FMT_RGB24,m_cols,m_rows);
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

		if( rtspCamera.m_capture_callback != NULL )
				rtspCamera.m_capture_callback(0, pkt.size, pkt.data, CameraID);
	}
	else {
		iRet = 0;
	}

	return 0;
}

void CamReadThread::run()
{
	while(m_CameraFlag)
	{
			if(!(m_vcap.read(ReadFrame)))
			{
					dbgprint("%s(%d),%d CAM no frame!\n",DEBUGARGS,CameraID);
					waitKey(40);
					continue;
			}

			if(ReadFrame.empty())
			{
				waitKey(40);
				continue;
			}
      ReadFrame.copyTo(frame1);
      ReadFrame.copyTo(frame2);
      ReadFrame.copyTo(EncodeFrame);
      //TODO::EncodeFrame
      Encode(EncodeFrame);
	}
  releaseEncode();
	dbgprint("%s(%d),%d CamThread exit!\n",DEBUGARGS,CameraID);
	pthread_exit(NULL);
}

int CamReadThread::CreateCamReadThread()
{
	int iRet = -1;
	pthread_t CamReadThread;
	iRet = pthread_create(&CamReadThread,NULL,RunCamThread,this);
	if(iRet != 0){
		 dbgprint("%s(%d),create %d CamReadThread failed!\n",DEBUGARGS,CameraID);
		 return -1;
	}
	pthread_detach(CamReadThread);
	return 0;
}
