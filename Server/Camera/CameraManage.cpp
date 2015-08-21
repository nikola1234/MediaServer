#include "boost/filesystem.hpp"
#include "CameraManage.h"
#include "DataInfo.h"

ManageCamera::ManageCamera()
{
	boost::filesystem::create_directory("log");
	m_log.InitLog("./log/Cam-");
	m_log.Add("Manage Camera Start !");

}

ManageCamera::~ManageCamera()
{

}

int ManageCamera::read_CameraID_from_DB(char * path)
{
	int ret = 0;
	sqlite3* db = NULL;
	char sqlbuf[1024];
	char *ErrMsg=NULL;
	char **pszResult = NULL;
	
	int nRow = 0;
	int nColumn = 0;

	memset(sqlbuf,0,sizeof(sqlbuf));

	sqlite3_open(path,&db);
	sprintf(sqlbuf,"select CameraID from vnmp_CameraInfo;");
	ret = sqlite3_get_table(db, sqlbuf, &pszResult, &nRow, &nColumn, &ErrMsg);
	if (ret != SQLITE_OK)
	{
		 m_log.Add("%s(%d),SQL error! %s" ,DEBUGARGS,ErrMsg);
		sqlite3_free(ErrMsg);
		ErrMsg = NULL;
		sqlite3_free_table(pszResult);
		sqlite3_close(db);
		return -1;
	}
	else
	{
		int num = 0;
		for (int i = 0; i <= nRow; ++i)
		{
			for (int j = 0; j < nColumn; ++j)
			{
				int n= atoi(*(pszResult + nColumn * i + j));
				if( n != 0){
					CamIDList.push_back((uint32)n);
					num++;
				}
			}
		}
	}
	if (ErrMsg != NULL) sqlite3_free(ErrMsg);
	ErrMsg = NULL;
	sqlite3_free_table(pszResult);
	sqlite3_close(db);
	return 0;

}

void ManageCamera::erase_id_from_CamIDList(uint32 ID)
{
	std::list<uint32>::iterator Itor;
	WriteLock write_lock(m_CamIDListMutex_);
	for ( Itor = CamIDList.begin(); Itor != CamIDList.end(); )
	{
		if ( *Itor == ID )
		{
			Itor = CamIDList.erase(Itor);
		}
		else
		{
			Itor++;
		}
	}
}

void ManageCamera::fill_push_cam(DBCAMERACONFI *pt_caminfo,ST_VDCS_VIDEO_PUSH_CAM*pt_addCam)
{
	memcpy(pt_addCam->ip,pt_caminfo->ip,IP_LEN_16);
	memcpy(pt_addCam->CameUrL,pt_caminfo->CamUrl,SINGLE_URL_LEN_128);
	pt_addCam->frameRate =  pt_caminfo->frameRate;
	pt_addCam->CameraFunc = pt_caminfo->CameraFunc;
	pt_addCam->AnalyzeNUM = pt_caminfo->AnalyzeNUM;
	pt_addCam->AnalyzeType = pt_caminfo->AnalyzeType;
}

