#include "CamAnaThread.h"

CamAnaThread::CamAnaThread(SingleCamera* sincam,NetServer *ser)
{
	cam = sincam;
	CameraID = cam->CameraID;

	server = ser;
	
	m_AnaFlag   = true;
	m_Status      = false;
	AnalyzeType      = 0;
	AnaIndex  	= 0;
	alarm         = 0;
	AnalyzeEn = false;

	frame = 0;
	startflag         = 0;
	stopflag          = 0;
	intervalflag      = 0;
	alarmStartframe   = 0;
	alarmStopframe    = 0;

	pthread_mutex_init (&mut,NULL);
	pthread_cond_init(&cond, NULL);


	region = new CRegion(CameraID);
	videoHandler = NULL;
	fire = new CFire(CameraID ,&videoHandler);
	smoke = new CSmoke(CameraID);
	human = new CHuman(CameraID);

	/*human*/
	memset(&t_HumanNum,0,sizeof(T_HUMANNUM));
}

CamAnaThread::~CamAnaThread()
{
	pthread_mutex_destroy(&mut);
	pthread_cond_destroy(&cond);

	delete region;
	region = NULL;
	videoHandler = NULL;
	delete fire;
	fire = NULL;
	delete smoke;
	smoke = NULL;
	delete human;
	human = NULL;
}

void CamAnaThread::set_video_draw( vector <VIDEO_DRAW> &DrawPkg)
{
	uint8 i =0;
	pkg.clear();
	for(i = 0; i <DrawPkg.size(); i++)
	{
		VIDEO_DRAW  tmp;
		tmp = DrawPkg[i];
		pkg.push_back(tmp);
	}
}

int CamAnaThread::alarmStrategy()
{
	//alarm strategy 5 frame alarm  and 4s later start another
	if( 1 == alarm && !startflag ){
		alarmStartframe++;
	}else{
		alarmStartframe = 0;
	}

	if(0 == alarm && stopflag){
		alarmStopframe++;
	}else{
		alarmStopframe = 0;
	}

	if(alarmStartframe > START_FRAME_INDEX)
	{
		startflag = 1;
		intervalflag = 1;
		alarmStartframe = 0;
		return alarmOn;
	}

	if(1 == intervalflag)
	{
		frame++;
	}

	if (frame > INTERVAL_FRAME_INDEX)
	{
		stopflag  = 1;
		intervalflag = 0;
		frame = 0;
	}

	if(alarmStopframe >  STOP_FRAME_INDEX)
	{
		stopflag  = 0;
		startflag = 0;
		alarmStopframe = 0;
		return alarmStop;
 	 }

 	 return 0;
}

int CamAnaThread::send_alarm_to_mcu(uint16 type,uint8 status)
{
	int iRet = -1;
	iRet = server->SendBufferToMCUClient(CameraID,status);
	return iRet;
}

int CamAnaThread::send_alarm_to_client(uint16 type,uint8 status)
{
	int iRet = -1;
	T_PacketHead                                 t_warnHead;
	T_SM_ANAY_VDCS_WARN_INFO t_warninfo;
	char warnBuff[28+150] ={0};

	t_warnHead.magic	   		 =  T_PACKETHEAD_MAGIC;
	t_warnHead.cmd			  =  SM_ANAY_VDCS_WARN_INFO;
	t_warnHead.UnEncryptLen	 =  sizeof(T_SM_ANAY_VDCS_WARN_INFO);
	memcpy(warnBuff,&t_warnHead,sizeof(T_PacketHead));

	memcpy(t_warninfo.CameUrl ,cam->CamUrl,SINGLE_URL_LEN_128);
	t_warninfo.WarnType = type;
	t_warninfo.Status       = status;
	if(type == HumanDetect)
	{
		t_warninfo.numALL  = t_HumanNum.humanALL;
		t_warninfo.Door[0].in    	=	t_HumanNum.doorIN[0];
		t_warninfo.Door[0].out  	=	t_HumanNum.doorOUT[0];
		t_warninfo.Door[1].in     	=	t_HumanNum.doorIN[1];
		t_warninfo.Door[1].out    =	t_HumanNum.doorOUT[1];
	}
	memcpy(warnBuff+sizeof(T_PacketHead),&t_warninfo,sizeof(T_SM_ANAY_VDCS_WARN_INFO));
	
	iRet = server->SendBufferToNetClient(warnBuff,sizeof(warnBuff));
	return iRet;
}

