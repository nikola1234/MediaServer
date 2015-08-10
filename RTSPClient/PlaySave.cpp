#include "stdafx.h"

#include <stdio.h>

#include "PlaySave.h"
#include "RTSPClient.h"
#include "VideoDeviceSDK.h"

CPlaySave::CPlaySave()
{
	m_bPlaySave = true;
}

CPlaySave::~CPlaySave(void)
{
}

int CPlaySave::RecvData(int timestamp, unsigned long dwDataType, unsigned char *data, unsigned long len)
{
	if( m_capture_callback == NULL || m_prtsp_client == NULL )return -1;

	//to do
	if( m_scale < 0.1 )
		NULL;

	((VIDEO_PLAYBACK_CALLBACK)m_capture_callback)(timestamp, dwDataType, len, data, (int)m_user);

	return len;
}

int CPlaySave::ChangePlaySpeed( float scale )
{
	return m_prtsp_client->ChangePlayCommandInPlay(m_url, scale);
}

int CPlaySave::ChangePlayRangeClock(const char* Range_clock)
{
	return m_prtsp_client->ChangePlayCommandInPlay(m_url, 1.0, Range_clock);
}

int CPlaySave::RePlayCommend()
{
	return m_prtsp_client->ChangePauseToPlay(m_url);
}

int CPlaySave::PauseCommend()
{
	return m_prtsp_client->ChangePlayToPause(m_url);
}

std::string CPlaySave::GetCurPlayBackTime()
{
	std::string strRet("");
	int iRet = m_prtsp_client->GetCurPlayBackTimeInPaly(m_url);
	if( iRet >= 0 ) strRet = m_prtsp_client->GetCurPlayBackTime();
	return strRet;
}