void ManageCamera::fill_var_param(DBCAMERAFUNCPARAM* pt_Camfuncparam,T_VDCS_VIDEO_CAMERA_PARAM*pt_varParam,vector <VIDEO_DRAW>   &PKG,uint8 num)
{
	uint8 i=0, j= 0;
	StrTimeDay timeday[WEEK_DAY_LEN_7];
	ALARM_DAY timeday_one[WEEK_DAY_LEN_7]; /* no use*/

	memcpy(pt_varParam->CameUrL , pt_Camfuncparam->CameraUrl,SINGLE_URL_LEN_128);
	pt_varParam->MaxHumanNum  =  pt_Camfuncparam->MaxHumanNum;
	pt_varParam->ChangRate        = pt_Camfuncparam->ChangeRate;

	if(num == 1)
	{
		pt_varParam->AnalyzeType  	=  pt_Camfuncparam->AnalyzeType1;
		pt_varParam->PkgNum    =pt_Camfuncparam->PkgNum1;
		CDataInfo.anlyzetime(pt_Camfuncparam->AlarmTime1,timeday,timeday_one);
		for(i= 0; i<WEEK_DAY_LEN_7;i++)
		{
			for(j= 0; j<3;j++)
			{	
				memcpy(pt_varParam->AlarmTime[i].alarmtime.Time[j].StartTime,timeday[i].Time[j].StartTime, 6);
				memcpy(pt_varParam->AlarmTime[i].alarmtime.Time[j].EndTime,timeday[i].Time[j].EndTime, 6);
			}
		}
		CDataInfo.DBToArea(pt_Camfuncparam->WatchRegion1,PKG);
	}

	if(num == 2)
	{
		pt_varParam->AnalyzeType  	=  pt_Camfuncparam->AnalyzeType2;
		pt_varParam->PkgNum    =pt_Camfuncparam->PkgNum2;
		CDataInfo.anlyzetime(pt_Camfuncparam->AlarmTime2,timeday,timeday_one);
		for(i= 0; i<WEEK_DAY_LEN_7;i++)
		{
			for(j= 0; j<3;j++)
			{	
				memcpy(pt_varParam->AlarmTime[i].alarmtime.Time[j].StartTime,timeday[i].Time[j].StartTime, 6);
				memcpy(pt_varParam->AlarmTime[i].alarmtime.Time[j].EndTime,timeday[i].Time[j].EndTime, 6);
			}
		}
		CDataInfo.DBToArea(pt_Camfuncparam->WatchRegion2,PKG);
	}

}

int ManageCamera::init_camera_from_DB()
{
	uint32 ID = 0;
	string rtspurl;
	uint8  num= 0;

	int iRet = -1;
	vector<int> camID;
	DBCAMERACONFI Caminfo;
	DBCAMERAFUNCPARAM Camfuncparam;

	ST_VDCS_VIDEO_PUSH_CAM  t_addCam;
	T_VDCS_VIDEO_CAMERA_PARAM t_varParam;
	vector <VIDEO_DRAW>   pkg;
	
	CDataInfo.getAllCameraConfigID(NULL,camID);

	if(camID.size() > 0){
		for(num = 0; num < camID.size() ; num++ )
		{	
			ID = (uint32)camID[num];
			CDataInfo.getCameraConfig(camID[num],&Caminfo); 
			iRet = try_to_open(Caminfo.ip,Caminfo.CamUrl);
			if(iRet < 0)
			{
				m_log.Add("%s(%d),DB have  camera  %s cant open!" ,DEBUGARGS ,Caminfo.ip);
				continue;
			}
			
			SingleCamPtr camptr = SingleCamPtr (new SingleCamera(this,ID));
			WriteLock writelock_(m_SinCamListMutex_);
			m_SinCamList.push_front(camptr);
			writelock_.unlock();
			fill_push_cam(&Caminfo,&t_addCam);
			camptr->set_camera_fix_param(t_addCam);	
		        iRet = CDataInfo.getCameraAlarmInfo(camID[num],&Camfuncparam);
			if(iRet < 0)
			{
				m_log.Add("%s(%d),DB get Camera no AlarmInfo  !" ,DEBUGARGS);
				continue;
			}
			
			if(Camfuncparam.AnalyzeNUM == 1)
			{
				pkg.clear();
				memset(&t_varParam , 0 ,sizeof(T_VDCS_VIDEO_CAMERA_PARAM));
				fill_var_param(&Camfuncparam,&t_varParam,pkg,1);				
				camptr->reset_camera_var_param_from_db(&t_varParam,pkg,1);
			}
			else if(Camfuncparam.AnalyzeNUM == 2)
			{
				pkg.clear();
				memset(&t_varParam , 0 ,sizeof(T_VDCS_VIDEO_CAMERA_PARAM));
				fill_var_param(&Camfuncparam,&t_varParam,pkg,1);
				camptr->reset_camera_var_param_from_db(&t_varParam,pkg,1);
				
				pkg.clear();
				memset(&t_varParam , 0 ,sizeof(T_VDCS_VIDEO_CAMERA_PARAM));
				fill_var_param(&Camfuncparam,&t_varParam,pkg,2);
				camptr->reset_camera_var_param_from_db(&t_varParam,pkg,2);
			}
			else
			{
				m_log.Add("%s(%d),DB have wrong  camera AnalyzeNUM %d !" ,DEBUGARGS,Camfuncparam.AnalyzeNUM);
			}
			
			erase_id_from_CamIDList(ID);
			m_CamList.add_cam_list(Caminfo.CamUrl, ID);	
		 }

		return 0;
	}
	m_log.Add("%s(%d),DB have no camera!" ,DEBUGARGS);

	return -1;
}

