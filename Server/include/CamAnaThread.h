#ifndef _CAM_ANA_THREAD_H_
#define _CAM_ANA_THREAD_H_

#include "Common.h"
#include "CmdDefine.h"

#include "region/region.h"
#include "fire/VideoHandler.h"
#include "fire/fire.h"
#include "smoke/smoke.h"
#include "human/human.h"

#include "SingleCamera.h"

#include "NetServer.h"

#define START_FRAME_INDEX     8
#define INTERVAL_FRAME_INDEX  400
#define STOP_FRAME_INDEX      40

enum warnevent
{
    alarmOn   = 0x01,
    alarmStop = 0x02,
};

class SingleCamera;
class NetServer;
class CamAnaThread
{
public:
	CamAnaThread(SingleCamera* sincam,NetServer *ser);
	~CamAnaThread();


	bool  m_AnaFlag;
	bool  m_Status;
	bool AnalyzeEn;

	Mat origFrame;

	uint8 AnaIndex;
	uint16 AnalyzeType;
	uint8  alarm;

	uint32 frame;
	uint32 alarmStartframe;
	uint32 alarmStopframe;
	uint8  startflag;
	uint8  stopflag;
	uint8  intervalflag;

	CRegion* region;
	VideoHandler* videoHandler;
	CFire*   fire;
	CSmoke*  smoke;
	
	CHuman*  human;
	T_HUMANNUM t_HumanNum;

	vector <VIDEO_DRAW> pkg;


	void set_video_draw( vector <VIDEO_DRAW> &DrawPkg);
	int alarmStrategy();

	int send_alarm_to_mcu(uint16 type,uint8 status);
	int send_alarm_to_client(uint16 type,uint8 status);

	int human_detect(Mat &frame);
	int region_detect(Mat &frame);
	int fire_detect(Mat &frame);
	int smoke_detect(Mat &frame);

	int alarm_run(Mat &frame ,uint8 iType);
	
	void analyze_pause_release();
	void analyze_sleep_release();
	
	int parse_human_draw_package(vector <VIDEO_DRAW> & Pkg,vector<Rect>& tmprect,vector<Line>  &tmpline);
	int parse_draw_package(vector <VIDEO_DRAW> & Pkg,vector<Rect>& tmprect);
	void set_analyze_vector( vector <VIDEO_DRAW> & DrawPkg);



	void SetCamera_StartThread(uint16 type,uint8 num);

	int check_thread_status();
	void run();
	int  CreateAnaThread();
	static void* RunAnaThread(void*  param){
	CamAnaThread* p = (CamAnaThread*)param;
	p->run();
	return NULL;
	}
	void quit(){ m_AnaFlag = false;}
	void resume();
	void pause();

	SingleCamera *cam;
	NetServer *server;

private:
	uint32 CameraID;

	pthread_mutex_t mut;
	pthread_cond_t cond;
};

#endif
