#pragma once
#include <memory.h>

class IData
{
public:
#define STR_LEN 64
	typedef struct _DBCAMERAINFO_
	{
		int		CameraID;
		char	CameraName[STR_LEN];
		char	UnitName[STR_LEN];
		char	Input[256];
		char	Remark[STR_LEN];
		int		DevID;
		int		DevType;
		char	sControlIP[STR_LEN];
		int     CameraType;
		int     isMatrix;
		_DBCAMERAINFO_()
		{
			CameraID	= 0;
			memset(CameraName, 0, STR_LEN);
			memset(UnitName, 0, STR_LEN);
			memset(Remark, 0, STR_LEN);
			DevID		= 0;
			DevType		= 0;
			CameraType = 0;
			memset(sControlIP, 0, STR_LEN);
			memset( Input, 0, 256 );
			isMatrix = 0;
		}
	} DBCAMERAINFO;

	typedef struct _DBDEVINFO_
	{
		int		DevID;
		char	IPAddr[STR_LEN];
		int		Port;
		char	User[STR_LEN];
		char	Pwd[STR_LEN];
		int		DevType;

		_DBDEVINFO_()
		{
			DevID	= 0;
			memset(IPAddr, 0, STR_LEN);
			Port	= 0;
			memset(User, 0, STR_LEN);
			memset(Pwd, 0, STR_LEN);
			DevType	= 0;
		}

	} DBDEVINFO;

	typedef struct _DBCODERINFO_ 
	{
		int		CoderID;
		char	IPAddr[STR_LEN];
		int		Port;
		char	User[STR_LEN];
		char	Pwd[STR_LEN];
		int     Input;
		int		OutPut;
		int		MatrixID;
		_DBCODERINFO_()
		{
			CoderID	= 0;
			memset(IPAddr, 0, STR_LEN);
			Port	= 0;
			memset(User, 0, STR_LEN);
			memset(Pwd, 0, STR_LEN);
			OutPut	= 0;
			MatrixID= 0;
		}
	} DBCODERINFO;

	typedef struct _DBMATRIXINFO_
	{
		int MatrixID;
		int MatrixType;
		int	Baud;
		int DataBit;
		int StartBit;
		int StopBit;
		int Parity;
		int FlowControl;
		char SoftConIPAddr[STR_LEN];//软件控制IP地址
		DBCODERINFO ControlCoder;
		_DBMATRIXINFO_()
		{
			MatrixID	= 0;
			MatrixType	= 0;
			Baud		= 0;
			DataBit		= 0;
			StartBit	= 0;
			StopBit		= 0;
			Parity		= 0;
			FlowControl	= 0;
			memset(SoftConIPAddr, 0, STR_LEN);
		}
	} DBMATRIXINFO;

public:  
	virtual ~IData() {}
	virtual int getCameraInfo(int cameraid, DBCAMERAINFO* camera) = 0;     //获取摄像机信息
	virtual int getCoderInfo(int CoderID, DBCODERINFO* coder) = 0;	       //获取编码器通道信息
	virtual int getMatrixInfo(int MatrixID, DBMATRIXINFO* MatrixInfo) = 0; //获取矩阵信息
	virtual int getDevInfo(int DevID, DBDEVINFO* dev) = 0;	               //获取设备信息
};

typedef struct tagResultParam
{
	int resultType;
	union tagResultInfo{
		int fullCameraID[63];//对接平台时，作为通道满时，返回正在打开的摄像机ID。
		char strResult[252];//openCamera、switchCamera返回错误时，存储错误信息，传递给客户端显示
	}unResultInfo;
}RESULT_PARAM_S;

#ifndef WIN32
#ifndef __stdcall
#define __stdcall __attribute__((__stdcall__))
#endif
#endif

typedef bool (__stdcall *VIDEO_CAPTURE_CALLBACKEX)(unsigned long dwDataType, unsigned long bufsize, unsigned char *buffer, int user);

class ICamera
{
public:
	ICamera(): m_pIData(NULL){} //构造函数中m_pIData尚未赋值不能使用,可在调用StartRTSPServer之后使用
	virtual ~ICamera() {}
	void SetIData( IData* pIData ){ m_pIData = pIData; } //在调用StartRTSPServer函数后，m_pIData可用
	virtual int openCamera(int CameraID, int ChannelID, int user, VIDEO_CAPTURE_CALLBACKEX capture_callback, RESULT_PARAM_S *resultParam) = 0;
	virtual int closeCamera(int CameraID) = 0;
	virtual int switchCamera(int CameraID, int oldCameraID, int ChannelID, int user, VIDEO_CAPTURE_CALLBACKEX capture_callback) = 0;
	virtual int controlCamera(int CameraID, int ChannelID, int ControlType, int ControlSpeed) = 0;
protected:
	IData* m_pIData;
};

typedef struct _XMLCAMERAINFO_
{
	char chCameraName[64];	   //摄像机名称
	char chCameraInput[256];   //摄像机唯一标识
	int  iOnline;              //0-不在线 1-在线；
	int  iStatus;              //0-正常 1-无法打开 2-花屏或黑屏；
	char chSmX[64];			   //经度
	char chSmY[64];			   //纬度
	int  iUnitID;			   //iUnitID<=0时 默认取DevID的前四位 接入点单位ID=AccessPointID
	char chRemark[64];         //备注 MS代理时格式：摄像机ID&21编码&是否可控&排序
	_XMLCAMERAINFO_()
	{
		memset(chCameraName, 0, 64);
		memset(chCameraInput, 0, 256);
		iOnline		= 1;
		iStatus		= 0;
		memset(chSmX, 0, 64);
		memset(chSmY, 0, 64);
		iUnitID = -1;
		memset(chRemark, 0, 64);
	}
}STCAMERAINFO;

class IDevice
{
public:
	IDevice(){} //构造函数中m_pIData 尚未赋值 不能使用
	virtual ~IDevice() {}
	void SetIData( IData* pIData ){ m_pIData = pIData; } //在调用StartRTSPServer函数后，m_pIData可用

	//outResult为MSDLL内部分配内存大小为65536
	//访问接口方式：http://IP:PORT/getDeviceInfo?AAA&BBBB 其中IP为MSIP PORT为MSRTSPPORT pParam为 AAA&BBBB
	virtual int getDeviceInfo(const char* pParam, char* outResult) = 0;
protected:
	IData* m_pIData;
};