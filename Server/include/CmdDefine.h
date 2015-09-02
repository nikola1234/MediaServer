#ifndef _CMDDEFINE_H_
#define	_CMDDEFINE_H_

#include "Common.h"

#pragma pack(push)
#pragma pack(1)

#define T_PACKETHEAD_MAGIC	0xfefefefe

#define  IP_LEN_16        16

#define  TIME_NUM_3        3
#define  WEEK_DAY_LEN_7   7

#define  CAM_MAX_LEN           20
#define  SINGLE_URL_LEN_128    128
#define  PACKET_HEAD_LEN       28
#define  MCU_MAC_LEN_20        20
#define  PIC_BUFFER_LEN  	   640*360*3

typedef struct COMMON_PACKET_HEAD
{
	uint32	magic;
	uint16  encrypt;
	uint16	cmd;
	uint32  EncryptLen;
	uint32  UnEncryptLen;
	uint32	CompressedLen;
	uint32	UnCompressedLen;
	uint16	chksum;
	uint16	unused;

	COMMON_PACKET_HEAD(){
		memset(this, 0, sizeof(COMMON_PACKET_HEAD));
	}
} T_PacketHead,*PT_PacketHead;

enum ClientType
{
	client_config = 0x00,
	Client_Mcu,
	client_APP
};


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
	HumanDetect 		=	0x0001,
	SmokeDetect 		=	0x0002,
	RegionDetect 		= 0x0004,
	FixedObjDetect	= 0x0008,
	FireDetect 			= 0x0010,
	ResidueDetect	 	=	0x0020,
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
	char		ip[IP_LEN_16];
	char		CameUrL[SINGLE_URL_LEN_128];
	char    RtspUrL[SINGLE_URL_LEN_128];
	uint8   ack; 			/* 1 success  0 failure*/
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

#define  DOOR_NUM_2  2
typedef struct _DOOR_
{
	uint32 in;
	uint32 out;

	_DOOR_(){
		memset(this, 0, sizeof(_DOOR_));
	}
}T_DOOR;

//SM_ANAY_VDCS_WARN_INFO
typedef struct _SM_ANAY_VDCS_WARN_INFO
{
	char   	CameUrl[SINGLE_URL_LEN_128];
	uint8 	WarnType;
	uint8        Status;       	/* 1 start / 0 stop */
	uint32      numALL;
	T_DOOR Door[DOOR_NUM_2];

	_SM_ANAY_VDCS_WARN_INFO(){
		memset(this, 0, sizeof(_SM_ANAY_VDCS_WARN_INFO));
	}
} T_SM_ANAY_VDCS_WARN_INFO;

//SM_ANAY_VDCS_DEVICE_STATUS
typedef struct _SM_ANAY_VDCS_DEVICE_STATUS
{
	char   	CameUrl[SINGLE_URL_LEN_128];
	uint8 	DeviceType;
	uint8 	Status;  /* 1 break 0 nomal */

	_SM_ANAY_VDCS_DEVICE_STATUS(){
		memset(this, 0, sizeof(_SM_ANAY_VDCS_DEVICE_STATUS));
	}
}T_SM_ANAY_VDCS_DEVICE_STATUS;

//SM_VDCS_ANAY_DELETE_CAMERA_ACK
typedef struct _VDCS_VIDEO_CAMERA_DELETE_ACK
{
	char         ip[IP_LEN_16];
	char   	CameUrl[SINGLE_URL_LEN_128];
	uint8 	Ack;

	_VDCS_VIDEO_CAMERA_DELETE_ACK(){
		memset(this, 0, sizeof(_VDCS_VIDEO_CAMERA_DELETE_ACK));
	}
}T_VDCS_VIDEO_CAMERA_DELETE_ACK;


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

	VIDEO_ALARM_TIME alarmtime;

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
	uint16  Type;				// 1 rectangle 2 line

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

//SM_VDCS_ANAY_DEVICE_STATUS_ACK
typedef struct _VDCS_ANAY_DEVICE_STATUS_ACK
{
	char   	CameUrl[SINGLE_URL_LEN_128];
	uint8 	DeviceType;
	uint8        Ack;  	/* 1 sucess 0 fail */

	_VDCS_ANAY_DEVICE_STATUS_ACK(){
		memset(this, 0, sizeof(_VDCS_ANAY_DEVICE_STATUS_ACK));
	}
}T_VDCS_ANAY_DEVICE_STATUS_ACK;


