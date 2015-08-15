#ifndef _CMDDEFINE_H_
#define	_CMDDEFINE_H_

#include "Common.h"

#pragma pack(push)
#pragma pack(1)

#define T_PACKETHEAD_MAGIC	0xfefefefe

#define  IP_LEN_16        16

#define  TIME_NUM_3        3
#define  WEEK_DAY_LEN_7   7

#define  CAM_MAX_LEN   20
#define  SINGLE_URL_LEN_128  128
#define  PACKET_HEAD_LEN  28

typedef struct COMMON_PACKET_HEAD
{
	uint32	magic;					   
	uint16  	encrypt;				  
	uint16	cmd;					     
	uint32  	EncryptLen;				
	uint32  	UnEncryptLen;			
	uint32	CompressedLen;		 
	uint32	UnCompressedLen;	 
	uint16	chksum;					   
	uint16	unused;	

	COMMON_PACKET_HEAD(){
		memset(this, 0, sizeof(COMMON_PACKET_HEAD));
	}
} T_PacketHead,*PT_PacketHead;


/*********************city Server interaction*******************************/

enum ANAY_VDCS_CMD {  				/* city Server interaction CMD */
	SM_ANAY_VDCS_REGISTER = 0x0400,
	SM_VDCS_ANAY_REGISTER_ACK,

	SM_ANAY_VDCS_DEVICE_STATUS ,
	SM_VDCS_ANAY_DEVICE_STATUS_ACK,

	SM_ANAY_VDCS_WARN_INFO,
	SM_VDCS_ANAY_WARN_INFO_ACK,

	SM_VDCS_ANAY_PUSH_CAMERA,
	SM_VDCS_ANAY_PUSH_CAMERA_ACK,
	
	SM_VDCS_ANAY_PUSH_CAMERA_PARAM,
	SM_VDCS_ANAY_PUSH_CAMERA_PARAM_ACK,
		
	SM_VDCS_ANAY_DELETE_CAMERA,
	SM_VDCS_ANAY_DELETE_CAMERA_ACK,	
	

	SM_ANAY_HEATBEAT = 0X8003
};

enum DeviceType {
	DeviceTypeNetCamera = 1,	//IPCAMERA
	DeviceTypeWarn,
	DeviceTypeLock,
	DeviceTypeGuard,
	DeviceTypeSpeak,
	DeviceTypeWork,
	DeviceTypeYA,
	DeviceTypeMcu,
	DeviceTypeNetControl,
	DeviceTypeRecorder,
};

#define AnalyzeNumber    6

enum AnalyzeType
{
	HumanDetect 	=	0x0001,
	SmokeDetect 	=	0x0002,
	RegionDetect 	= 	0x0004,
	FixedObjDetect	= 	0x0008,
	FireDetect 		= 	0x0010,
	ResidueDetect	 =	0x0020,
};
/***************************************************send******************************************************/
//SM_VDCS_ANAY_REGISTER_ACK
typedef struct _ANAY_VDCS_REGISTER_ACK
{
	uint32 ServerID;
	char   Serverip[IP_LEN_16];

	_ANAY_VDCS_REGISTER_ACK(){
		memset(this, 0, sizeof(_ANAY_VDCS_REGISTER_ACK));
	}
} T_ANAY_VDCS_REGISTER_ACK;

//SM_VDCS_ANAY_PUSH_CAMERA_ACK
typedef struct _ANAY_VDCS_PUSH_CAM_ACK
{
	uint8        ack; // 1 success  0 failure
	char		ip[IP_LEN_16];
	char		CameUrL[SINGLE_URL_LEN_128];
	char         RtspUrL[SINGLE_URL_LEN_128];

	_ANAY_VDCS_PUSH_CAM_ACK(){
		memset(this, 0, sizeof(_ANAY_VDCS_PUSH_CAM_ACK));
	}
}T_ANAY_VDCS_PUSH_CAM_ACK;

