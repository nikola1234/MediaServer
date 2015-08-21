#include "CamTimeThread.h"

CamTimeThread::CamTimeThread(SingleCamera* sincam)
{
	cam =sincam;
	CameraID = cam ->CameraID;

	m_TimeFlag = true;
	m_Status     = false;

	Ana1Status = false;
	Ana2Status = false;
	AnalyzeNUM = 0;


	pthread_mutex_init (&mut,NULL);
	pthread_cond_init(&cond, NULL);

}

CamTimeThread::~CamTimeThread()
{
	pthread_mutex_destroy(&mut);
	pthread_cond_destroy(&cond);
}

void CamTimeThread::SetCamera_StartThread(uint8 num)
{
	AnalyzeNUM = num;
	CreateTimeThread();
}

void CamTimeThread::parse_time_further_more(ALARM_TIME_INT* time_i,ALARM_TIME* time_c)
{
	char hour[3];
	char min[3];
	memset(hour, 0, 3);
	memset(min, 0, 3);

	memcpy(hour, time_c->StartTime, 2);
	memcpy(min,  time_c->StartTime+3, 2);

	time_i->Start.hour = atoi(hour);
	time_i->Start.min  = atoi(min);

	memset(hour, 0, 3);
	memset(min, 0, 3);

	memcpy(hour,  time_c->EndTime, 2);
	memcpy(min,  time_c->EndTime+3, 2);
	time_i->End.hour = atoi(hour);
	time_i->End.min= atoi(min);

}
void CamTimeThread::parse_time_further(ALARM_TIME_INT* time_int,ALARM_TIME* time_char)
{
	uint8 i = 0;
	for(i=0; i < TIME_NUM_3 ; i++)
	{
		parse_time_further_more(&time_int[i] , &time_char[i]);
	}
}
void CamTimeThread::parse_time(ALARM_DAY* AlarmTime,T_VDCS_VIDEO_ALARM_TIME *Time)
{
	uint8 i= 0;
	for(i = 0; i < WEEK_DAY_LEN_7; i++ )
	{
		parse_time_further(AlarmTime[i].dayTime.time,Time[i].alarmtime.Time);
	}

}
void CamTimeThread::reset_time1_param(T_VDCS_VIDEO_ALARM_TIME *Time)
{
	uint8 i= 0;
	//int j = 0;
	pause();
	usleep(100*1000);
	for(i = 0; i < WEEK_DAY_LEN_7; i++ )
	{
		memset(&AlarmTime1[i] , 0,sizeof(ALARM_DAY));
	}
	parse_time(AlarmTime1,Time);
	/*
	for(i = 0;i < 7; i++)
	{
		 for(j = 0; j < 3; j++)
		 {
		 	printf(" time %d  start time is %d : %d ",j,AlarmTime1[i].dayTime.time[j].Start.hour ,AlarmTime1[i].dayTime.time[j].Start.min);
		 	printf(" time %d  End  time is %d : %d ",j,AlarmTime1[i].dayTime.time[j].End.hour ,AlarmTime1[i].dayTime.time[j].End.min);
		 }
		 printf("\n");
	}	
	*/
	resume();
}

void CamTimeThread::reset_time2_param(T_VDCS_VIDEO_ALARM_TIME *Time)
{
	uint8 i= 0;
	pause();
	usleep(100*1000);
	for(i = 0; i < WEEK_DAY_LEN_7; i++ )
	{
		memset(&AlarmTime2[i] , 0,sizeof(ALARM_DAY));
	}
	parse_time(AlarmTime2,Time);
	resume();
}


void CamTimeThread::set_analyze_num( uint8 num)
{
	AnalyzeNUM = num;
}

void CamTimeThread::change_time_by_num(uint8 num,ALARM_DAY* time)
{
	uint8 i = 0;
	switch (num) {
	case 1:
	      for(i=0;i < WEEK_DAY_LEN_7;i++)
	      {
	        memcpy(&AlarmTime1[i] , &time[i] ,sizeof(ALARM_DAY));
	      }
	      break;
	case 2:
	      for(i=0;i < WEEK_DAY_LEN_7;i++)
	      {
	        memcpy(&AlarmTime2[i] , &time[i] ,sizeof(ALARM_DAY));
	      }
	      break;
	default :break;
	}
}

void CamTimeThread::clear_time()
{
	uint8 i = 0;
	for(i=0;i < WEEK_DAY_LEN_7 ;i++)
	{
	memset(&AlarmTime1[i] , 0 ,sizeof(ALARM_DAY));
	}

	for(i=0;i < WEEK_DAY_LEN_7 ;i++)
	{
	memset(&AlarmTime2[i] , 0 ,sizeof(ALARM_DAY));
	}

	Ana1Status = false;
	Ana2Status = false;
}

int CamTimeThread::AlarmTimeCompare(T_AlarmTime & t1,T_AlarmTime &t2)
{
	if(t1.hour < t2.hour) return -1;
	if(t1.hour > t2.hour) return 1;
	if(t1.hour  ==  t2.hour)
	{
		if(t1.min < t2.min) return -1;
		if(t1.min > t2.min) return 1;
		if(t1.min == t2.min) return 0;
	}
	return 2;
}

int CamTimeThread::JudgeTimeDayStart(T_AlarmTime &time) /* 00:00 */
{
	if( (time.hour == 0)&&(time.min == 0) ) return 1;
	return 0;
}