//SM_VDCS_ANAY_WARN_INFO_ACK
typedef struct _VDCS_ANAY_WARN_INFO_ACK
{
	char 	CameUrL[SINGLE_URL_LEN_128];
	uint16    	AnalyzeType;         /*single analyze*/
	uint8        Ack;				/* 1 sucess 0 fail */

	_VDCS_ANAY_WARN_INFO_ACK(){
		memset(this, 0, sizeof(_VDCS_ANAY_WARN_INFO_ACK));
	}
}T_VDCS_ANAY_WARN_INFO_ACK;


//SM_VDCS_ANAY_DELETE_CAMERA
typedef struct _VDCS_VIDEO_CAMERA_DELETE
{
	char		ip[IP_LEN_16];
	char 	CameUrL[SINGLE_URL_LEN_128];

	_VDCS_VIDEO_CAMERA_DELETE(){
		memset(this, 0, sizeof(_VDCS_VIDEO_CAMERA_DELETE));
	}

}T_VDCS_VIDEO_CAMERA_DELETE;

////////////////////////////////MCU////////////////////////////////////////////////

#define  MCU_MAC_LEN_20         	20
#define  USRNAME_LEN_20             20


enum //分析服务器和MCU通信命令
{
	SM_MCU_VDCS_ENCRY_REQ = 0x1000,
	SM_VDCS_MCU_ENCRYPT_ACK,
	SM_MCU_VDCS_ENCRYPT_ACK,
	SM_VDCS_MCU_PUBLIC_KEY,
	SM_MCU_VDCS_ENCRYPT_KEY,
	SM_VDCS_MCU_ENCRYPT_KEY_ACK,
	SM_MCU_VDCS_REGISTER,
	SM_VDCS_MCU_REGISTER_ACK,
	SM_VDCS_MCU_OPERATE_TERM,
	SM_MCU_VDCS_OPERATE_TERM_ACK,
	SM_MCU_VDCS_ROUTING_INSPECTION,
	SM_VDCS_MCU_ROUTING_INSPECTION_ACK,
	
    SM_MCU_HEARTBEAT = 0x8001
};

//SM_MCU_VDCS_REGISTER
typedef struct _MCU_VDCS_REGISTER
{
	uint8 MCUAddr[MCU_MAC_LEN_20];
	uint8 ClientType;
	
	_MCU_VDCS_REGISTER(){
		memset(this, 0, sizeof(_MCU_VDCS_REGISTER));
	}
	
}T_MCU_VDCS_REGISTER;

//SM_VDCS_MCU_REGISTER_ACK
typedef struct  _VDCS_MCU_REGISTER_ACK
{
	uint8	MCUAddr[MCU_MAC_LEN_20];
	uint8 	ClientType;
	uint16	Ack;    //ACK状态 0：成功 1：失败
	
	_VDCS_MCU_REGISTER_ACK(){
		memset(this, 0, sizeof(_VDCS_MCU_REGISTER_ACK));
	}
} T_VDCS_MCU_REGISTER_ACK;


//SM_VDCS_MCU_OPERATE_TERM
typedef struct  _VDCS_MCU_OPERATE_TERM
{
	uint8   	UserName[USRNAME_LEN_20];
	uint8		MCUAddr [MCU_MAC_LEN_20];
	uint8		port;              // 1~4   4个报警器
	uint8		TermType;          // 2/3 报警器 还是电控锁
	uint8		OpFlag;            // 操作指令码       0 / 1  kai /guan
	uint16		Res;     
	
	_VDCS_MCU_OPERATE_TERM(){
		memset(this, 0, sizeof(_VDCS_MCU_OPERATE_TERM));
	}	
} T_VDCS_MCU_OPERATE_TERM;