int CamAnaThread::human_detect(Mat &frame)
{
	int iRet = -1;

	if(!frame.empty())
	{
		human->HumanDetectRun(frame);
		alarm = human->alarm ;
		t_HumanNum = human->GetAlarmHumanNum();
	
	}else{
		alarm = 0;
		usleep(40*1000);
	}
	
	iRet =  alarmStrategy();

	if(iRet == alarmOn)
	{
		//TODO: notify client huamn alarm start 
		dbgprint("%s(%d),cam %d  human alarm start !\n",DEBUGARGS,CameraID);
		//send_alarm_to_client(HumanDetect,1);
		send_alarm_to_mcu(HumanDetect,1);
	}

	if(iRet == alarmStop)
	{
		//TODO: notify client huamn alarm stop
		dbgprint("%s(%d),cam %d  human alarm stop !\n",DEBUGARGS,CameraID);
		//send_alarm_to_client(HumanDetect,0);
		send_alarm_to_mcu(HumanDetect,0);
	}
	return 0;
}

int  CamAnaThread::region_detect(Mat &frame)
{
	int iRet = -1;
	if(!frame.empty())
	{
		region->RegionDetectRun(frame);
  		alarm = region->alarm ;
		usleep(30*1000);
	}else{
		alarm = 0;
		usleep(40*1000);
	}
	iRet =  alarmStrategy();

	if(iRet == alarmOn)
	{
		//TODO: notify client region alarm start 
		dbgprint("%s(%d),cam %d  region alarm start !\n",DEBUGARGS,CameraID);
		//send_alarm_to_client(RegionDetect,1);
		send_alarm_to_mcu(RegionDetect,1);
	}

	if(iRet == alarmStop)
	{
		//TODO: notify client region alarm stop
		dbgprint("%s(%d),cam %d  region alarm stop !\n",DEBUGARGS,CameraID);
		//send_alarm_to_client(RegionDetect,0);
		send_alarm_to_mcu(RegionDetect,0);
	}
	return 0;
}

int  CamAnaThread::fire_detect(Mat &frame)
{
	int iRet = -1;

	if(!frame.empty())
	{
		fire->FireDetectRun(frame,(void *)videoHandler);
		alarm = fire->alarm ;
	}
	else
	{
		alarm = 0;
		usleep(40*1000);
	}
	iRet =  alarmStrategy();

	if(iRet == alarmOn)
	{
		//TODO: notify client fire alarm start
		dbgprint("%s(%d),cam %d  fire alarm start !\n",DEBUGARGS,CameraID);
		//send_alarm_to_client(FireDetect,1);
		send_alarm_to_mcu(FireDetect,1);
	}

	if(iRet == alarmStop)
	{
		//TODO: notify client fire alarm stop
		dbgprint("%s(%d),cam %d  fire alarm stop !\n",DEBUGARGS,CameraID);
		//send_alarm_to_client(FireDetect,0);
		send_alarm_to_mcu(FireDetect,0);
	}
	return 0;
}

int  CamAnaThread::smoke_detect(Mat &frame)
{
	int iRet = -1;

	if(!frame.empty())
	{
		smoke->SmokeDetectRun(frame);
		alarm = smoke->alarm ;
	}else{
		alarm = 0;
		usleep(40*1000);
	}
	
	iRet =  alarmStrategy();

	if(iRet == alarmOn)
	{
		//TODO: notify client smoke alarm start and push rtsp
		dbgprint("%s(%d),cam %d  smoke alarm start !\n",DEBUGARGS,CameraID);
		//send_alarm_to_client(SmokeDetect,1);
		send_alarm_to_mcu(SmokeDetect,1);
	}

	if(iRet == alarmStop)
	{
		//TODO: notify client smoke alarm stop
		dbgprint("%s(%d),cam %d  smoke alarm stop !\n",DEBUGARGS,CameraID);
		//send_alarm_to_client(SmokeDetect,0);
		send_alarm_to_mcu(SmokeDetect,0);
	}
	return 0;
}


