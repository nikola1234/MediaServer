#include "stdafx.h"
#include <stdio.h>

#include "OpenCamera.h"
#include "RTSPClient.h"
#include "UDP_RTSPClient.h"
#include "TCP_RTSPClient.h"
#include "RAW_RTSPClient.h"
#include "VideoDeviceSDK.h"

#include "ConnectMC.h"
#include "ManageLogin.h"

extern CManageLogin::ConnectMCPtr g_connectMCptr_;

boost::atomic<int> COpenCamera::s_mediaid(1);

COpenCamera::COpenCamera()
	:m_transport_type(TRANSPORT_UDP),
	m_prtsp_client(NULL),
	m_cameraid(0),
	m_port(0),
	m_iscallbackex(false),
	m_capture_callback(NULL),
	m_user(NULL),
	m_scale(1.0),
	m_curRecvDataTime(time(NULL)),
	m_nReConnectTimes(0),
	m_bPlaySave(false)
{
	memset(m_ip, 0, 256);
	memset(m_url, 0, 256);
	memset(m_username, 0, 256);
	memset(m_password, 0, 256);
	memset(m_openErrorInfo, 0, 256);
	m_mediaid = s_mediaid++;
}

COpenCamera::~COpenCamera(void)
{
	StopCamera();
}

int COpenCamera::InitOpenCamera(char* url, int transport_type)
{
	int iret = GetIPAndAddressByURL(url);
	m_transport_type = transport_type;
	return iret;
}

int COpenCamera::StartCamera(void* capture_callback, void* user, bool iscallbackex, const char* Range_clock, float scale)
{
	int nret = -1;
	m_scale = scale;

	StopCamera();

	m_capture_callback = capture_callback;
	m_iscallbackex = iscallbackex;
	m_user = user; 

	switch( m_transport_type )
	{
	case TRANSPORT_UDP:
		m_prtsp_client = new CUDP_RTSPClient(this);
		break;
	case TRANSPORT_TCP:
		m_prtsp_client = new CTCP_RTSPClient(this);
		break;
	case TRANSPORT_RAW:
		m_prtsp_client = new CRAW_RTSPClient(this);
		break;
	default:
		m_prtsp_client = new CUDP_RTSPClient(this);
		break;
	}

	if(m_prtsp_client == 0)
		return -1;

	m_prtsp_client->setUsernameAndPassword(m_username, m_password);
	nret = m_prtsp_client->startRTSPRequest(m_ip, m_port, m_url, Range_clock, scale);
	if( nret == CRTSPClient::NeedToRedirectCommand ){
		nret = m_prtsp_client->stopRTSPRequest();
		nret = GetIPAndAddressByURL(m_prtsp_client->GetLocationUrl());
		if( nret < 0 )return nret;
		nret = m_prtsp_client->startRTSPRequest(m_ip, m_port, m_url, Range_clock, scale);
	}
	if( nret == CRTSPClient::FullChannelResponse )
	{
		int inewCamera = m_cameraid;
		nret = m_prtsp_client->stopRTSPRequest();
		
		GetRealUserNameAndLevels(); //获取权限信息

		bool bHavePrivate = false;
		int nPrivateNUM = 0;
		for (std::size_t i = 0; i < m_channleLevels.size(); i++){
			if( m_channleLevels[i] >= 7 ){ bHavePrivate = true; nPrivateNUM++; }
		}

		if( ( !bHavePrivate ) && ( g_connectMCptr_->m_clientType&0x01 > 0 ) )
			return ERROR_CODEC_FULL;

		int cameraidNO = 0;
		int cameraID = g_connectMCptr_->FindNoOpenCameraID(m_prtsp_client->GetCameraList(), cameraidNO);
		if(cameraID <= 0)return ERROR_CODEC_FULL;

		m_prtsp_client->setUsernameAndPassword(m_username, m_password);
		sprintf( m_url, "rtsp://%s:%d/%d", m_ip, m_port, cameraID );
		nret = GetIPAndAddressByURL(m_url);
		if( nret < 0 )return nret;
		nret = m_prtsp_client->startRTSPRequest(m_ip, m_port, m_url, Range_clock, scale);

		if( bHavePrivate && cameraidNO <= nPrivateNUM ) //专有通道 todo
		{ //专有通道时 直接切换
			if( nret >= 0 )nret = SwitchCamera( cameraID, inewCamera );
			if( nret >= 0 )ChangeCameraIDAndUrl( inewCamera );
		}
	}
	if ( nret < 0 && m_prtsp_client != NULL )
	{
		if( strlen(m_prtsp_client->GetOpenErrorInfo()) > 0 )
			strcpy(m_openErrorInfo, m_prtsp_client->GetOpenErrorInfo());
		m_prtsp_client->stopRTSPRequest();
	}

	return nret;
}