int ManageCamera::InitFromDB()
{
        char DB_file[20] ="MS.sqlite";
	read_CameraID_from_DB(DB_file);
	CDataInfo.sqlite3_create_db(DBNAME);	 
	CDataInfo.sqlite3_create_table(DBNAME);
	init_camera_from_DB();
  	return 0;
}

int ManageCamera::try_to_open(char * ip ,char* stream)
{
	int ret ;
	char sys[50] ={0};
	
	sprintf(sys,"%s %s","ping -c 1",ip);
	ret = system( sys);
	if(WEXITSTATUS(ret)  ==  0)
	{
		printf("ping oK!\n");
		return 0;
	}
	else
	{
		printf("ping failed!\n");
		m_log.Add("%s(%d),try to open failed!" ,DEBUGARGS);
		return -1;
	}

	VideoCapture  vcap;
	if(!vcap.open(stream)) {
		m_log.Add("%s(%d),try to open failed!" ,DEBUGARGS);
		return -1;
	}
	return 0;
}

uint32  ManageCamera::get_camera_id()
{	uint32 ID = 0;
	ReadLock read_lock (m_CamIDListMutex_);
	if(CamIDList.size() >0)
		ID = CamIDList.front();
	read_lock.unlock();
	WriteLock write_lock(m_CamIDListMutex_);
	CamIDList.pop_front();
	return ID;
}

void  ManageCamera::Get_Rest_Camlist()
{
	printf("Get_Rest_Camlist:");
	std::list<uint32>::iterator Itor;
	ReadLock read_lock(m_CamIDListMutex_);
	for ( Itor = CamIDList.begin(); Itor != CamIDList.end(); Itor++)
	{
		printf("%d ",*Itor);
	}
	printf("\n");
}

SingleCamPtr ManageCamera::search_camera_by_id(uint32 ID)
{
	SingleCamPtr tmpcamptr ;
	ReadLock readlock_(m_SinCamListMutex_);
	std::list<SingleCamPtr>::iterator it = m_SinCamList.begin();
	for ( ; it != m_SinCamList.end() ; it++ )
	{
		if ((*it)->GetCameraID() == ID)
		{
			tmpcamptr = *it;
			return tmpcamptr;
		}
	}
	return tmpcamptr;
}

SingleCamPtr ManageCamera::search_camera_by_url(char *url )
{
	uint32 ID= 0;
	SingleCamPtr tmpcamptr;
	m_CamList.search_cam_by_url(url, &ID);
	if(ID > 0)
	{
		tmpcamptr = search_camera_by_id(ID);
	}else{
		m_log.Add("%s(%d),get camer ptr fail!" ,DEBUGARGS);
	}
	return tmpcamptr;
}