int CamAnaThread::alarm_run(Mat &frame ,uint8 iType)
{
	switch (iType) {
	case HumanDetect:
	      human_detect(frame);
	      break;
	case SmokeDetect:
	      smoke_detect(frame);
	      break;
	case RegionDetect:
	      region_detect(frame);
	      break;
	case FixedObjDetect:
	      //fixobj_detect(frame);
	      break;
	case FireDetect:
	      fire_detect(frame);
	      break;
	case ResidueDetect:
	      //residue_detect(frame);
	      break;
	default:
	      dbgprint("%s(%d),cam %d alarmindex  %d wrong AnalyzeType !\n",DEBUGARGS,CameraID,iType);
	      usleep(40*1000);
	      break;
	}
	return 0;
}

void CamAnaThread::analyze_pause_release()
{
	alarm = 0;

	frame = 0;
	startflag         = 0;
	stopflag          = 0;
	intervalflag      = 0;
	alarmStartframe   = 0;
	alarmStopframe    = 0;

	//human
	human->pause_release();
	region->pause_release();
	fire->pause_release();
	smoke->pause_release();
}

void CamAnaThread::analyze_sleep_release()
{
	alarm = 0;

	frame = 0;
	startflag         = 0;
	stopflag          = 0;
	intervalflag      = 0;
	alarmStartframe   = 0;
	alarmStopframe    = 0;

	//human
	human->sleep_release();
	region->sleep_release();
	fire->sleep_release();
	smoke->sleep_release();
}


int CamAnaThread::parse_human_draw_package(vector <VIDEO_DRAW> & Pkg,vector<Rect>& tmprect,vector<Line>  &tmpline)
{
	uint16 i = 0;
	int x= 0;
	int y = 0;
	int width = 0;
	int height = 0;

	tmpline.clear();
	tmprect.clear();
	
	if(Pkg.size() == 0)
	{
		dbgprint("%s(%d),%d camera parse_human_draw_package size is 0!\n",DEBUGARGS,CameraID);
		return -1;
	}

	for(i=0; i < Pkg.size(); i++ )
	{
		VIDEO_DRAW tmp = Pkg[i];
		if(tmp.Type == 1)
		{
			x=(int )tmp.StartX;
			y=(int )tmp.StartY;
			width =(int )tmp.EndX;
			height=(int )tmp.EndY;		
			tmprect.push_back(Rect(x, y, width, height));
		}else if(tmp.Type ==2)
		{ 
			Line line;
			line.Start.x =tmp.StartX;
			line.Start.y=tmp.StartY;
			line.End.x=tmp.EndX;
			line.End.y=tmp.EndY;
			tmpline.push_back(line);
		}else{
			dbgprint("%s(%d),%d camera parse_human_draw_package wrong type!\n",DEBUGARGS,CameraID);
		}
	}

	if(tmpline.size() == 0 && tmprect.size() == 0)
	{
		dbgprint("%s(%d),%d camera parse_human_draw_package rect and line size is 0!\n",DEBUGARGS,CameraID);
		return -1;
	}
	return 0;
}

int CamAnaThread::parse_draw_package(vector <VIDEO_DRAW> & Pkg,vector<Rect>& tmprect)
{
	uint16 i = 0;
	int x= 0;
	int y = 0;
	int width = 0;
	int height = 0;

	tmprect.clear();
	
	if(Pkg.size() == 0)
	{
		dbgprint("%s(%d),%d camera parse_draw_package size is 0!\n",DEBUGARGS,CameraID);
		return -1;
	}

	for(i=0; i < Pkg.size(); i++ )
	{
		VIDEO_DRAW tmp = Pkg[i];
		if(tmp.Type == 1)
		{
			x=(int )tmp.StartX;
			y=(int )tmp.StartY;
			width =(int )tmp.EndX;
			height=(int )tmp.EndY;		
			tmprect.push_back(Rect(x, y, width, height));
		}
		else
		{
			dbgprint("%s(%d),%d camera parse_draw_package wrong type!\n",DEBUGARGS,CameraID);
		}
	}

	if( tmprect.size() == 0)
	{
		dbgprint("%s(%d),%d camera parse_draw_package rect and line size is 0!\n",DEBUGARGS,CameraID);
		return -1;
	}
	return 0;
}

