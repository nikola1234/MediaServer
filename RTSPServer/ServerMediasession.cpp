#include "ServerMediasession.h"
#include <time.h>

#include <string>
#include "DataInfo.h"
#include "RTPPackSender.h"
#include "TCPRTPSender.h"
#include "RAWSender.h"
#include "UDPSender.h"

#include "RTSPClient.h"

namespace live555RTSP
{

RTSPServer* ServerMediasession::m_pRTSPServer = NULL;
boost::atomic<int> ServerMediasession::s_MRMediaIDBegin( 1000000001 );
ServerMediasession::ServerMediasession(RTSPServer* ourserver, CDataInfo::DBCAMERAINFO &cameradata, int coderid)
	:m_coderid(coderid),
	m_mediastate(CREATE),
	m_isHaveHeadData(false),
	m_headLen(0),
	m_bliveMediaSession(true),
	m_cur_recvdata_time(time(NULL)),
	m_bRun(true),
	m_iTSPSHeadBufCurSize(0),
	m_iH264NalCurSize(0),
	m_iFrameCurSize(0),
	m_bChannelControl(false)
{
	m_cameradata = cameradata;
	m_coderid = coderid;

	if( m_coderid == 0 )
		m_mediaid = m_cameradata.CameraID;
	else
	{
		m_mediaid = m_coderid;
		m_bChannelControl = true;
	}

	char strMediaID[32]; memset(strMediaID, 0, 32);
	//itoa(m_mediaid, strMediaID, 10);
	sprintf(strMediaID, "%d", m_mediaid);
	m_strURLSuffix = strMediaID;

	if( m_pRTSPServer == NULL )
		m_pRTSPServer = ourserver;

	memset( m_strResult, 0, 256 );
	m_pRTSPServer->m_log.Add( "ServerMediasession::ServerMediasession m_mediaid = %d", m_mediaid );


	//////////////////////////////////////////////////////////////////////////
	memset(m_headData,0,HEAD_SIZE);
	m_headLen = 40;
	m_isHaveHeadData = true;
	//////////////////////////////////////////////////////////////////////////
}

ServerMediasession::~ServerMediasession(void)
{
	StopMediaServer();

	writeLock writelock_(m_senderMutex);
	m_senderList.clear();
	writelock_.unlock();

	m_bRun = false;
	if(m_pHandleRecvDataThread != NULL)
		m_pHandleRecvDataThread->join();

	m_pRTSPServer->m_log.Add("ServerMediasession::~ServerMediasession m_mediaid = %d", m_mediaid);
}

void ServerMediasession::RemoveRTPPackSender(SenderPtr rtpsenderptr)
{
	writeLock writelock_(m_senderMutex);
	m_senderList.remove(rtpsenderptr);
}

void ServerMediasession::RemoveRTPPackSenderBySessionID(unsigned int sessionid)
{
	writeLock writelock_(m_senderMutex);
	std::list<SenderPtr>::iterator it = m_senderList.begin();
	for (; it != m_senderList.end(); it++)
	{
		if((*it)->GetSessionID() == sessionid)
		{
			(*it)->Stop();
			m_senderList.erase(it);
			break;
		}
	}
}

unsigned int ServerMediasession::GetFirstSenderSessionid()
{
	readLock readlock_(m_senderMutex);
	std::list<SenderPtr>::iterator it = m_senderList.begin();
	if( it != m_senderList.end() ){
		return (*it)->GetSessionID();
	}
	return 0;
}

void ServerMediasession::CloseClientSessionByUserName(char* UserName)
{
	readLock readlock_(m_senderMutex);
	std::list<SenderPtr>::iterator it = m_senderList.begin();
	boost::system::error_code ec;
	for ( ; it != m_senderList.end(); it++ )
	{
		RTSPServer::RTSPClientPtr rtspClinetPtr = (*it)->GetRTSPClientPtr();
		if( rtspClinetPtr != NULL && rtspClinetPtr->GetTokenType() == TOKEN_RECORDBYUSER
			&& strcmp(rtspClinetPtr->GetUserName(), UserName) == 0 ){
			rtspClinetPtr->GetrtspSocket().close(ec);
			break;
		}
	}
}

void ServerMediasession::StopSenderBySessionID(unsigned int sessionid)
{
	readLock readlock_(m_senderMutex);
	std::list<SenderPtr>::iterator it = m_senderList.begin();
	for (; it != m_senderList.end(); it++)
	{
		if((*it)->GetSessionID() == sessionid){
			(*it)->Stop();
			break;
		}
	}
}

ServerMediasession::SenderPtr ServerMediasession::GetRTPPackSenderBySessionID(unsigned int sessionid)
{
	SenderPtr rtp_packsender_ptr;

	readLock readlock_(m_senderMutex);
	std::list<SenderPtr>::iterator it = m_senderList.begin();
	for (; it != m_senderList.end(); it++)
	{
		if((*it)->GetSessionID() == sessionid){
			rtp_packsender_ptr = *it;
			break;
		}
	}

	return rtp_packsender_ptr;
}

int ServerMediasession::GetSenderCount()
{
	readLock readlock_(m_senderMutex);
	return m_senderList.size();
}

char* ServerMediasession::generateSDPDescription(int iCameraID, const char* chUnitName, const char* chCameraName)
{
	time_t fCreationTime = time(NULL);
	//time(&fCreationTime);
	unsigned long time1 = (unsigned long)fCreationTime;
	unsigned long time2 = time1 * 2;

	char const* const sdpPrefixFmt =
		"v=0\r\n"
		"o=- %u %u IN IP4 %s\r\n"
		"s=MS\r\n"
		"i=%s %s&%d\r\n"
		"t=0 0\r\n"
		"a=tool: MS\r\n"
		"a=range:npt=0-  0.00000\r\n"
		"m=video 0 RTP/AVP 96\r\n"
		"a=rtpmap:96 H264/90000\r\n"
		"a=fmtp:96 packetization-mode=1;\r\n"//profile-level-id=000042;sprop-parameter-sets=h264
		"a=framerate:25.00\r\n"
		"a=control:track1\r\n\r\n";

	std::string ipstr = m_pRTSPServer->getRTSPAddress();
	char ipaddress[64]; memset(ipaddress, 0 ,64);
	strcpy(ipaddress,ipstr.c_str());

	char *sdp = new char[8000];
	memset(sdp,0,8000);
	if( iCameraID == 0 || chUnitName == NULL)
	{
		// Generate the SDP prefix (session-level lines):
		sprintf(sdp, sdpPrefixFmt,
			time1,
			time2, // o= <session id>
			ipaddress ,  // o= <address>
			m_cameradata.UnitName,
			m_cameradata.CameraName,
			GetCurCameraID()/*,
			ipaddress ,
			m_pRTSPServer->GetRTSPListenPort(),
			GetCurCameraID()*/); // miscellaneous session SDP lines (if any)
	}
	else
	{
		// Generate the SDP prefix (session-level lines):
		sprintf(sdp, sdpPrefixFmt,
			time1,
			time2, // o= <session id>
			ipaddress ,  // o= <address>
			chUnitName,
			chCameraName,
			GetCurCameraID()/*,
			ipaddress ,
			m_pRTSPServer->GetRTSPListenPort(),
			GetCurCameraID()*/); // miscellaneous session SDP lines (if any)
	}

	return sdp;
}

int ServerMediasession::getStreamParameters(
	StreamingMode streammode,
	unsigned clientSessionId, // in
	const char* clientAddress, // in
	unsigned short& clientRTPPort, // in
	unsigned short& clientRTCPPort, // in
	unsigned short& serverRTPPort, // out
	unsigned short& serverRTCPPort // out
	)
{
	SenderPtr senderptr_;
	switch (streammode)
	{
	case RTP_UDP:
		{
			//senderptr_ = SenderPtr(new CUDPRTPSender(m_pRTSPServer));
			senderptr_ = SenderPtr(new CUDPSender(m_pRTSPServer));
		}
		break;
	case RTP_TCP:
		{
			senderptr_ = SenderPtr(new CTCPRTPSender(m_pRTSPServer));
		}
		break;
	case RAW_TCP:
		{
			senderptr_ = SenderPtr(new CRAWSender(m_pRTSPServer));
		}
		break;
	default:
		{
			//senderptr_ = SenderPtr(new CUDPRTPSender(m_pRTSPServer));
			senderptr_ = SenderPtr(new CUDPSender(m_pRTSPServer));
		}
		break;
	}

	int iret = senderptr_->InitRTPPackSender(clientSessionId, (char *)clientAddress, clientRTPPort,
		clientRTCPPort, serverRTPPort, serverRTCPPort);
	senderptr_->Start();

	if( streammode == TCP_RTMP )
		senderptr_->SetSenderState(CSender::WAITFOR_SENDHAED);

	writeLock writelock_(m_senderMutex);
	m_senderList.push_back(senderptr_);

	return iret;
}

int ServerMediasession::startStream(unsigned clientSessionId, std::string time_param, float scale)
{
	if(m_mediastate != CREATE)return 0;
	m_mediastate = OPENING;

	RESULT_PARAM_S resultParam;
	memset(&resultParam, 0, sizeof(RESULT_PARAM_S));

	int iret  = m_pRTSPServer->GetDevInfo().openCamera
		(m_cameradata.CameraID, m_coderid, m_mediaid, video_capture_callbackEx, &resultParam);

	if( iret < 0 )
	{
		resultParam.unResultInfo.strResult[251] = 0;
		resultParam.unResultInfo.strResult[250] = 0;
		strcpy(m_strResult, resultParam.unResultInfo.strResult);
	}

	m_mediastate = WORKING;
	return iret;
}

int ServerMediasession::switchCamera(int ioldCamera, int inewCamera, std::vector<int>& channleLevels)
{
	if(m_mediastate != WORKING)return 0;
	int iReturn = -2;
	return iReturn;
}

int ServerMediasession::CameraControl(int cmd, int param1, int param2)
{
	if(m_mediastate != WORKING)return 0;
	int iret  = m_pRTSPServer->GetDevInfo().controlCamera(GetCurCameraID(), m_coderid, cmd, param1);
	return iret;
}

int ServerMediasession::stopStream(unsigned clientSessionId)
{
	int iret = 0;
	StopSenderBySessionID(clientSessionId);
	return iret;
}

int ServerMediasession::StopMediaServer()
{
	if(!m_bliveMediaSession)return -1;
	if(m_mediastate != WORKING)return -1;
	int iret = m_pRTSPServer->GetDevInfo().closeCamera( m_cameradata.CameraID );
	m_mediastate = STOP;
	return iret;
}

int ServerMediasession::SendData(unsigned long  dwDataType, unsigned long bufsize, unsigned char *buffer, int timestampinc)
{
	int iret = 0;
	m_cur_recvdata_time = time(NULL);

	//if (!m_isHaveHeadData) ����ͷ������
	FullHeadData(dwDataType, bufsize, buffer);

	readLock readlock_(m_senderMutex);
	std::list<SenderPtr>::iterator it = m_senderList.begin();
	for ( ; it != m_senderList.end(); it++ )
	{
		if((*it)->GetRTPSenderState() == CRTPSender::WAITFOR_SENDHAED && m_isHaveHeadData )
		{
			int datatype = (1 << 16) + (dwDataType&0xFFFF);
			iret = (*it)->SendData( datatype, m_headData, m_headLen, timestampinc );
		}else if ((*it)->GetRTPSenderState() == CRTPSender::WORKING)
		{
			//iret = (*it)->SendData( dwDataType&0xFFFF, buffer, bufsize, timestampinc );
			if ( ( dwDataType&0x00010000 ) > 0 && m_isHaveHeadData )
			{
				int datatype = (1 << 16) + (dwDataType&0xFFFF);
				iret = (*it)->SendData( datatype, m_headData, m_headLen, timestampinc );
			}else
			{
				iret = (*it)->SendData( dwDataType&0xFFFF, buffer, bufsize, timestampinc );
			}
		}
	}
	return iret;
}

int ServerMediasession::FullHeadData(unsigned long  dwDataType, unsigned long bufsize, unsigned char *buffer)
{
	int iret = 0;
	//if(m_isHaveHeadData)return 0;
	if( m_isHaveHeadData && ( dwDataType >> 16 ) != 1 )return 0;
	if ( ( dwDataType >> 16 ) > 0 && bufsize <= HEAD_SIZE ){
		m_headLen = bufsize;
		memcpy(m_headData, buffer, bufsize);
		m_isHaveHeadData = true;
	}
	return iret;
}


int ServerMediasession::HandleH264PackData(unsigned long  dwDataType, unsigned long bufsize, unsigned char *buffer, int timestampinc)
{
	if( m_pH264_RTP_PACK == NULL || m_pH264PackBuf == NULL  ){
		m_pH264_RTP_PACK = (boost::shared_ptr<CH264_RTP_PACK>)(new CH264_RTP_PACK(0));
		m_pH264PackBuf = boost::shared_ptr<char>(new char[RAWDATALEN + 20]);
		m_iH264NalCurSize = 0;
		m_iFrameCurSize = 0;

		m_pH264SPSPPSdata = boost::shared_ptr<STH264SPSPPSdata>(new STH264SPSPPSdata());
		memset(m_pH264SPSPPSdata.get(), 0, sizeof(STH264SPSPPSdata));
	}
	unsigned char* pbuf = (unsigned char*)(m_pH264PackBuf.get()) + 20;

	if( m_iH264NalCurSize + bufsize < RAWDATALEN )
	{
		memcpy(pbuf + m_iH264NalCurSize, buffer, bufsize);
		return SendH264RTPPack(dwDataType, m_iH264NalCurSize + bufsize, pbuf);
	}
	else
	{
		memcpy(pbuf + m_iH264NalCurSize, buffer, RAWDATALEN - m_iH264NalCurSize - 1);
		return SendH264RTPPack(dwDataType, RAWDATALEN - 1, pbuf);
	}
	return 0;
}

int ServerMediasession::SendH264RTPPack(unsigned long  dwDataType, unsigned long bufsize, unsigned char *buffer, int timestampinc)
{
	static int cc = 0;

	unsigned char *pbuf = buffer;
	unsigned char *pNAL = pbuf;
	unsigned char *pBegin = pbuf, *pEnd, *pOEFEnd = pbuf + bufsize;
	int Nalhead = 0;

	bool bGetStart = false;
	bool bGetEnd = false;
	for( int i = 0; i < (int)bufsize; ){
		if( (bufsize - i > 3) && pbuf[i] == 0 && pbuf[i + 1] == 0 && pbuf[i + 2] == 1 )
		{
			if( bGetStart ){
				pEnd = pbuf + i;
				bGetEnd = true;
				//i += 3;
			}else{
				pBegin = pbuf + i - 1;
				*pBegin = 0;
				bGetStart = true;
				i += 3;
			}
		}else if ( (bufsize - i > 4) && pbuf[i] == 0 && pbuf[ i + 1 ] == 0 && pbuf[ i + 2 ] == 0 && pbuf[ i + 3 ] == 1 )
		{
			if( bGetStart ){
				pEnd = pbuf + i;
				bGetEnd = true;
				//i += 4;
			}else{
				pBegin = pbuf + i;
				bGetStart = true;
				i += 4;
			}
		}
		else{  i++; }

		if( bGetStart && bGetEnd ){
			bGetStart = false;
			bGetEnd = false;

			HandleH264NaluData(dwDataType, pBegin, pEnd - pBegin, timestampinc);
			m_iH264NalCurSize = 0;
		}
	}

	if( bGetStart ){
		pEnd = pOEFEnd;
		memmove(pbuf, pBegin, pEnd - pBegin);
		m_iH264NalCurSize = pEnd - pBegin;
	}
	if( !bGetStart && !bGetEnd ){
		m_iH264NalCurSize = 0;
	}
	return 0;
}

void ServerMediasession::HandleH264NaluData(unsigned long  dwDataType, unsigned char* pH264NalBuf, unsigned long iH264NalLen, int timestampinc)
{
	if( m_pH264SPSPPSdata != NULL && ( m_pH264SPSPPSdata->nPpsLen == 0 || m_pH264SPSPPSdata->nSpsLen == 0 ) )
	{
		int unit_nal_type = 0;
		int i = 0;
		int iNalBegin = 0;
		if( (iH264NalLen > 3) && pH264NalBuf[i] == 0 && pH264NalBuf[i + 1] == 0 && pH264NalBuf[i + 2] == 1 )
		{
			unit_nal_type = pH264NalBuf[i + 3];
			iNalBegin = 3;
		}else if ( (iH264NalLen > 4) && pH264NalBuf[i] == 0 && pH264NalBuf[ i + 1 ] == 0 && pH264NalBuf[ i + 2 ] == 0 && pH264NalBuf[ i + 3 ] == 1 )
		{
			unit_nal_type = pH264NalBuf[i + 4];
			iNalBegin = 4;
		}

		if( (unit_nal_type&0x1F) == 7 ) //SPS
		{
			if( iH264NalLen < 1024 && m_pH264SPSPPSdata != NULL)
			{
				m_pH264SPSPPSdata->nSpsLen = iH264NalLen - iNalBegin;
				memcpy(m_pH264SPSPPSdata->Sps, pH264NalBuf + iNalBegin, iH264NalLen - iNalBegin);
			}
		}
		if( (unit_nal_type&0x1F) == 8 ) //PPS
		{
			if( iH264NalLen < 1024 && m_pH264SPSPPSdata != NULL)
			{
				m_pH264SPSPPSdata->nPpsLen = iH264NalLen - iNalBegin;
				memcpy(m_pH264SPSPPSdata->Pps, pH264NalBuf + iNalBegin, iH264NalLen - iNalBegin);
			}
		}
	}
	m_pH264_RTP_PACK->Set ( pH264NalBuf, iH264NalLen, 0, true );
	GetAndSendRTPPackData(dwDataType, timestampinc);
}

void ServerMediasession::GetAndSendRTPPackData(unsigned long  dwDataType,int timestampinc)
{
	int oldDataType = dwDataType;
	unsigned char *pPacket ;
	unsigned short wLen ;
	while ( pPacket = m_pH264_RTP_PACK->Get ( &wLen ) ) {
		if (!m_isHaveHeadData)
		{
			dwDataType = (1 << 16) + ( oldDataType & 0xffff );
			unsigned char head[40]; memset(head, 0, 40);
			SendData(dwDataType, 40, head, timestampinc);
			dwDataType = oldDataType;
		}
		else
			dwDataType = oldDataType;
		SendData(dwDataType, wLen, pPacket, timestampinc);
	}
}

bool __stdcall ServerMediasession::video_capture_callbackEx(unsigned long  dwDataType, unsigned long bufsize, unsigned char *buffer, int user)
{
	RTSPServer::ServerMediaPtr servermadiaptr = m_pRTSPServer->FindServerMediaSession((int)user);
	if(servermadiaptr == NULL)return false;

	if( !(servermadiaptr->GetDevType() <= SDK_AGENT_DEVTYPE && servermadiaptr->GetDevType() > H264_MAX_DEVTYPE) )
	{
		dwDataType = ( dwDataType&0xFFFF0000 ) + ( servermadiaptr->GetDevType() & 0x0000FFFF );
	}

	if( servermadiaptr->GetDevType() <= H264_MAX_DEVTYPE )
	{
		servermadiaptr->HandleH264PackData(dwDataType, bufsize, buffer);
		return true;
	}
	servermadiaptr->SendData(dwDataType, bufsize, buffer);
	return true;
}

}// end namespace