int ManageCamera::remove_camera_by_id(uint32 ID)
{
	WriteLock writelock_(m_SinCamListMutex_);
	std::list<SingleCamPtr>::iterator it = m_SinCamList.begin();
	for ( ; it != m_SinCamList.end() ; it++ )
	{
		if ((*it)->GetCameraID() == ID)
		{
			m_SinCamList.erase(it);
			return 0;
		}
	}
	m_log.Add("%s(%d),remove camera by id  fail!" ,DEBUGARGS);
	return -1;
}

int ManageCamera::resume_cameraID_in_list(uint32 ID)
{
	int iRet = -1;
	iRet = m_CamList.delete_cam_by_id(ID);
	if(iRet < 0){
		m_log.Add("%s(%d),resume_cameraID_in_list  fail!" ,DEBUGARGS);
		return -1;
	}

	WriteLock writelock_(m_CamIDListMutex_);
	CamIDList.push_back(ID);
	return 0;
}

int ManageCamera::remove_camera_from_db_by_id(uint32 ID)
{
	int iRet = -1;
	iRet = CDataInfo.DelCameraConfig((int) ID);
	if(iRet != 0)
	{
		m_log.Add("%s(%d),DelCameraConfig  fail!" ,DEBUGARGS);
		return -1;
	}
	iRet = CDataInfo.DelCameraAlarmInfo((int) ID);
	if(iRet != 0)
	{
		m_log.Add("%s(%d),DelCameraAlarmInfo  fail!" ,DEBUGARGS);
		return -1;
	}
	return 0;
}

int  ManageCamera::reset_camera_param(uint32 ID,ST_VDCS_VIDEO_PUSH_CAM & addCam,string &url)
{
	SingleCamPtr camptr ;
	camptr = search_camera_by_id(ID);
	if(camptr == NULL)return -1;

	WriteLock writelock_(m_SinCamListMutex_);
	camptr->reset_camera_fix_param(addCam);
	writelock_.unlock();

	ReadLock readlock_(m_SinCamListMutex_);
	url =camptr->get_rtsp_url();
	if(url.length() == 0){
		m_log.Add("%s(%d),wrong rtsp url!" ,DEBUGARGS);
		return -1;
	}
	readlock_.unlock();
	return 0;
}
int ManageCamera::add_camID_list(char *url ,uint32 ID)
{
	int iRet =  m_CamList.add_cam_list(url,ID);
	return iRet;
}


int ManageCamera::parser_type(DBCAMERAFUNCPARAM*  pt_FuncParam,uint16 type)
{
	if(type & HumanDetect)  {
		pt_FuncParam->AnalyzeType1= HumanDetect;
		pt_FuncParam->AnalyzeType2=check_analyzetype(type&(uint16)(~HumanDetect));
		return 0;
	}
	if(type & SmokeDetect)  {
		pt_FuncParam->AnalyzeType1= SmokeDetect;
		pt_FuncParam->AnalyzeType2=check_analyzetype(type&(uint16)(~SmokeDetect));
		return 0;
	}
	if(type & RegionDetect)  {
		pt_FuncParam->AnalyzeType1 = RegionDetect;
		pt_FuncParam->AnalyzeType2=check_analyzetype(type&(uint16)(~RegionDetect));
		return 0;
	}
	if(type & FixedObjDetect)  {
		pt_FuncParam->AnalyzeType1 = FixedObjDetect;
		pt_FuncParam->AnalyzeType2=check_analyzetype(type&(uint16)(~FixedObjDetect));
		return 0;
	}
	if(type & FireDetect)  {
		pt_FuncParam->AnalyzeType1 = FireDetect;
		pt_FuncParam->AnalyzeType2=check_analyzetype(type&(uint16)(~FireDetect));
		return 0;
	}
	m_log.Add("%s(%d),parser_type failure!" ,DEBUGARGS);
	return -1;
}


