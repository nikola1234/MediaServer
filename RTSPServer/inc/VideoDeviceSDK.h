#ifndef _VNMP_NET_SDK_2014_08_05
#define _VNMP_NET_SDK_2014_08_05

#ifdef	WIN32
#define  VIDDEVSDK	_declspec(dllexport) 
#else
#define  VIDDEVSDK	
#endif

#ifndef WIN32
#ifndef __stdcall
#define __stdcall __attribute__((__stdcall__))
#endif
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

/************************************************************************/
/*				视频设备数据结构				                        */
/************************************************************************/
typedef struct tagVIDEODEVICE
{
	//设备登录信息（用户提供给SDK的信息）
	char		ip[64];			/*设备IP地址*/
	int			port;			/*设备端口号*/
	char		username[64];	/*用户登录名称*/
	char		password[64];	/*用户登录口令*/
	
	//用来标识视频信息
	char		channel[256];		/*视频源对应的通道号*/
	
	//供SDK内部使用(存储登录情况、调看情况)
	int			handle;			/*设备句柄*/
	int			devnum;			/*设备编号*/
	
	//保留字段，用于扩展功能(内部使用，或协商使用)
	int			reuse;			/*复用字段*/
	char		reserve[256];	/*保留字段*/
}VIDEODEVICE;

/************************************************************************/
/*					回调函数                                            */
/************************************************************************/

//视频图像缓冲区编码数据的获取
typedef void (__stdcall *VIDEO_CAPTURE_CALLBACK)(unsigned long dwDataType, unsigned long bufsize, unsigned char *buffer, unsigned long user);

typedef bool (__stdcall *VIDEO_CAPTURE_CALLBACKEx)(unsigned long dwDataType, unsigned long bufsize, unsigned char *buffer,  void* user);

//切换成功后，新的视频摄像机的域名信息回调函数原型
typedef void (__stdcall *VIDEO_SWITCH_CAMERA_UPDATE_CALLBACK)(unsigned long dwOldCameraID,unsigned long dwNewCameraID, unsigned char *NewCameraName, unsigned long user);

/********************预览回调函数*********************/
#define NET_DVR_SYSHEAD         1   //系统头数据
#define NET_DVR_STREAMDATA      2   //视频流数据（包括复合流和音视频分开的视频流数据）
#define NET_DVR_AUDIOSTREAMDATA 3   //音频流数据
#define NET_DVR_STD_VIDEODATA   4   //标准视频流数据
#define NET_DVR_STD_AUDIODATA   5   //标准音频流数据

/*************码流传输协议定义 ..capturing_start VIDEODEVICE.reuse********/
#define VNMP_TRANSPORT_UDP		0x1000   //RTP over UDP      0x1000
#define VNMP_TRANSPORT_TCP		0x1001   //RTP over TCP	     0x1001
#define VNMP_TRANSPORT_RAW		0x1002   //RAW+12 over TCP	 0x1002 //默认

/************************************************************************/
/*					          错误码定义                                */
/************************************************************************/
#define ERROR_PROCESS_SUCCESS		0		//成功

#define ERROR_CAMERA_INVALID		-11		//无效摄像机ID
#define ERROR_CAMERA_NOLEVEL		-12		//对不起，您无权调看此图像
#define ERROR_CODEC_FULL			-13		//无空闲通道
#define ERROR_CAMERA_DISLINE		-14		//摄像机不在线
#define ERROR_CAMERA_FAILURE		-15		//视频源连接失败

#define ERROR_CAMERA_NOOPEN			-20		//摄像机未打开
#define ERROR_CAMERA_NOCONTROL		-21		//摄像机不能控制
#define ERROR_CONTROL_NOLEVEL		-22		//无权控制此摄像机
#define ERROR_SWITCH_REOPEN			-23		//切换失败，需重新打开
#define ERROR_SWITCH_REOPEN_FIALED	-24		//切换失败
#define ERROR_SWITCH_NOLEVEL		-31		//用户权限不是所有调看用户中最高,无法切换
#define ERROR_SWITCH_ERROR			-32		//切换打开失败
#define ERROR_NEWCAMERA_NOLEVEL		-33		//用户无权调看要切换的摄像机

#define ERROR_REPLAY_ERROR			-41		//摄像机无法回放
#define ERROR_REPLAY_NOLEVEL		-42		//用户无权回放

#define ERROR_SYSTEM_ERROR			-100	//系统错误

#define ERROR_SOCKET_CRAETE			-50		//SOCKET创建失败,本地资源不足,可能是客户端过载
#define ERROR_SOCKET_SEND			-51		//发送请求失败，与服务器之间的网络故障
#define ERROR_SOCKET_RECV			-52		//没有响应请求，与服务器之间的网络故障
#define ERROR_SOCKET_CONNECT		-53		//网络连接失败，与服务器之间的网络故障
#define ERROR_SOCKET_OVERTIME		-54		//接收响应超时，与服务器之间的网络故障

