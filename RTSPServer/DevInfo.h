// DevInfo.h: interface for the CDevInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEVINFO_H__AE362017_F227_47CB_988C_B8EAFBF9334D__INCLUDED_)
#define AFX_DEVINFO_H__AE362017_F227_47CB_988C_B8EAFBF9334D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataInfo.h"
#include "errorcode.h"
//#include "commanddef.h"

enum MSDBTYPE {
	DB_SQLITE = 0,
	DB_POSTGRE,
	DB_POSTGRE_MATRIX
};

class CDevInfo  
{
public:
	CDevInfo();
	virtual ~CDevInfo();

	int initDevInfo(ICamera* pICamera, const char* ms_dbfile);

	//openCamera调看成功返回此摄像机的类型
	int openCamera(int CameraID, int ChannelID, int user, VIDEO_CAPTURE_CALLBACKEX capture_callback, RESULT_PARAM_S *resultParam);
	int closeCamera(int CameraID);
	int switchCamera(int CameraID, int oldCameraID, int ChannelID, int user, VIDEO_CAPTURE_CALLBACKEX capture_callback);
	int controlCamera(int CameraID, int ChannelID, int ControlType, int ControlSpeed);

public:
	CDataInfo* GetDataInfoPtr(){ return m_pDataInfo; };
	int GetDBType(){ return m_iDBType; };

private:
	CDataInfo* m_pDataInfo;
	ICamera* m_pICamera;
	int m_iDBType;
};

#endif // !defined(AFX_DEVINFO_H__AE362017_F227_47CB_988C_B8EAFBF9334D__INCLUDED_)