//SM_MCU_VDCS_OPERATE_TERM_ACK
typedef struct _MCU_VDCS_OPERATE_TERM_ACK
{
	uint8		UserName[USRNAME_LEN_20];
	uint8		MCUAddr[MCU_MAC_LEN_20];
	uint8		port;
	uint8		TermType;
	uint16		Ack;	           //ACK状态 0：成功 1：失败
	
	_MCU_VDCS_OPERATE_TERM_ACK(){
		memset(this, 0, sizeof(_MCU_VDCS_OPERATE_TERM_ACK));
	}	
	
} T_MCU_VDCS_OPERATE_TERM_ACK;

/********************************报警服务器*******************************************/

enum ANAY_ALARM_CMD {  

	SM_ANAY_ALARM_REGISTER = 0x2000,

	SM_ANAY_ALARM_DEVICE_STATUS ,

	SM_ANAY_ALARM_WARN_INFO,

	SM_ANAY_ALARM_HEATBEAT = 0X8005
};


//SM_ANAY_ALARM_REGISTER
typedef struct _VDCS_ANAY_ALARM_REGISTER
{
	
	uint32 AnaServerID;   	/* 每一个ID 表示分析服务器的编号和地址 */
	char   AnaServerIp[16];
	uint8  CameraNum;     	/* 每一个分析服务器所分析的摄像头个数 */
	
	_VDCS_ANAY_ALARM_REGISTER(){
		memset(this, 0, sizeof(_VDCS_ANAY_ALARM_REGISTER));
	}		
}T_ANAY_ALARM_REG;

typedef struct _ANA_CAM_PARAM
{
	char		CamIp[IP_LEN_16];
	char		CamUrL[SINGLE_URL_LEN_128];
	uint8		AnalyzeNUM;	
	uint16    	AnalyzeType;   /* 分析类型相或*/ 

	_ANA_CAM_PARAM(){
		memset(this, 0, sizeof(_ANA_CAM_PARAM));
	}	
}T_ANA_CAM_PARAM;

//SM_ANAY_ALARM_WARN_INFO
typedef struct  _VDCS_ANAY_ALARM_WARN_INFO
{
	uint32 		AnaServerID;   	       /* 每一个ID 表示分析服务器的编号和地址 */
	char   		AnaServerIp[16];
	char		CamIp[IP_LEN_16];
	char		CamUrL[SINGLE_URL_LEN_128];
	uint16		AnalyzeType;   /*单一分析类型*/
	uint8       PicbuffEN;
	
	_VDCS_ANAY_ALARM_WARN_INFO(){
		memset(this, 0, sizeof(_VDCS_ANAY_ALARM_WARN_INFO));
	}		
}T_ANAY_ALARM_WARN_INFO;


//SM_ANAY_ALARM_DEVICE_STATUS
typedef struct _VDCS_ANAY_ALARM_DEVICE_STATUS
{
	uint32 AnaServerID;   	       /* 每一个ID 表示分析服务器的编号和地址 */
	char   AnaServerIp[16];
	char   CamIp[IP_LEN_16];			/* 摄像头的ip */
	char   CamUrL[SINGLE_URL_LEN_128];  /* 摄像头的url */
	
	_VDCS_ANAY_ALARM_DEVICE_STATUS(){
		memset(this, 0, sizeof(_VDCS_ANAY_ALARM_DEVICE_STATUS));
	}		
}T_ANAY_ALARM_DEV;


//SM_ANAY_ALARM_HEATBEAT
typedef struct _VDCS_ANAY_ALARM_HEATBEAT
{
	uint32 AnaServerID;   	       /* 每一个ID 表示分析服务器的编号和地址 */
	char   AnaServerIp[16];
	uint8  CamNum;
	
	_VDCS_ANAY_ALARM_HEATBEAT(){
		memset(this, 0, sizeof(_VDCS_ANAY_ALARM_HEATBEAT));
	}		
}T_ANAY_ALARM_HEATBEAT;

typedef struct _VDCS_ANAY_ALARM_CAM_PIC
{
	char   CamIp[IP_LEN_16];			/* 摄像头的ip */
	char   CamUrL[SINGLE_URL_LEN_128];  /* 摄像头的url */
	char   buffer[PIC_BUFFER_LEN];
	
	_VDCS_ANAY_ALARM_CAM_PIC(){
		memset(this, 0, sizeof(_VDCS_ANAY_ALARM_CAM_PIC));
	}	
}T_ANAY_ALARM_CAM_PIC;

#pragma pack(pop)

#endif