int ManageCamera::set_camerafunc_DB(DBCAMERACONFI* pt_Caminfo)
{
	DBCAMERAFUNCPARAM t_FuncParam;

	t_FuncParam.CameraID   =   pt_Caminfo->CameraID;
	memcpy(t_FuncParam.ip ,pt_Caminfo->ip,IP_LEN_16);
	memcpy(t_FuncParam.CameraUrl,pt_Caminfo->CamUrl,SINGLE_URL_LEN_128);

	if(pt_Caminfo->AnalyzeNUM == 0){
		CDataInfo.AddCameraAlarmInfo(pt_Caminfo->CameraID,&t_FuncParam);
		return 0;
	}
	if(pt_Caminfo->AnalyzeNUM == 1)
	{
		t_FuncParam.AnalyzeNUM = 1;
		t_FuncParam.AnalyzeType1 = pt_Caminfo->AnalyzeType;
		CDataInfo.AddCameraAlarmInfo(pt_Caminfo->CameraID,&t_FuncParam);
		return 0;
	}
	if(pt_Caminfo->AnalyzeNUM == 2)
	{
		t_FuncParam.AnalyzeNUM = 2;
		parser_type(&t_FuncParam,pt_Caminfo->AnalyzeType);
		CDataInfo.AddCameraAlarmInfo(pt_Caminfo->CameraID,&t_FuncParam);
		return 0;
	}

	m_log.Add("%s(%d),set_camerafunc_DB failure!" ,DEBUGARGS);
	return -1;
}


void ManageCamera::parse_time_for_db(T_AlarmTime *pt_time ,char *time_c)
{
	char hour[3];
	char min[3];
	memset(hour, 0, 3);
	memset(min, 0, 3);

	memcpy(hour, time_c, 2);
	memcpy(min,  time_c+3, 2);

	pt_time->hour = atoi(hour);
	pt_time->min  = atoi(min);
}


int ManageCamera::Renew_camerafunc_DB(T_VDCS_VIDEO_CAMERA_PARAM* pt_CameraParam,vector <VIDEO_DRAW> & PKG,int ID)
{
	uint8  i= 0 ;
	uint8 j = 0;
	int iRet = -1;
	ALARM_DAY timeday[WEEK_DAY_LEN_7]; 
	StrTimeDay timeday_one[WEEK_DAY_LEN_7];/* no use*/
	
	DBCAMERAFUNCPARAM t_FuncParam;
	
	iRet = CDataInfo.getCameraAlarmInfo(ID,&t_FuncParam);
	if(iRet < 0)
	{
		m_log.Add("%s(%d),Renew_camerafunc_DB failure!" ,DEBUGARGS);
		return -1;		
	}
	if(pt_CameraParam->AnalyzeType == HumanDetect)
	{
		t_FuncParam.MaxHumanNum = pt_CameraParam->MaxHumanNum;
	}
	
	if(pt_CameraParam->AnalyzeType == RegionDetect)
	{
		t_FuncParam.ChangeRate = pt_CameraParam->ChangRate;
	}	
	
	if(t_FuncParam.AnalyzeType1 == pt_CameraParam->AnalyzeType)
	{
		t_FuncParam.PkgNum1  = pt_CameraParam->PkgNum;
		for(i= 0; i< WEEK_DAY_LEN_7; i++)
		{
			for(j = 0; j < 3; j ++)
			{
			    	parse_time_for_db(&(timeday[i].dayTime.time[j].Start),pt_CameraParam->AlarmTime[i].alarmtime.Time[j].StartTime);
			    	parse_time_for_db(&(timeday[i].dayTime.time[j].End),pt_CameraParam->AlarmTime[i].alarmtime.Time[j].EndTime);
			}
		}
		CDataInfo.TimeToDB(t_FuncParam.AlarmTime1,timeday_one,timeday,0);

		CDataInfo.AreaToDB( t_FuncParam.WatchRegion1, PKG);
	
		iRet= CDataInfo.setCameraAlarmInfo(ID,&t_FuncParam);

		return 0;
	}
	
	if(t_FuncParam.AnalyzeType2 == pt_CameraParam->AnalyzeType)
	{
		t_FuncParam.PkgNum2  = pt_CameraParam->PkgNum;
		for(i= 0; i< WEEK_DAY_LEN_7; i++)
		{
			for(j = 0; j < 3; j ++)
			{
				parse_time_for_db(&(timeday[i].dayTime.time[j].Start),pt_CameraParam->AlarmTime[i].alarmtime.Time[j].StartTime);
				parse_time_for_db(&(timeday[i].dayTime.time[j].End),pt_CameraParam->AlarmTime[i].alarmtime.Time[j].EndTime);
			}
		}
		CDataInfo.TimeToDB(t_FuncParam.AlarmTime2,timeday_one,timeday,0);
		CDataInfo.AreaToDB( t_FuncParam.WatchRegion2, PKG);
		CDataInfo.setCameraAlarmInfo(ID,&t_FuncParam);
		return 0;
	}
	m_log.Add("%s(%d),Renew_camerafunc_DB failure!" ,DEBUGARGS);
	return -1;	
}