//SM_VDCS_ANAY_PUSH_CAMERA_PARAM_ACK
typedef struct _ANAY_VDCS_PUSH_CAM_PARAM_ACK
{
	uint8        ack; // 1 success  0 failure
	
	_ANAY_VDCS_PUSH_CAM_PARAM_ACK(){
		memset(this, 0, sizeof(_ANAY_VDCS_PUSH_CAM_PARAM_ACK));
	}
}T_ANAY_VDCS_PUSH_CAM_PARAM_ACK;


/***************************************************receive******************************************************/
//SM_VDCS_ANAY_PUSH_CAMERA
typedef struct  _VDCS_VIDEO_PUSH_CAM
{
	char		ip[IP_LEN_16];
	char		CameUrL[SINGLE_URL_LEN_128];

	uint8		Enable;
	uint8		frameRate;
	uint8		CameraFunc;  /* 1 take photo 2 analyze*/
	uint8		AnalyzeNUM; 
	uint16      AnalyzeType; 

	_VDCS_VIDEO_PUSH_CAM(){
	memset(this, 0, sizeof(_VDCS_VIDEO_PUSH_CAM));
	}
}ST_VDCS_VIDEO_PUSH_CAM;


typedef struct _ALARM_TIME{

	char   StartTime[6];
	char   EndTime[6];

	_ALARM_TIME(){
		memset(this, 0, sizeof(_ALARM_TIME));
	}
}ALARM_TIME;

typedef struct _VIDEO_ALARM_TIME{

	ALARM_TIME Time[TIME_NUM_3];

	_VIDEO_ALARM_TIME(){
		memset(this, 0, sizeof(_VIDEO_ALARM_TIME));
	}
}VIDEO_ALARM_TIME;

typedef struct _VDCS_VIDEO_ALARM_TIME{

	VIDEO_ALARM_TIME AlarmTime;
	
	_VDCS_VIDEO_ALARM_TIME(){
		memset(this, 0, sizeof(_VDCS_VIDEO_ALARM_TIME));
	}
}T_VDCS_VIDEO_ALARM_TIME;

typedef struct _VIDEO_DRAW  
{
	uint16 	StartX;				//StartX
	uint16 	StartY;				//StartY
	uint16 	EndX;				//EndX    if draw is rectangle  means width
	uint16 	EndY;				//EndY    if draw is rectangle  means height
	uint16  	Type;				// 1 rectangle 2 line

	_VIDEO_DRAW(){
		memset(this, 0, sizeof(_VIDEO_DRAW));
	}
}VIDEO_DRAW;


//SM_VDCS_ANAY_PUSH_CAMERA_PARAM
typedef struct  _VDCS_VIDEO_CAMERA_PARAM
{
	char 	CameUrL[SINGLE_URL_LEN_128];
	uint16    	AnalyzeType;         /*single analyze*/
	
	uint16      MaxHumanNum;    /* HumanDetect needs */
	float         ChangRate;           /* RegionDetect needs */
	uint16      PkgNum;                /* structure refer to VIDEO_DRAW */
	
	T_VDCS_VIDEO_ALARM_TIME  AlarmTime[WEEK_DAY_LEN_7];
	
	_VDCS_VIDEO_CAMERA_PARAM(){
		memset(this, 0, sizeof(_VDCS_VIDEO_CAMERA_PARAM));
	}
}T_VDCS_VIDEO_CAMERA_PARAM;


//SM_VDCS_ANAY_DELETE_CAMERA
typedef struct _VDCS_VIDEO_CAMERA_DELETE
{
	char		ip[IP_LEN_16];
	char 	CameUrL[SINGLE_URL_LEN_128];
	
	_VDCS_VIDEO_CAMERA_DELETE(){
		memset(this, 0, sizeof(_VDCS_VIDEO_CAMERA_DELETE));
	}
	
}T_VDCS_VIDEO_CAMERA_DELETE;

#pragma pack(pop)

#endif
