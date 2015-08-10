// DevInfo.cpp: implementation of the CDevInfo class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DevInfo.h"


#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/xml_parser.hpp>


CDevInfo::CDevInfo()
	: m_pDataInfo(NULL)
	, m_pICamera(NULL)
	, m_iDBType(DB_SQLITE)
{

}

CDevInfo::~CDevInfo()
{
	if (m_pDataInfo){
		delete m_pDataInfo;
		m_pDataInfo = NULL;
	}
}

int CDevInfo::initDevInfo(ICamera* pICamera, const char* ms_dbfile)
{
	m_pICamera = pICamera;
	try{
		//boost::property_tree::ptree pt;
		//boost::property_tree::xml_parser::read_xml("MSConfigure.xml",pt);
		//std::string strDBType = pt.get<std::string>("MSBaseConfig.DBType");
		//if( strDBType.compare("Postgre") == 0 && m_pDataInfo == NULL )
		//{
		//	m_pDataInfo = new CPGDataInfo();
		//	m_iDBType = DB_POSTGRE;
		//	m_pDataInfo->SetConfig("PORT_TOClient",pt.get<std::string>("MSBaseConfig.PORT_TOClient").c_str());
		//	m_pDataInfo->SetConfig("MSIP", pt.get<std::string>("MSBaseConfig.MSIP").c_str());

		//	std::string subSystemID = pt.get<std::string>("MSBaseConfig.SUBSYSTEMID","-100");
		//	std::string connectDBStr = pt.get<std::string>("MSBaseConfig.ConnectDBStr","NODB");
		//	std::string strMATRIX = pt.get<std::string>("MSBaseConfig.MATRIX","0");
		//	if( strMATRIX.compare("1") == 0 ) m_iDBType = DB_POSTGRE_MATRIX;
		//	if( subSystemID.compare("-100") != 0 && connectDBStr.compare("NODB") != 0 ){
		//		m_pDataInfo->SetConfig("SUBSYSTEMID",subSystemID.c_str());
		//		m_pDataInfo->SetConfig("ConnectDBStr",connectDBStr.c_str());
		//	}
		//}
	}catch(...){}

	if(m_pDataInfo == NULL){
		m_pDataInfo = new CDataInfo();
		m_pDataInfo->Open( ms_dbfile );
	}
	if( m_pICamera != NULL )
		m_pICamera->SetIData( ( IData* ) m_pDataInfo );
	return 0;
}

int CDevInfo::openCamera(int CameraID, int ChannelID, int user, VIDEO_CAPTURE_CALLBACKEX capture_callback, RESULT_PARAM_S *resultParam)
{
	try
	{
		if( m_pICamera == NULL )return ERROR_SYSTEM_ERROR;

		int iRet = m_pICamera->openCamera(CameraID, ChannelID, user, capture_callback, resultParam);
		return iRet;
	}
	catch (...)
	{
		return ERROR_SYSTEM_ERROR;
	}
	return 0;
}

int CDevInfo::closeCamera(int CameraID)
{
	try
	{
		if( m_pICamera == NULL )return ERROR_SYSTEM_ERROR;
		return m_pICamera->closeCamera(CameraID);
	}
	catch (...)
	{
		return ERROR_SYSTEM_ERROR;
	}
	return 0;
}

int CDevInfo::switchCamera(int CameraID, int oldCameraID, int ChannelID, int user, VIDEO_CAPTURE_CALLBACKEX capture_callback)
{
	try
	{
		if( m_pICamera == NULL )return ERROR_SYSTEM_ERROR;
		int iRet = m_pICamera->switchCamera(CameraID, oldCameraID, ChannelID, user, capture_callback);
		return iRet;
	}
	catch (...)
	{
		return ERROR_SYSTEM_ERROR;
	}
	return 0;
}

int CDevInfo::controlCamera(int CameraID, int ChannelID, int ControlType, int ControlSpeed)
{
	try
	{
		if( m_pICamera == NULL )return ERROR_SYSTEM_ERROR;
		return m_pICamera->controlCamera(CameraID, ChannelID, ControlType, ControlSpeed);
	}
	catch (...)
	{
		return ERROR_SYSTEM_ERROR;
	}
	return 0;
}