int ManageCamera::Set_or_Renew_Camera_Param(T_VDCS_VIDEO_CAMERA_PARAM* pt_CameraParam,vector <VIDEO_DRAW> & Pkg)
{
	//int i = 0; 
	//int j = 0;
	int iRet = -1;
	uint32 ID= 0;
	/*
	printf("pt_CameraParam->camera is %s\n",pt_CameraParam->CameUrL);
	printf("pt_CameraParam->AnalyzeType is %x\n",pt_CameraParam->AnalyzeType);
	printf("pt_CameraParam->MaxHumanNum is %d\n",pt_CameraParam->MaxHumanNum);
	printf("pt_CameraParam->ChangRate is %f\n",pt_CameraParam->ChangRate);
	printf("pt_CameraParam->PkgNum is %d\n",pt_CameraParam->PkgNum);
	
	for(i = 0;i < 7; i++)
	{
		 for(j = 0; j < 3; j++)
		 {
		 	printf(" time %d  start time is %s ",j,pt_CameraParam->AlarmTime[i].alarmtime.Time[j].StartTime);
		 	printf(" time %d  End  time is %s ",j,pt_CameraParam->AlarmTime[i].alarmtime.Time[j].EndTime);
		 }
		 printf("\n");
	}	
	
	for(i= 0; i < Pkg.size() ; i ++)
	{
		VIDEO_DRAW tmp = Pkg[i];
		printf("StartX is %d ,StartY is %d ,EndX is %d ,EndY is %d ,Type is %d \n",tmp.StartX ,tmp.StartY,tmp.EndX,tmp.EndY,tmp.Type);
	}
	*/

	iRet = m_CamList.search_cam_by_url(pt_CameraParam->CameUrL,&ID);
	if(iRet == -1)
	{
		m_log.Add("%s(%d),get camer id failure!" ,DEBUGARGS);
		return -1;
	}

	SingleCamPtr camptr ;
	camptr = search_camera_by_id(ID);
	if(camptr == NULL)
	{
		m_log.Add("%s(%d),search camera by ID failure!" ,DEBUGARGS);
		return -1;
	}

	WriteLock writelock_(m_SinCamListMutex_);
	camptr->reset_camera_var_param(pt_CameraParam,Pkg);
	writelock_.unlock();


	DBCAMERAFUNCPARAM  t_FuncParam;
	iRet = CDataInfo.getCameraAlarmInfo((int)ID,&t_FuncParam);
	if(iRet < 0)
	{
		m_log.Add("%s(%d),search no camera info DB!" ,DEBUGARGS);
		return -1;
	}

	Renew_camerafunc_DB(pt_CameraParam,Pkg,(int)ID);

	return 0;
}

