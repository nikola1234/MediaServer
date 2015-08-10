#pragma once


#ifdef	WIN32
#define  MSDLL	_declspec(dllexport) 
#else
#define  MSDLL	
#endif

#include "ICamera.h"

MSDLL int __stdcall StartRTSPServer(ICamera* pICamera);

MSDLL void __stdcall StopRTSPServer();

////////////////////////////扩展接口///////////////////////////////////////////////////////
//发送错误信息到客户端
MSDLL int  __stdcall SendCameraError(int user,const char* chErrorInfo);
//代理转发时切换更新
MSDLL int  __stdcall UpdateSwitchCameraInfo(int oldCameraID, int iNewCameraID);
//设置其他扩展接口
MSDLL void __stdcall SetIDevice(IDevice* pIDevice);
////////////////////////////////////////////////////////////////////////////////////////////

#define ERROR_LOADMSDB_FAILED              -101          //加载MS数据库失败
#define ERROR_RTSPPORT_CONFIG_FAILED       -102          //配置rtsp端口 错误
#define ERROR_STARTLISTEN_RTSPPORT_FAILED  -103			 //监听rtsp端口失败
#define ERROR_VIDEOPATH_CONFIG_FAILED      -104          //录像路径 设置错误
#define ERROR_MEDIASAVE_INIT_FAILED        -105          //录像模块启动失败
#define ERROR_LOADMRDB_FAILED			   -106          //加载存储数据库失败


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
#define CAMERA_COMMAND_MONITOR_CAMERA_SELECT 43 //摄像机切换中MONITOR_CAMERA的选择

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
#define PLAY_BACK_GETTIME 		        208//获取当前回放时间