#define ERROR_GIVEUP_SELECT			-61		//放弃选择占用摄像机的打开	
#define ERROR_PARAMETER_INVALID		-62		//参数错误	
#define ERROR_USER_OVERNUM			-65		//超过dll支持的最大用户连接数

#define ERROR_NET_FAILED            -201    //网络故障
#define ERROR_OPEN_CAMERA           -202    //打开失败

/************************************************************************/
/*					接口的初始化与释放                                  */
/************************************************************************/
VIDDEVSDK int __stdcall  video_device_init();
VIDDEVSDK int __stdcall  video_device_destroy();

/************************************************************************/
/*					流媒体服务器登录、注销控制                          */
/************************************************************************/
VIDDEVSDK int __stdcall video_device_login(VIDEODEVICE *videodev);
VIDDEVSDK int __stdcall video_device_logout(VIDEODEVICE *videodev);

/************************************************************************/
/*					切换时摄像机名称更新回调函数注册			        */
/************************************************************************/
VIDDEVSDK void __stdcall video_device_set_camera_switch_update(VIDEODEVICE *videodev,
															   VIDEO_SWITCH_CAMERA_UPDATE_CALLBACK camera_switch_update_callback, unsigned long user);

/************************************************************************/
/*					视频传输开始、停止控制				                */
/************************************************************************/
VIDDEVSDK int __stdcall video_device_capturing_start(VIDEODEVICE *videodev,
													 unsigned long user, VIDEO_CAPTURE_CALLBACK capture_callback);

VIDDEVSDK int __stdcall video_device_capturing_startEx(VIDEODEVICE *videodev, void* user, VIDEO_CAPTURE_CALLBACKEx capture_callback);

VIDDEVSDK int __stdcall video_device_capturing_stop(VIDEODEVICE *videodev);

/************************************************************************/
/*					摄像机云台控制的支持								*/
/************************************************************************/
//云台控制相关参数
#define CAMERA_COMMAND_STOP				0	    //停止
#define CAMERA_COMMAND_UP				1	    //云台上
#define CAMERA_COMMAND_UPSTOP           10001	//云台上停止
#define CAMERA_COMMAND_DOWN				2		//云台下
#define CAMERA_COMMAND_DOWNSTOP		    10002   //云台下停止
#define CAMERA_COMMAND_LEFT				3		//云台左
#define CAMERA_COMMAND_LEFTSTOP		    10003	//云台左停止
#define CAMERA_COMMAND_RIGHT			4		//云台右
#define CAMERA_COMMAND_RIGHTSTOP	    10004	//云台右停止

#define CAMERA_COMMAND_ZOOM_IN			5		//镜头近
#define CAMERA_COMMAND_ZOOM_INSTOP	    10005	//镜头近停止
#define CAMERA_COMMAND_ZOOM_OUT			6		//镜头远
#define CAMERA_COMMAND_ZOOM_OUTSTOP	    10006	//镜头远停止

#define CAMERA_COMMAND_FOCUS_NEAR		7		//焦距近
#define CAMERA_COMMAND_FOCUS_NEARSTOP	10007	//焦距近停止
#define CAMERA_COMMAND_FOCUS_FAR		8		//焦距远
#define CAMERA_COMMAND_FOCUS_FARSTOP	10008	//焦距远停止

#define CAMERA_COMMAND_LIGHT_CLOSE		9		//光圈合
#define CAMERA_COMMAND_LIGHT_CLOSESTOP	10009	//光圈合停止
#define CAMERA_COMMAND_LIGHT_OPEN		10		//光圈开
#define CAMERA_COMMAND_LIGHT_OPENSTOP	100010	//光圈开停止

#define CAMERA_COMMAND_BRUSH_CLOSE		11		//雨刷合
#define CAMERA_COMMAND_BRUSH_OPEN		12		//雨刷开
#define CAMERA_COMMAND_RAY_CLOSE		13		//灯光合
#define CAMERA_COMMAND_RAY_OPEN		    14		//灯光开

#define CAMERA_COMMAND_SET_PRESET		15		//预制位设置
#define CAMERA_COMMAND_GOTO_PRESET		16		//预制位调用
#define CAMERA_COMMAND_DEL_PRESET		17		//预制位删除

#define CAMERA_COMMAND_CAMERA_SELECT	21		//摄像机选择
#define CAMERA_COMMAND_CAMERA_LOCK		22		//摄像机锁定
#define CAMERA_COMMAND_CAMERA_UNLOCK	23		//摄像机解锁