void ManageCamera::Add_Camerainfo_DB(ST_VDCS_VIDEO_PUSH_CAM* pt_addCam,string url,uint32 ID)
{
	DBCAMERACONFI Caminfo;
	
	Caminfo.CameraID  = (int)ID;
	Caminfo.frameRate  = pt_addCam->frameRate;
	Caminfo.CameraFunc = pt_addCam->CameraFunc;
	Caminfo.AnalyzeNUM  =  pt_addCam->AnalyzeNUM;
	Caminfo.AnalyzeType  = pt_addCam->AnalyzeType;

	memcpy(Caminfo.ip ,pt_addCam->ip,IP_LEN_16);
	memcpy(Caminfo.CamUrl,pt_addCam->CameUrL,SINGLE_URL_LEN_128);
	memcpy(Caminfo.RtspUrl,url.c_str(),url.length());

	CDataInfo.AddCameraConfig((int )ID, &Caminfo);
	set_camerafunc_DB(&Caminfo);
}

void ManageCamera::Renew_camerainfo_DB(ST_VDCS_VIDEO_PUSH_CAM* pt_addCam,string url,uint32 ID)
{
	DBCAMERACONFI Caminfo;
	
	Caminfo.CameraID  = (int)ID;
	Caminfo.frameRate  = pt_addCam->frameRate;
	Caminfo.CameraFunc = pt_addCam->CameraFunc;
	Caminfo.AnalyzeNUM  =  pt_addCam->AnalyzeNUM;
	Caminfo.AnalyzeType  = pt_addCam->AnalyzeType;

	memcpy(Caminfo.ip ,pt_addCam->ip,IP_LEN_16);
	memcpy(Caminfo.CamUrl,pt_addCam->CameUrL,SINGLE_URL_LEN_128);
	memcpy(Caminfo.RtspUrl,url.c_str(),url.length());

	CDataInfo.DelCameraConfig((int)ID);
	CDataInfo.AddCameraConfig((int)ID, &Caminfo);
	CDataInfo.DelCameraAlarmInfo((int )ID);
	set_camerafunc_DB(&Caminfo);
}

string ManageCamera::Create_or_Renew_Camera(ST_VDCS_VIDEO_PUSH_CAM & addCam)
{
	int iRet = -1;
	uint32 ID = 0;
	string url = "TT";

	iRet = m_CamList.search_cam_by_url(addCam.CameUrL,&ID);
	if(iRet == -1){ /* have no camera */
		m_log.Add("%s(%d),have no this camera %s!" ,DEBUGARGS,addCam.ip);
		ID = get_camera_id();
		if(ID == 0){
			m_log.Add("%s(%d),get camer id failure!" ,DEBUGARGS);
			return url;
		}
		m_log.Add("%s(%d),get camera id %d!" ,DEBUGARGS,ID);
		SingleCamPtr camptr = SingleCamPtr(new SingleCamera(this,ID));
		WriteLock writelock_(m_SinCamListMutex_);
		m_SinCamList.push_front(camptr);
		writelock_.unlock();
		camptr->set_camera_fix_param(addCam);
		url =camptr->get_rtsp_url();
		if(url.length() == 0){
			m_log.Add("%s(%d),wrong rtsp url!" ,DEBUGARGS);
			return url;
		}
		iRet =add_camID_list(addCam.CameUrL,ID);
		if(iRet < 0){
			string tmpurl ="TT";
			m_log.Add("%s(%d),add_camID_list failure!" ,DEBUGARGS);
			return tmpurl;
		}
		Renew_camerainfo_DB(&addCam,url,(int)ID);
		return url;
	}

	if(iRet == 0){ /* have exist camera */
		if(ID <= 0){
			m_log.Add("push exist camera %s and ID= %d return failure !" ,addCam.CameUrL,ID);
			return url;
		}
		reset_camera_param(ID,addCam,url); /*stop analyze*/
		Renew_camerainfo_DB(&addCam,url,ID);
		return url;
	}

	return url;
}
