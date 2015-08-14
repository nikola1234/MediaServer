#include "CamTimeThread.h"

CamTimeThread::CamTimeThread(uint32 ID)
{
	CameraID = ID;
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

int CamTimeThread::time_analyze(T_AlarmTime &timenow,ALARM_TIME_INT &time1, ALARM_TIME_INT &time2)
{
	int iRet = -1;

	iRet = JudgeTimeDayStart(time1.Start)&&JudgeTimeDayStart(time1.End);
	if(iRet == 1) return 0;

	iRet = AlarmTimeCompare(timenow ,time1.Start);
	if(iRet < 0) return 0;
	if(iRet == 0) return 1;

	iRet = AlarmTimeCompare(timenow ,time1.End);
	if(iRet <= 0) return 1;

	iRet = JudgeTimeDayStart(time2.Start)&&JudgeTimeDayStart(time2.End);
	if(iRet == 1) return 0;

	iRet = AlarmTimeCompare(timenow ,time2.Start);
	if(iRet < 0)  return 0;

	iRet = AlarmTimeCompare(timenow ,time2.End);
	if(iRet <= 0) return 1;
	if(iRet >0) return 0;
	return 2;
}

void  CamTimeThread::change_analyze_status(int ret,bool &status)
{
	switch (ret){
		case 0:
			if(status ==false)
			if(status == true)
				status = false;
			break;
		case 1:
			if(status == true)
			if(status == false)
				status = true;
			break;
		default: break;
	}
}
int CamTimeThread::time_detect(ALARM_DAY * Time,bool &status)
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

	if(Time[day].En == 0)
	{
		status = false;
		return 0;
	}

	NowTime.hour = (uint8)pTM->tm_hour;
	NowTime.min  = (uint8)pTM->tm_min;

	iRet = time_analyze(NowTime,Time[day].dayTime.time1,Time[day].dayTime.time2);
	change_analyze_status(iRet,status);
	return 0;
}

void CamTimeThread::time_detect_by_num(uint8 num)
{
	switch(AnalyzeNUM){
		case 1:
			time_detect(AlarmTime1,Ana1Status);
			break;
		case 2:
			time_detect(AlarmTime2,Ana2Status);
			break;
		default :
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
	  	sleep(2);
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