int CamTimeThread::JudgeTimeDayEnd(T_AlarmTime &time)  /* 23:59 */
{
	if( (time.hour == 23)&&(time.min == 59) ) return 1;
	return 0;
}

int CamTimeThread::time_analyze(T_AlarmTime timenow, ALARM_TIME_INT*time_int )
{
	int iRet = -1;

	iRet = JudgeTimeDayStart(time_int[0].Start) && JudgeTimeDayStart(time_int[0].End);
	if(iRet == 1) return 0;

	iRet = AlarmTimeCompare(timenow ,time_int[0].Start);
	if(iRet < 0) return 0;

	iRet = AlarmTimeCompare(timenow ,time_int[0].End);
	if(iRet <= 0) return 1;

	iRet = JudgeTimeDayStart(time_int[1].Start) && JudgeTimeDayStart(time_int[1].End);
	if(iRet == 1) return 0;

	iRet = AlarmTimeCompare(timenow ,time_int[1].Start);
	if(iRet < 0)  return 0;

	iRet = AlarmTimeCompare(timenow ,time_int[1].End);
	if(iRet <= 0) return 1;

	iRet = JudgeTimeDayStart(time_int[2].Start) && JudgeTimeDayStart(time_int[2].End);
	if(iRet == 1) return 0;

	iRet = AlarmTimeCompare(timenow ,time_int[2].Start);
	if(iRet < 0)  return 0;

	iRet = AlarmTimeCompare(timenow ,time_int[2].End);
	if(iRet <= 0) return 1;
	return 0;

	return 2;
}

void  CamTimeThread::change_analyze_status(int ret,bool &status)
{
	//printf("ret is %d\n",ret);
	switch (ret){
		case 0:
			if(status == true)
				status = false;
			break;
		case 1:
			if(status == false)
				status = true;
			break;
		default:
			dbgprint("%s(%d),%d CamTimeThread wrong time_analyze %d!\n",DEBUGARGS,CameraID,ret);
			break;
	}
}
int CamTimeThread::time_detect(ALARM_DAY * Time)
{
	uint8 day = 0;
	int iRet = -1;
	time_t timep;
	struct tm * pTM=NULL;

	T_AlarmTime NowTime;

	time(&timep);

	pTM = localtime(&timep);

	if(pTM->tm_wday == 0) pTM->tm_wday =7;
	day =  pTM->tm_wday -1;

	NowTime.hour = (uint8)pTM->tm_hour;
	NowTime.min  = (uint8)pTM->tm_min;

	iRet = time_analyze(NowTime,Time[day].dayTime.time);
	//change_analyze_status(iRet,status);
	return iRet;
}

void CamTimeThread::time_detect_by_num(uint8 num)
{
	int iRet = -1;
	switch(AnalyzeNUM){
		case 1:
			iRet =time_detect(AlarmTime1);
			change_analyze_status(iRet,cam->Ana1Thread->AnalyzeEn);
			break;
		case 2:
			iRet = time_detect(AlarmTime2);
			change_analyze_status(iRet,cam->Ana2Thread->AnalyzeEn);
			break;
		default :
			dbgprint("%s(%d),%d CamTimeThread wrong AnalyzeNUM %d!\n",DEBUGARGS,CameraID,AnalyzeNUM);
			break;
	}
}

void CamTimeThread::run()
{
	while(m_TimeFlag){
		pthread_mutex_lock(&mut);
		while (!m_Status)
		{
		  pthread_cond_wait(&cond, &mut);
		}
		pthread_mutex_unlock(&mut);

		switch(AnalyzeNUM){
		case 1:
			time_detect_by_num(1);
			break;
		case 2:
			time_detect_by_num(1);
			time_detect_by_num(2);
			break;
		default :
			break;
		}
	  	usleep(50*1000);
	}

	dbgprint("%s(%d),%d CamTimeThread exit!\n",DEBUGARGS,CameraID);
	pthread_exit(NULL);
}

int CamTimeThread::CreateTimeThread()
{
	int iRet = -1;
	pthread_t TimeThread;
	iRet = pthread_create(&TimeThread,NULL,RunTimeThread,this);
	if(iRet != 0)
 	 {
		 dbgprint("%s(%d),cam %d create TimeThread failed!\n",DEBUGARGS,CameraID);
		 return -1;
	}
	dbgprint("%s(%d),cam %d create TimeThread success!\n",DEBUGARGS,CameraID);
	pthread_detach(TimeThread);
	return 0;
}

void CamTimeThread::resume()
{
    if (m_Status == false)
    {
        pthread_mutex_lock(&mut);
        m_Status = true;
        pthread_cond_signal(&cond);
        dbgprint("%s(%d), cam %d time pthread run!\n",DEBUGARGS,CameraID);
        pthread_mutex_unlock(&mut);
    }
    else
    {
        dbgprint("%s(%d), cam %d time pthread run already!\n",DEBUGARGS,CameraID);
    }
}

void CamTimeThread::pause()
{
    if (m_Status == true)
    {
        pthread_mutex_lock(&mut);
        m_Status = false;
        dbgprint("%s(%d), cam %d time pthread stop!\n",DEBUGARGS,CameraID);
        pthread_mutex_unlock(&mut);
    }
    else
    {
        dbgprint("%s(%d), cam %d time pthread stop already!\n",DEBUGARGS,CameraID);
    }
}
