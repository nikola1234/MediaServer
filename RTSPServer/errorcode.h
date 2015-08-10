/*
 * $Id: errorcode.h,v 2.0.0.0 2008/07/07 litianshun Exp $
 *
 * errorcode.h
 *
 * error code defination
 *
 * intervideo manager platform
 *
 * Copyright (c) 2006-2007
 *
 * The Initial Developer of the Original Code is qu licheng.
 *
 * Contributor(s): 
 *
 * $Log: errorcode.h,v $
 * Revision 2.0.0.0  2008/07/07 litianshun
 * 整理错误代码
 *
 * Revision 1.1.1.2  2007/08/03 02:54:26  jijing
 * 增加错误代码 ERROR_USER_DOMAIN——非本域用户
 *
 * Revision 1.1.1.1  2007/07/31 07:36:47  qulicheng
 * initial import
 */

#ifndef _ERROR_CODE_
#define _ERROR_CODE_

#ifdef __cplusplus
extern "C" {
#endif

#define ERROR_PROCESS_SUCCESS		0		//成功

#define ERROR_VER_TOKEN				-1		//不符合要求的版本或标识
#define ERROR_COMMAND_INVALID		-2		//无效的命令
#define ERROR_USER_LOGIN_ALREADY	-3		//该用户已登录
#define ERROR_USER_NOLOGINED		-4		//用户未登录
#define ERROR_USER_PASSWORD			-5		//用户密码错误

#define ERROR_CAMERA_END			-10		//摄像机列表传输结束
#define ERROR_CAMERA_INVALID		-11		//无效摄像机ID
#define ERROR_CAMERA_NOLEVEL		-12		//对不起，您无权调看此图像
#define ERROR_CODEC_FULL			-13		//无空闲通道
#define ERROR_CAMERA_DISLINE		-14		//摄像机不在线
#define ERROR_CAMERA_FAILURE		-15		//视频源连接失败
#define ERROR_CAMERA_ERR			-16		//传输摄像机列表错误

#define ERROR_CAMERA_NOOPEN			-20		//摄像机未打开
#define ERROR_CAMERA_NOCONTROL		-21		//摄像机不能控制
#define ERROR_CONTROL_NOLEVEL		-22		//无权控制此摄像机
#define	ERROR_CAMERA_REOPEN         -23     //提示客户端先关闭,后打开

#define ERROR_SWITCH_NOLEVEL		-31		//用户权限不是所有调看用户中最高,无法切换
#define ERROR_SWITCH_ERROR			-32		//切换失败
#define ERROR_NEWCAMERA_NOLEVEL		-33		//用户无权调看要切换的摄像机

#define ERROR_REPLAY_ERROR			-41		//摄像机无法回放
#define ERROR_REPLAY_NOLEVEL		-42		//用户无权回放


#define ERROR_SENTBL_FULL			-90		//发送表满

#define ERROR_SYSTEM_ERROR			-100	//系统错误

#define ERROR_NET_FAILED            -201    //网络故障
#define ERROR_OPEN_CAMERA           -202    //打开摄像机故障

#ifdef __cplusplus
}
#endif

#endif	//_ERROR_CODE_