//#define CAMERA_COMMAND_MONITOR_SELECT	31		//监视器选择
//#define CAMERA_COMMAND_MONITOR_CAMERA_SELECT 43 //摄像机切换中MONITOR_CAMERA的选择

#define CAMERA_COMMAND_UP_LEFT			51	    //云台左上
#define CAMERA_COMMAND_UP_LEFTSTOP      10051	//云台左上停止
#define CAMERA_COMMAND_UP_RIGHT			52	    //云台右上
#define CAMERA_COMMAND_UP_RIGHTSTOP     10052	//云台右上停止
#define CAMERA_COMMAND_DOWN_LEFT		53		//云台左下
#define CAMERA_COMMAND_DOWN_LEFTSTOP	10053   //云台左下停止
#define CAMERA_COMMAND_DOWN_RIGHT		54		//云台右下
#define CAMERA_COMMAND_DOWN_RIGHTSTOP	10054   //云台右下停止

//巡航命令时param1为续航编号 
//巡航命令=61 当将预置点加入巡航序列时param2定义为：
//第一个字节：预置点
//第二个字节：停留时间
//第三个字节：速度
#define CAMERA_COMMAND_CRUISE_ADD	    61		//将预置点加入巡航序列
#define CAMERA_COMMAND_CRUISE_DEL		62		//将预置点从巡航序列中删除
#define CAMERA_COMMAND_CRUISE_CALL	    63		//开始巡航
#define CAMERA_COMMAND_CRUISE_CALLSTOP	64		//停止巡航

//ControlSpeed 范围（1-7） ControlType 为以上云台命令
VIDDEVSDK int __stdcall video_device_cameracontrol(VIDEODEVICE *videodev, int ControlType, int ControlSpeed);


/************************************************************************/
/*							视频回放相关								*/
/************************************************************************/

//数据结构
typedef struct{
	unsigned long dwYear;		//年
	unsigned long dwMonth;		//月
	unsigned long dwDay;		//日
	unsigned long dwHour;		//时
	unsigned long dwMinute;		//分
	unsigned long dwSecond;		//秒
}REPALY_TIME, *LPREPLAY_TIME;

//时间间隔
typedef struct tagTimeSlice{
	REPALY_TIME sBeginTime;
	REPALY_TIME sEndTime;
}TIME_SLICE;

//视频图像缓冲区编码数据的获取 其中回放是dwParam为时间单位为秒 /*下载是dwParam为下载的文件大小单位为字节*/
typedef void (__stdcall *VIDEO_PLAYBACK_CALLBACK)(unsigned long dwParam,unsigned long dwDataType, unsigned long bufsize,
	unsigned char *buffer, unsigned long user);

//查询指定时间录像
VIDDEVSDK int __stdcall video_device_FindPlayBackByTime(
	VIDEODEVICE *videodev, 
	IN TIME_SLICE* pRecordTimeSlice,  /*录像文件信息*/
	char* pPlayBackInfo,
	unsigned long PlayBackInfoSize);

//回放指定时间录像
VIDDEVSDK int __stdcall video_device_PlayBackByTime(
	VIDEODEVICE *videodev, 
	IN TIME_SLICE* pRecordTimeSlice,  /*录像文件信息*/
	VIDEO_PLAYBACK_CALLBACK playback,       
	unsigned long dwUser);

//下载指定时间录像
VIDDEVSDK int __stdcall video_device_DownloadByTime(
	VIDEODEVICE *videodev, 
	IN TIME_SLICE* pRecordTimeSlice,  /*录像文件信息*/
	VIDEO_PLAYBACK_CALLBACK playback,       
	unsigned long dwUser);

//回放控制相关参数
#define PLAY_BACK_START 		1//开始播放
#define PLAY_BACK_STOP			2//停止播放
#define PLAY_BACK_PAUSE 		3//暂停播放
#define PLAY_BACK_RESTART		4//恢复播放
#define PLAY_BACK_FAST 			5//快放
#define PLAY_BACK_SLOW 			6//慢放
#define PLAY_BACK_SKIPTIME 		7//选择时间

//回放控制
VIDDEVSDK int __stdcall video_device_PlayBackControl(
	VIDEODEVICE *videodev,
	unsigned long dwControlCode,          /*控制命令*/
	unsigned long dwParam);               /*控制参数*/

//停止回放
VIDDEVSDK int __stdcall video_device_StopPlayBack(VIDEODEVICE *videodev);

//停止下载
VIDDEVSDK int __stdcall video_device_StopDownload(VIDEODEVICE *videodev);
#endif //_VNMP_NET_SDK_2014_08_05
