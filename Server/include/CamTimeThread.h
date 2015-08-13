#ifndef _CAM_TIME_THREAD_H_
#define _CAM_TIME_THREAD_H_

#include "Common.h"
#include "Data.h"

class CamTimeThread
{
public:

	CamTimeThread(uint32 ID);
	~CamTimeThread();

	bool Ana1Status;
	bool Ana2Status;

	void set_analyze_num( uint8 num);
	void change_time_by_num(uint8 num,ALARM_DAY * time);
	void clear_time();

	void run();
	int  CreateTimeThread();
	static void* RunTimeThread(void*  param){
		CamTimeThread* p = (CamTimeThread*)param;
		p->run();
		return NULL;
	}
	void quit(){m_TimeFlag = false;}
	void resume();
	void pause();

private:
	uint32 CameraID;
	uint8   AnalyzeNUM;
	bool  m_TimeFlag;
	bool  m_Status;
	pthread_mutex_t mut;
	pthread_cond_t cond;

	ALARM_DAY  AlarmTime1[WEEK_DAY_LEN_7];
	ALARM_DAY  AlarmTime2[WEEK_DAY_LEN_7];

	int AlarmTimeCompare(T_AlarmTime & t1,T_AlarmTime &t2);
	int JudgeTimeDayStart(T_AlarmTime &time);
	int JudgeTimeDayEnd(T_AlarmTime &time);
	void change_analyze_status(int ret,bool &status);
	int time_analyze(T_AlarmTime &timenow,ALARM_TIME_INT &time1, ALARM_TIME_INT &time2);
	int time_detect(ALARM_DAY * Time,bool &status);
	void time_detect_by_num(uint8 num);

};


#endif