int COpenCamera::GetSaveFileInfo(const char* Range_clock, char* resultStr)
{
	StopCamera();
	m_prtsp_client = new CRAW_RTSPClient(this);

	if(m_prtsp_client == 0)
		return -1;

	m_prtsp_client->setUsernameAndPassword(m_username, m_password);
	int nret = m_prtsp_client->GetSaveFileInfo(m_ip, m_port, m_url, Range_clock, resultStr);
	m_prtsp_client->stopRTSPRequest();

	return nret;
}

int COpenCamera::SwitchCamera(int ioldCamera, int inewCamera)
{
	//boost::lock_guard<boost::mutex> lock_(m_mutex);
	g_MSNetSDKlog.Add("SwitchCamera ioldCamera = %d, inewCamera = %d", ioldCamera, inewCamera);
	return m_prtsp_client->SwitchCameraInPlay(m_url, ioldCamera, inewCamera);
}

void COpenCamera::ChangeCameraIDAndUrl(int inewCamera)
{
	m_cameraid = inewCamera;
	sprintf( m_url, "rtsp://%s:%d/%d", m_ip, m_port, m_cameraid );
}

int COpenCamera::CameraPTZCtrl(int ptzCMD, int param1, int param2)
{
	//boost::lock_guard<boost::mutex> lock_(m_mutex);
	return m_prtsp_client->CameraPTZCtrlInPlay(m_url, ptzCMD, param1, param2);
}

void COpenCamera::StopCamera()
{
	boost::lock_guard<boost::mutex> lock_(m_mutex);
	if (m_prtsp_client != NULL)
	{
		m_prtsp_client->stopRTSPRequest();
		delete m_prtsp_client;
		m_prtsp_client = NULL;
	}
	m_capture_callback = NULL;
}

int COpenCamera::RecvData(int timestamp, unsigned long dwDataType, unsigned char *data, unsigned long len)
{
	//g_MSNetSDKlog.Add("COpenCamera::RecvData timestamp = %d dwDataType = %d, len = %d", timestamp, dwDataType, len);
	if( m_capture_callback == NULL || m_prtsp_client == NULL )return -1;
	m_curRecvDataTime = time(NULL);
	if (m_iscallbackex)
	{
		bool nret = ((VIDEO_CAPTURE_CALLBACKEx)m_capture_callback)(dwDataType, len, data, m_user);
		if (! nret ){
			m_prtsp_client->stopRTSPRequest();
			g_MSNetSDKlog.Add("COpenCamera::RecvData false m_user = %d, iCamera = %d", m_user, m_prtsp_client->GetCurCameraID());
		}
	}
	else
	{
		((VIDEO_CAPTURE_CALLBACK)m_capture_callback)(dwDataType, len, data, (int)m_user);
	}
	return len;
}

int COpenCamera::GetIPAndAddressByURL(const char* url)
{
	char* username;
	char* password;
	char* address;
	char* urlSuffix;
	int portNum;

	int iret = 0;
	if(CRTSPClient::parseRTSPURL( url, username, password, address, portNum, urlSuffix ))
	{
		if(address != NULL && urlSuffix!= NULL &&
			strlen(address) < 256 /*&& atoi( urlSuffix ) > 0*/ )
		{
			iret = 0;
			if(username != NULL) strcpy(m_username, username);
			if(password != NULL) strcpy(m_password, password);
			if(address != NULL)
				strcpy(m_ip, address);

			m_port = portNum;
			m_cameraid = atoi( urlSuffix );
			if( m_cameraid <= 10000000 || m_cameraid > 999999999 || strlen(urlSuffix) > 14 ) 
				m_cameraid = CRTSPClient::s_virtualCameraID++;
			sprintf( m_url, "rtsp://%s:%d/%s", m_ip, m_port, urlSuffix );

		} else {  iret = -1; }

	} else  {  iret = -1; }

	delete[] username; username = NULL;
	delete[] password; password = NULL;
	delete[] address; address = NULL;
	delete[] urlSuffix; urlSuffix = NULL;

	return iret;
}