void CamAnaThread::set_analyze_vector( vector <VIDEO_DRAW> & DrawPkg)
{	
	vector<Rect> tmpRect;
	vector<Line>  tmpLine;
	switch (AnalyzeType){
		case HumanDetect:
			parse_human_draw_package(DrawPkg,tmpRect,tmpLine);
			human->set_rectangle_line(tmpRect,tmpLine);
			break;
		case SmokeDetect:
			parse_draw_package(DrawPkg,tmpRect);
			smoke->set_rectangle(tmpRect);
			break;		
		case RegionDetect:
			parse_draw_package(DrawPkg,tmpRect);
			region->set_rectangle(tmpRect);
			break;
		case FixedObjDetect:
			break;		
		case FireDetect:
			parse_draw_package(DrawPkg,tmpRect);
			fire->set_rectangle(tmpRect);
			break;		
		case ResidueDetect:
			break;		
		default :
			dbgprint("%s(%d),%d set_analyze_vector analyze wrong!\n",DEBUGARGS,CameraID);
			break;
	}
}


void CamAnaThread::SetCamera_StartThread(uint16 type,uint8 num)
{
	AnalyzeType  = type;
	AnaIndex = num;
	CreateAnaThread();
}

int CamAnaThread::check_thread_status()
{
	if((m_Status == true)&&(AnalyzeEn ==true))
	{
		return 1;
	}
	return 0;
}


void CamAnaThread::run()
{  
	while(m_AnaFlag){
		pthread_mutex_lock(&mut);
		while (!m_Status)
		{
			  analyze_pause_release();
			  pthread_cond_wait(&cond, &mut);
			  set_analyze_vector(pkg);
		}
		pthread_mutex_unlock(&mut);
		//printf("AnalyzeEn is %d\n",AnalyzeEn);
		if(AnalyzeEn)
		{
			cam->ReadThread->anaframe.copyTo(origFrame);
			alarm_run(origFrame,AnalyzeType);
		}else{
			analyze_sleep_release();
			usleep(40*1000);
		}
	}

	dbgprint("%s(%d),%d CAlarmThread exit!\n",DEBUGARGS,CameraID);
	pthread_exit(NULL);
}

int CamAnaThread::CreateAnaThread()
{
	int iRet = -1;
	pthread_t AnaThread;
	iRet = pthread_create(&AnaThread,NULL,RunAnaThread,this);
	if(iRet != 0)
  	{
		 dbgprint("%s(%d),cam %d create %d AlarmThread failed!\n",DEBUGARGS,CameraID,AnaIndex);
		 return -1;
	}
	 dbgprint("%s(%d),cam %d create %d AlarmThread sucess!\n",DEBUGARGS,CameraID,AnaIndex);
	pthread_detach(AnaThread);
	return 0;
}

void CamAnaThread::resume()
{
    if (m_Status == false)
    {
        pthread_mutex_lock(&mut);
        m_Status = true;
        pthread_cond_signal(&cond);
        dbgprint("%s(%d), cam %d alarm %d pthread run!\n",DEBUGARGS,CameraID,AnaIndex);
        pthread_mutex_unlock(&mut);
    }
    else
    {
        dbgprint("%s(%d), cam %d alarm %d pthread run already!\n",DEBUGARGS,CameraID,AnaIndex);
    }
}

void CamAnaThread::pause()
{
    if (m_Status == true)
    {
        pthread_mutex_lock(&mut);
        m_Status = false;
        dbgprint("%s(%d), cam %d alarm %d pthread stop!\n",DEBUGARGS,CameraID,AnaIndex);
        pthread_mutex_unlock(&mut);
    }
    else
    {
        dbgprint("%s(%d), cam %d alarm %d pthread stop already!\n",DEBUGARGS,CameraID,AnaIndex);
    }
}