int COpenCamera::GetCameraIDFromURL(const char* sz_url)
{
	char* username;
	char* password;
	char* address;
	char* urlSuffix;
	int portNum;

	int iret = 0;
	int icameraID = -1;
	if(CRTSPClient::parseRTSPURL( sz_url, username, password, address, portNum, urlSuffix ))
	{
		if(address != NULL && urlSuffix!= NULL &&
			strlen(address) < 256/* && atoi( urlSuffix ) > 0 */)
		{
			iret = 0;
			icameraID = atoi( urlSuffix );
			if( icameraID <= 10000000 || icameraID > 999999999 || strlen(urlSuffix) > 14 ) 
				icameraID = CRTSPClient::s_virtualCameraID++;
		} else {  iret = -1; }
	} else  {  iret = -1; }

	delete[] username; username = NULL;
	delete[] password; password = NULL;
	delete[] address; address = NULL;
	delete[] urlSuffix; urlSuffix = NULL;

	return icameraID;
}

int COpenCamera::GetRTSPUrlOnvif( const char* sz_url, std::string& rtsp_url )
{
	char* username;
	char* password;
	char* address;
	char* urlSuffix;
	int portNum;

	int iret = -1;
	if(CRTSPClient::parseRTSPURL( sz_url, username, password, address, portNum, urlSuffix ))
	{
		if(address != NULL && urlSuffix!= NULL &&
			strlen(address) < 256/* && atoi( urlSuffix ) > 0 */)
		{
			iret = 0;
			if(username != NULL)m_strOnvifUser = username;
			if(password != NULL)m_strOnvifPwd = password;
			if(address != NULL) m_strOnvifIP = address;
			m_iOnvifPort = portNum;
			m_iOnvifChannel = atoi( urlSuffix );

		} else {  iret = -1; }
	} else  {  iret = -1; }

	delete[] username; username = NULL;
	delete[] password; password = NULL;
	delete[] address; address = NULL;
	delete[] urlSuffix; urlSuffix = NULL;

	rtsp_url = "";

	return iret;
}

int COpenCamera::IsInTheSameRoad(int Caid1, int Caid2)
{
	if ( ((Caid1/100000) == (Caid2/100000)) && Caid1 > 100000000 ) {
		return true;
	}
	else{
		return false;
	}
}

int COpenCamera::ReStartCamera()
{
	g_MSNetSDKlog.Add("COpenCamera::ReStartCamera cameraid = %d url = %s user = %d", m_cameraid, m_url, m_user);
	return this->StartCamera(m_capture_callback, m_user, m_iscallbackex);
}

int COpenCamera::GetRealUserNameAndLevels()
{
	if (strlen(m_username) == 0) return -1;

	char userlevels[256]; memset(userlevels, 0, 256);
	unsigned resultlen = 0;
	unsigned char* pBase64 = base64Decode((char*)m_username, resultlen, True);
	if( resultlen < 256 ) memcpy(userlevels, pBase64, resultlen);
	delete[] pBase64;

	std::string strUserLevels = userlevels;

	std::string::size_type userPos = strUserLevels.find_first_of(':');
	std::string strUserName,strLevels;
	if( userPos == std::string::npos )return -1;

	strUserName = strUserLevels.substr(0, userPos);
	strLevels = strUserLevels.substr(userPos + 1, strUserLevels.length() - userPos - 1);
	std::string::size_type levelPos = strLevels.find_first_of('-');
	//if( strUserName.size() < 64 ) strcpy(m_szUserName, strUserName.c_str());

	if( levelPos ==  std::string::npos ) {
		for ( std::string::size_type i = 0; i < strLevels.length(); i ++ )
		{
			if( strLevels[i] >= '0' && strLevels[i] <= '9' )
				m_channleLevels.push_back( strLevels[i] - '0' );
			else
				m_channleLevels.push_back( 0 );
		}
	} else {
		if( strLevels[0] >= '0' && strLevels[0] <= '9' )
			m_UserLevel = strLevels[0] - '0';
		else
			m_UserLevel = 0;
	}
	return 0;
}