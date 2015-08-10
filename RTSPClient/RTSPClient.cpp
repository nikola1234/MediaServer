#include "RTSPClient.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "ManageLogin.h"
#include "ConnectMC.h"
#include "OpenCamera.h"
extern CManageLogin::ConnectMCPtr g_connectMCptr_;

#include "Log.h"
extern CLog g_MSNetSDKlog;

unsigned CRTSPClient::responseBufferSize = RECVRTSPRESPONSELEN;
int CRTSPClient::m_sClientID = 0;
int CRTSPClient::s_virtualCameraID = 10000000;

VIDEO_SWITCH_CAMERA_UPDATE_CALLBACK CRTSPClient::s_UpdateCameraNameCallBack = NULL;
unsigned long CRTSPClient::s_UpdateCameraCallBackUser = -1;

CRTSPClient::CRTSPClient(void)
	:rtsp_socket_( io_service_ ),
	m_SrvVideoUDPPort(0),
	m_SrvAudioUDPPort(0),
	m_brun(true),
	m_cameraid(0),
	m_commandStatusInPlay(FREE_STATUS),
	m_responseCodeInPlay(0),
	m_bSendingVideoSETUP(false),
	m_bSendingAudioSETUP(false),
	m_bVideoThreadRun(false)
{
	fResponseBuffer = new char[responseBufferSize];
	resetResponseBuffer();

	m_iNetType = RTSP_TCP;
	srand( (unsigned)time( NULL ) );
	m_icseq = rand();
	m_iCommandStatus = NULLCOMMAND;

	memset( m_url, 0, 256 );
	memset( m_baseUrl, 0, 256 );
	memset( m_SessionID, 0, 256 );
	memset( m_SrvAddressStr, 0 ,256 );

	m_pSaveFileInfo = new char[RTSPCOMMANDLEN];
	memset(m_pSaveFileInfo, 0, RTSPCOMMANDLEN);

	//if(strlen(m_user_agent) == 0)
	if(m_sClientID == 0)
		m_sClientID = rand();

	memset(m_openErrorInfo, 0, 256);
	memset(m_curPlayBackTime, 0, 256);
	sprintf(m_user_agent, "VNMPNetSDK V1.0 00 %d", m_sClientID);
}

CRTSPClient::~CRTSPClient(void)
{
	try
	{
		boost::system::error_code err;
		m_brun = false;
		rtsp_socket_.close(err);
		io_service_.stop();
	} catch (...){ }

	try{
		m_recvVideoThread_.join();
		m_recvAudioThread_.join();
	}catch(...){ }

	delete[] fResponseBuffer; fResponseBuffer = NULL;
	delete[] m_pSaveFileInfo; m_pSaveFileInfo = NULL;
}

int CRTSPClient::setRTSPClientType(int NetType)
{
	m_iNetType = NetType;
	return 0;
}

int CRTSPClient::sendtoSvr(char *buf, int len)
{
	int nSumLen = len;
	int nRet = 0,nCur = 0;

	std::cout<< buf <<std::endl;

	try
	{
		while(nCur < nSumLen)
		{
			nRet = rtsp_socket_.send( boost::asio::buffer( (char *)buf + nCur, nSumLen - nCur ) );
			if(nRet <= 0)return -1;
			nCur += nRet;
		}
		return len;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return ERROR_SOCKET_SEND;
	}
	return ERROR_SOCKET_SEND;
}

int CRTSPClient::sendOptionsCommand(const char * url)
{
	char sendBuf[RTSPCOMMANDLEN];
	memset(sendBuf, 0,RTSPCOMMANDLEN);
	char * OptionsInfo = 
		"OPTIONS %s RTSP/1.0\r\n"
		"CSeq: %d\r\n"
		"User-Agent:%s\r\n"
		"\r\n";
	sprintf(sendBuf, OptionsInfo, url, m_icseq, m_user_agent);
	m_iCommandStatus = OPTOINSCOMMAND;
	return sendtoSvr(sendBuf,strlen(sendBuf));
	return 0;
}

int CRTSPClient::sendDescribeCommand(const char * url, const char* contentType, const char* Range_clock)
{
	char* authenticatorStr = createAuthenticatorString("DESCRIBE", url);

	char clockStr[256]; memset(clockStr, 0, 256);
	if(strcmp(contentType, "SaveFileInfo") == 0)
		sprintf(clockStr, "Range: clock=%s\r\n", Range_clock);

	char sendBuf[RTSPCOMMANDLEN];
	memset(sendBuf, 0, RTSPCOMMANDLEN);
	char * OptionsInfo = 
		"DESCRIBE %s RTSP/1.0\r\n"
		"CSeq: %d\r\n"
		"%s"
		"%s"
		"Accept: application/%s\r\n"
		"User-Agent:%s\r\n"
		"\r\n";
	sprintf(sendBuf, OptionsInfo, url, m_icseq, authenticatorStr, clockStr, contentType, m_user_agent);
	m_strAuthenticator = authenticatorStr;
	delete[] authenticatorStr;
	m_iCommandStatus = DESCRIBECOMMAND;
	return sendtoSvr(sendBuf,strlen(sendBuf));

	return 0;
}

int CRTSPClient::sendPlayCommand(const char * url, float scale, const char* Range_clock )
{
	char* authenticatorStr = createAuthenticatorString("PLAY", url);

	std::string strRange_clock;
	if( strlen( Range_clock ) < 3 ){
		strRange_clock = "Range: npt=0.000-\r\n";
	}
	else
	{
		strRange_clock = "Range: clock=";
		strRange_clock += Range_clock;
		strRange_clock += "\r\n";
	}

	char sendBuf[RTSPCOMMANDLEN];
	memset(sendBuf, 0, RTSPCOMMANDLEN);
	char * OptionsInfo = 
		"PLAY %s/ RTSP/1.0\r\n"
		"CSeq: %d\r\n"
		"%s"
		"Session: %s\r\n"
		"%s"
		"Scale: %1.1f\r\n"
		"User-Agent:%s\r\n"
		"\r\n";

	sprintf(sendBuf, OptionsInfo, url, m_icseq, authenticatorStr/*m_strAuthenticator.c_str()*/, m_SessionID, strRange_clock.c_str(), scale, m_user_agent);
	m_iCommandStatus = PLAYCOMMAND;

	delete[] authenticatorStr;

	if(!m_recvVideoThread_.joinable())
		m_recvVideoThread_ = boost::thread( &CRTSPClient::handleRecvVideoDataThread , this );

	boost::this_thread::sleep(boost::posix_time::milliseconds(100));

	return sendtoSvr(sendBuf, strlen(sendBuf));
	return 0;
}

int CRTSPClient::sendPauseCommand( const char * url )
{
	char sendBuf[RTSPCOMMANDLEN];
	memset(sendBuf, 0, RTSPCOMMANDLEN);
	char * OptionsInfo = 
		"PAUSE %s RTSP/1.0\r\n"
		"CSeq: %d\r\n"
		"Session:%s\r\n"
		"User-Agent:%s\r\n"
		"\r\n";
	sprintf(sendBuf, OptionsInfo, url, m_icseq, m_SessionID, m_user_agent);
	m_iCommandStatus = TEARDOWNCOMMAND;
	return sendtoSvr(sendBuf,strlen(sendBuf));
	return 0;
}

int CRTSPClient::ChangePlayToPause(const char * url)
{
	m_icseq++;
	m_commandStatusInPlay = WAITFOR_RESPONSE;
	int iret = sendPauseCommand(url);
	if( iret < 0 )return iret;

	//int waitseconds = 0;
	//while(waitseconds++ < 10*100){
	//	if(m_commandStatusInPlay != WAITFOR_RESPONSE)break;
	//	boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	//}

	//if(m_commandStatusInPlay != SUCCESS_RESPONSE)
	//	return -1;

	return SUCCESS_RESPONSE;
}

int CRTSPClient::ChangePauseToPlay(const char * url, float scale, const char* Range_clock)
{
	m_icseq++;
	m_commandStatusInPlay = WAITFOR_RESPONSE;
	int iret = sendPlayCommand(url, scale, Range_clock);
	if( iret < 0 )return iret;

	//int waitseconds = 0;
	//while(waitseconds++ < 10*100){
	//	if(m_commandStatusInPlay != WAITFOR_RESPONSE)break;
	//	boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	//}

	//if(m_commandStatusInPlay != SUCCESS_RESPONSE)
	//	return -1;

	return SUCCESS_RESPONSE;
}

int CRTSPClient::SwitchCameraInPlay(const char * url, int ioldCamera, int inewCamera)
{
	m_icseq++;
	m_commandStatusInPlay = WAITFOR_RESPONSE;
	int iret = sendSet_ParameterCommand(url, CAMERA_COMMAND_MONITOR_CAMERA_SELECT, ioldCamera, inewCamera);
	if( iret < 0 )return iret;

	int waitseconds = 0;
	while(waitseconds++ < 10*100){
		if(m_commandStatusInPlay != WAITFOR_RESPONSE)break;
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}

//	m_log.Add("CRTSPClient::SwitchCameraInPlay m_commandStatusInPlay = %d ", m_commandStatusInPlay);
	if(m_commandStatusInPlay != SUCCESS_RESPONSE)
	{
		if( m_responseCodeInPlay == 565 )
			return -2; //reopen
		else if( m_responseCodeInPlay == 566 )
			return -3; //È¨ÏÞ²»×ã
		else 
			return -1; //ÇÐ»»Ê§°Ü
	}

	return SUCCESS_RESPONSE;
}

int CRTSPClient::CameraPTZCtrlInPlay(const char * url, int cmd, int param1, int param2)
{
	m_icseq++;
	m_commandStatusInPlay = WAITFOR_RESPONSE;
	int iret = sendSet_ParameterCommand(url, cmd, param1, param2);
	if( iret < 0 )return iret;

	//int waitseconds = 0;
	//while(waitseconds++ < 10*100){
	//	if(m_commandStatusInPlay != WAITFOR_RESPONSE)break;
	//	boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	//}

	//if(m_commandStatusInPlay != SUCCESS_RESPONSE)
	//	return -1;

	return SUCCESS_RESPONSE;
}

int CRTSPClient::GetCurPlayBackTimeInPaly(const char * url)
{
	m_icseq++;
	m_commandStatusInPlay = WAITFOR_RESPONSE;
	int iret = sendSet_ParameterCommand(url, PLAY_BACK_GETTIME, 0, 0);
	if( iret < 0 )return iret;

	int waitseconds = 0;
	while(waitseconds++ < 1*100){
		if(m_commandStatusInPlay != WAITFOR_RESPONSE)break;
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}

	if(m_commandStatusInPlay != SUCCESS_RESPONSE)
		return -1;

	return SUCCESS_RESPONSE;
}

int CRTSPClient::ChangePlayCommandInPlay(const char * url, float scale, const char* Range_clock)
{
	//int i = ChangePlayToPause(url);
	//if( i < 0 )return i;
	return ChangePauseToPlay(url, scale, Range_clock);
}

int CRTSPClient::sendTeardownCommand(const char * url)
{
	char sendBuf[RTSPCOMMANDLEN];
	memset(sendBuf, 0, RTSPCOMMANDLEN);
	char * OptionsInfo = 
		"TEARDOWN %s RTSP/1.0\r\n"
		"CSeq: %d\r\n"
		"Session:%s\r\n"
		"User-Agent:%s\r\n"
		"\r\n";
	sprintf(sendBuf, OptionsInfo, url, m_icseq, m_SessionID, m_user_agent);
	m_iCommandStatus = TEARDOWNCOMMAND;
	return sendtoSvr(sendBuf,strlen(sendBuf));
	return 0;
}

int CRTSPClient::sendSet_ParameterCommand(const char * url, int cmd, int param1, int param2)
{
	char sendBuf[RTSPCOMMANDLEN];
	memset(sendBuf, 0, RTSPCOMMANDLEN);
	char * OptionsInfo = 
		"SET_PARAMETER %s RTSP/1.0\r\n"
		"CSeq: %d\r\n"
		"Session:%s\r\n"
		"VnmpControl:CMD=%d;Parameter1=%d;Parameter2=%d\r\n"
		"User-Agent:%s\r\n"
		"\r\n";

	sprintf(sendBuf, OptionsInfo, url, m_icseq, m_SessionID, cmd, param1, param2, m_user_agent);
	return sendtoSvr(sendBuf,strlen(sendBuf));
	return 0;
}

int CRTSPClient::sendGet_ParameterCommand( const char * url )
{
	char sendBuf[RTSPCOMMANDLEN];
	memset(sendBuf, 0, RTSPCOMMANDLEN);
	char * OptionsInfo = 
		"GET_PARAMETER %s RTSP/1.0\r\n"
		"CSeq: %d\r\n"
		"Session:%s\r\n"
		"User-Agent:%s\r\n"
		"\r\n";

	sprintf(sendBuf, OptionsInfo, url, m_icseq, m_SessionID, m_user_agent);
	return sendtoSvr(sendBuf,strlen(sendBuf));
	return 0;
}

int CRTSPClient::recvRTSPResponse()
{
	try
	{
		int i = 0;
		for (i = 0; i < 1000; i++)
		{
			boost::asio::socket_base::bytes_readable command(true);
			rtsp_socket_.io_control(command);
			std::size_t bytes_readable = command.get();
			if( bytes_readable > 0 ) break;
			boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		}
		if( i == 1000) return ERROR_SOCKET_OVERTIME;

		int bytesRead = rtsp_socket_.read_some( boost::asio::buffer(
			(char*)&fResponseBuffer[fResponseBytesAlreadySeen], fResponseBufferBytesLeft) );

		return bytesRead;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return ERROR_SOCKET_RECV;
	}
	return ERROR_SOCKET_RECV;
}

int CRTSPClient::HandleIncomingData()
{
	int newBytesRead = recvRTSPResponse();
	if(newBytesRead <= 0)return newBytesRead;
	//g_MSNetSDKlog.Add("recv %s", fResponseBuffer);
	int i = 0;
	do 
	{
		i = handleResponseBytes(newBytesRead);
		if(i == NOEnoughData){
			newBytesRead = recvRTSPResponse();
			if( newBytesRead <= 0 )return newBytesRead;
		}
	} while (i == NOEnoughData);
	return i;
}

Boolean CRTSPClient
::parseResponseCode(char const* line, unsigned& responseCode, char const*& responseString) {
  if (sscanf(line, "RTSP/%*s%u", &responseCode) != 1 &&
      sscanf(line, "HTTP/%*s%u", &responseCode) != 1) return False;
  // Note: We check for HTTP responses as well as RTSP responses, both in order to setup RTSP-over-HTTP tunneling,
  // and so that we get back a meaningful error if the client tried to mistakenly send a RTSP command to a HTTP-only server.

  // Use everything after the RTSP/* (or HTTP/*) as the response string:
  responseString = line;
  while (responseString[0] != '\0' && responseString[0] != ' '  && responseString[0] != '\t') ++responseString;
  while (responseString[0] != '\0' && (responseString[0] == ' ' || responseString[0] == '\t')) ++responseString; // skip whitespace

  return True;
}

Boolean CRTSPClient::checkForHeader(char const* line, char const* headerName,
									unsigned headerNameLength, char const*& headerParams) 
{
	if (_strncasecmp(line, headerName, headerNameLength) != 0) return False;

	// The line begins with the desired header name.  Trim off any whitespace, and return the header parameters:
	unsigned paramIndex = headerNameLength;
	while (line[paramIndex] != '\0' && (line[paramIndex] == ' ' || line[paramIndex] == '\t')) ++paramIndex;
	if (&line[paramIndex] == '\0') return False; // the header is assumed to be bad if it has no parameters

	headerParams = &line[paramIndex];
	return True;
}

int CRTSPClient::sendANNOUNCEResponse(int icseq)
{
	char sendBuf[RTSPCOMMANDLEN];
	memset(sendBuf, 0, RTSPCOMMANDLEN);
	char * OptionsInfo = 
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Session:%s\r\n"
		"User-Agent:%s\r\n"
		"\r\n";
	sprintf(sendBuf, OptionsInfo, icseq, m_SessionID, m_user_agent);
	return sendtoSvr(sendBuf,strlen(sendBuf));
	return 0;
}

int CRTSPClient::handleANNOUNCERequest()
{
	char* headerDataCopy;
	char* bodyStart = NULL;
	unsigned numBodyBytes = 0;

	headerDataCopy = new char[responseBufferSize];
	strncpy(headerDataCopy, fResponseBuffer, fResponseBytesAlreadySeen);
	headerDataCopy[fResponseBytesAlreadySeen] = '\0';

	char* lineStart = headerDataCopy;
	char* nextLineStart = getLine(lineStart);

	// Scan through the headers, handling the ones that we're interested in:
	Boolean reachedEndOfHeaders;
	unsigned cseq = 0;
	unsigned contentLength = 0;
	unsigned numExtraBytesAfterResponse = 0;

	while (1) {
		reachedEndOfHeaders = True; // by default; may get changed below
		lineStart = nextLineStart;
		if (lineStart == NULL) break;

		nextLineStart = getLine(lineStart);
		if (lineStart[0] == '\0') break; // this is a blank line
		reachedEndOfHeaders = False;

		char const* headerParamsStr; 
		if (checkForHeader(lineStart, "CSeq:", 5, headerParamsStr)) {
			if (sscanf(headerParamsStr, "%u", &cseq) != 1 || cseq <= 0) {
				//envir().setResultMsg("Bad \"CSeq:\" header: \"", lineStart, "\"");
				break;
			}
			if(cseq != m_icseq)break;
		} else if (checkForHeader(lineStart, "Content-Length:", 15, headerParamsStr)) {
			if (sscanf(headerParamsStr, "%u", &contentLength) != 1) {
				//envir().setResultMsg("Bad \"Content-Length:\" header: \"", lineStart, "\"");
				break;
			}
		} 
	}

	if (!reachedEndOfHeaders){
		delete[] headerDataCopy;
		return Failed;
	}

	// If we saw a "Content-Length:" header, then make sure that we have the amount of data that it specified:
	unsigned bodyOffset = nextLineStart - headerDataCopy;
	bodyStart = &fResponseBuffer[bodyOffset];
	numBodyBytes = fResponseBytesAlreadySeen - bodyOffset;
	if (contentLength > numBodyBytes) {
		// We need to read more data.  First, make sure we have enough space for it:
		unsigned numExtraBytesNeeded = contentLength - numBodyBytes;
		unsigned remainingBufferSize = responseBufferSize - fResponseBytesAlreadySeen;
		if (numExtraBytesNeeded > remainingBufferSize) {
			return Failed;
		}

		delete[] headerDataCopy;
		//if (foundRequest != NULL) fRequestsAwaitingResponse.putAtHead(foundRequest);// put our request record back; we need it again
		return NOEnoughData; // We need to read more data
	}

	// We now have a complete response (including all bytes specified by the "Content-Length:" header, if any).
	char* responseEnd = bodyStart + contentLength;
	numExtraBytesAfterResponse = &fResponseBuffer[fResponseBytesAlreadySeen] - responseEnd;

	if (numExtraBytesAfterResponse > 0) {
		char* responseEnd = &fResponseBuffer[fResponseBytesAlreadySeen - numExtraBytesAfterResponse];

		memmove(fResponseBuffer, responseEnd, numExtraBytesAfterResponse);
		fResponseBytesAlreadySeen = numExtraBytesAfterResponse;
		fResponseBufferBytesLeft = responseBufferSize - numExtraBytesAfterResponse;
		fResponseBuffer[numExtraBytesAfterResponse] = '\0';
		
		delete[] headerDataCopy;
		return handleResponseBytes(0);
	} else {

		if ( numBodyBytes > 0 )
			handleSDPResponse(bodyStart);
		sendANNOUNCEResponse(cseq);
		delete[] headerDataCopy;
		resetResponseBuffer();
	}
	return ResponseSUCCESS;
}

int CRTSPClient::handleIncomingRequest()
{
	// Parse the request string into command name and 'CSeq', then 'handle' the command (by responding that we don't support it):

	char cmdName[RTSP_PARAM_STRING_MAX];
	char urlPreSuffix[RTSP_PARAM_STRING_MAX];
	char urlSuffix[RTSP_PARAM_STRING_MAX];
	char cseq[RTSP_PARAM_STRING_MAX];
	unsigned contentLength;
	if (!parseRTSPRequestString(fResponseBuffer, fResponseBytesAlreadySeen,
		cmdName, sizeof cmdName,
		urlPreSuffix, sizeof urlPreSuffix,
		urlSuffix, sizeof urlSuffix,
		cseq, sizeof cseq,
		contentLength)) {
			return Failed;
	} else {
		if (strcmp(cmdName, "ANNOUNCE") == 0) {
			return handleANNOUNCERequest();
		}
		char tmpBuf[2*RTSP_PARAM_STRING_MAX];
		snprintf((char*)tmpBuf, sizeof tmpBuf,
			"RTSP/1.0 405 Method Not Allowed\r\nCSeq: %s\r\n\r\n", cseq);
		sendtoSvr(tmpBuf, strlen(tmpBuf));
	}
	return Failed;
}

int CRTSPClient::handleResponseBytes(int newBytesRead)
{
	fResponseBufferBytesLeft -= newBytesRead;
	fResponseBytesAlreadySeen += newBytesRead;
	fResponseBuffer[fResponseBytesAlreadySeen] = '\0';
	unsigned numExtraBytesAfterResponse = 0;
	Boolean responseSuccess = False;
	Boolean needToResendCommand = False; // by default...
	Boolean needToRedirectCommand = False; // by default...
	Boolean bFullChannelResponse = False; // by default...

	int returnValue = Failed;
	do {
		Boolean endOfHeaders = False;
		char const* ptr = fResponseBuffer;
		if (fResponseBytesAlreadySeen > 3) {
			char const* const ptrEnd = &fResponseBuffer[fResponseBytesAlreadySeen-3];
			while (ptr < ptrEnd) {
				if (*ptr++ == '\r' && *ptr++ == '\n' && *ptr++ == '\r' && *ptr++ == '\n') {
					endOfHeaders = True;
					break;
				}
			}
		}

		if (!endOfHeaders) return NOEnoughData;

		char* headerDataCopy;
		unsigned responseCode = 200;
		char const* responseStr = NULL;
		//RequestRecord* foundRequest = NULL;
		char const* sessionParamsStr = NULL;
		char const* transportParamsStr = NULL;
		char const* scaleParamsStr = NULL;
		char const* rangeParamsStr = NULL;
		char const* rtpInfoParamsStr = NULL;
		char const* wwwAuthenticateParamsStr = NULL;
		char const* publicParamsStr = NULL;
		char const* LocationParamsStr = NULL;
		char const* contentTypeParamsStr = NULL;
		char const* fullChannelParamsStr = NULL;
		char const* openErrorParamsStr = NULL;
		char const* curPlayBackTime = NULL;

		char* bodyStart = NULL;
		unsigned numBodyBytes = 0;
		responseSuccess = False;

		do {

			headerDataCopy = new char[responseBufferSize];
			strncpy(headerDataCopy, fResponseBuffer, fResponseBytesAlreadySeen);
			headerDataCopy[fResponseBytesAlreadySeen] = '\0';

			char* lineStart = headerDataCopy;
			char* nextLineStart = getLine(lineStart);
			if (!parseResponseCode(lineStart, responseCode, responseStr)) {
				// This does not appear to be a RTSP response; perhaps it's a RTSP request instead?
				//handleIncomingRequest();
				handleSDPResponse(fResponseBuffer);
				break; // we're done with this data
			}

			// Scan through the headers, handling the ones that we're interested in:
			Boolean reachedEndOfHeaders;
			unsigned cseq = 0;
			unsigned contentLength = 0;

			while (1) {
				reachedEndOfHeaders = True; // by default; may get changed below
				lineStart = nextLineStart;
				if (lineStart == NULL) break;

				nextLineStart = getLine(lineStart);
				if (lineStart[0] == '\0') break; // this is a blank line
				reachedEndOfHeaders = False;

				char const* headerParamsStr; 
				if (checkForHeader(lineStart, "CSeq:", 5, headerParamsStr)) {
					if (sscanf(headerParamsStr, "%u", &cseq) != 1 || cseq <= 0) {
						//envir().setResultMsg("Bad \"CSeq:\" header: \"", lineStart, "\"");
						break;
					}
					if(cseq != m_icseq)break;
				} else if (checkForHeader(lineStart, "Content-Type:", 13, contentTypeParamsStr)) {
				} else if (checkForHeader(lineStart, "Content-Length:", 15, headerParamsStr)) {
					if (sscanf(headerParamsStr, "%u", &contentLength) != 1) {
						//envir().setResultMsg("Bad \"Content-Length:\" header: \"", lineStart, "\"");
						break;
					}
				} else if (checkForHeader(lineStart, "Content-Base:", 13, headerParamsStr)) {
					//setBaseURL(headerParamsStr);
				} else if (checkForHeader(lineStart, "Session:", 8, sessionParamsStr)) {

					char* sessionId = new char[responseBufferSize]; // ensures we have enough space
					// Check for a session id:
					if (sessionParamsStr == NULL || sscanf(sessionParamsStr, "%[^;]", sessionId) != 1){ }
					else {
						strcpy(m_SessionID,sessionId);
					}
					delete[] sessionId;

				} else if (checkForHeader(lineStart, "Transport:", 10, transportParamsStr)) {
				} else if (checkForHeader(lineStart, "Scale:", 6, scaleParamsStr)) {
				} else if (checkForHeader(lineStart, "Range:", 6, rangeParamsStr)) {
				} else if (checkForHeader(lineStart, "RTP-Info:", 9, rtpInfoParamsStr)) {
				} else if (checkForHeader(lineStart, "CurPlayBackTime:", 16, curPlayBackTime)) {
					std::string str = curPlayBackTime;
					if( str.length() > 0 && str.length() < 250 )
						strcpy(m_curPlayBackTime, curPlayBackTime);
				} else if (checkForHeader(lineStart, "OpenErrorInfo:", 14, openErrorParamsStr)) {
					std::string str = openErrorParamsStr;
					if( str.length() > 0 && str.length() < 250 )
						strcpy(m_openErrorInfo, openErrorParamsStr);
				} else if (checkForHeader(lineStart, "VnmpFullCameraID:", 17, fullChannelParamsStr)) {
					std::string camerastr = fullChannelParamsStr;
					int i = camerastr.find_first_of(';');
					m_cameraIdList.clear();
					while(i > 0){
						std::string str = camerastr.substr(0, i);
						m_cameraIdList.push_back(atoi(str.c_str()));
						camerastr = camerastr.substr(i + 1,camerastr.size());
						i = camerastr.find_first_of(';');
					}
				} else if (checkForHeader(lineStart, "WWW-Authenticate:", 17, headerParamsStr)) {
					// If we've already seen a "WWW-Authenticate:" header, then we replace it with this new one only if
					// the new one specifies "Digest" authentication:
					if (wwwAuthenticateParamsStr == NULL || _strncasecmp(headerParamsStr, "Digest", 6) == 0) {
						wwwAuthenticateParamsStr = headerParamsStr;
					}
				} else if (checkForHeader(lineStart, "Public:", 7, publicParamsStr)) {
				} else if (checkForHeader(lineStart, "Allow:", 6, publicParamsStr)) {
					// Note: we accept "Allow:" instead of "Public:", so that "OPTIONS" requests made to HTTP servers will work.
				} else if (checkForHeader(lineStart, "Location:", 9, LocationParamsStr)) {
					//setBaseURL(headerParamsStr);
					strcpy(m_baseUrl, LocationParamsStr);
				}
			}
			if (!reachedEndOfHeaders)break; // an error occurred

			// If we saw a "Content-Length:" header, then make sure that we have the amount of data that it specified:
			unsigned bodyOffset = nextLineStart - headerDataCopy;
			bodyStart = &fResponseBuffer[bodyOffset];
			numBodyBytes = fResponseBytesAlreadySeen - bodyOffset;
			if (contentLength > numBodyBytes) {
				// We need to read more data.  First, make sure we have enough space for it:
				unsigned numExtraBytesNeeded = contentLength - numBodyBytes;
				unsigned remainingBufferSize = responseBufferSize - fResponseBytesAlreadySeen;
				if (numExtraBytesNeeded > remainingBufferSize) {
					break;
				}

				delete[] headerDataCopy;
				//if (foundRequest != NULL) fRequestsAwaitingResponse.putAtHead(foundRequest);// put our request record back; we need it again
				return NOEnoughData; // We need to read more data
			}

			// We now have a complete response (including all bytes specified by the "Content-Length:" header, if any).
			char* responseEnd = bodyStart + contentLength;
			numExtraBytesAfterResponse = &fResponseBuffer[fResponseBytesAlreadySeen] - responseEnd;

			if(responseCode == 200){
				returnValue = ResponseSUCCESS;
				if(m_commandStatusInPlay == WAITFOR_RESPONSE)
					m_commandStatusInPlay = SUCCESS_RESPONSE;
				if ( m_iCommandStatus ==  DESCRIBECOMMAND) {
					if(contentTypeParamsStr != NULL){
						if ( numBodyBytes > 0 && strcmp(contentTypeParamsStr,"application/sdp") == 0)
							//if(!handleSDPResponse(bodyStart))break;
							handleSDPResponse(bodyStart);

						if ( numBodyBytes > 0 && strcmp(contentTypeParamsStr,"Application/Sdp") == 0)
							handleSDPResponse(bodyStart);

						if ( numBodyBytes > 0 && strcmp(contentTypeParamsStr,"application/SaveFileInfo") == 0)
							strcpy(m_pSaveFileInfo, bodyStart);
					}
				} else if ( m_iCommandStatus == SETUPCOMMAND ) {
					if(!handleSETUPResponse(transportParamsStr))break;
				} else if ( m_iCommandStatus == PLAYCOMMAND ) {
					m_iCommandStatus = PLAYCOMMANDED;
				} else if ( m_iCommandStatus == TEARDOWNCOMMAND ) {
				}
			} else if (responseCode == 401 && handleAuthenticationFailure(wwwAuthenticateParamsStr)) {
				// We need to resend the command, with an "Authorization:" header:
				needToResendCommand = True;
			} else if( (responseCode == 301 || responseCode == 302) && strlen(m_baseUrl) > 0 ){
				needToRedirectCommand = True;
			}
			if(responseCode == 563){
				bFullChannelResponse = True;
			}
			if(responseCode != 200 && m_commandStatusInPlay == WAITFOR_RESPONSE)
			{
				m_commandStatusInPlay = FAILED_RESPONSE;
				m_responseCodeInPlay = responseCode;
			}
			responseSuccess = True;
		} while (0);

		if ( numExtraBytesAfterResponse > 0 ) {

			char* responseEnd = &fResponseBuffer[fResponseBytesAlreadySeen - numExtraBytesAfterResponse];

			numBodyBytes -= numExtraBytesAfterResponse;
			if (numBodyBytes > 0) {
				char saved = *responseEnd;
				*responseEnd = '\0';
				bodyStart = strDup(bodyStart);
				*responseEnd = saved;
			}

			memmove(fResponseBuffer, responseEnd, numExtraBytesAfterResponse);
			fResponseBytesAlreadySeen = numExtraBytesAfterResponse;
			fResponseBufferBytesLeft = responseBufferSize - numExtraBytesAfterResponse;
			fResponseBuffer[numExtraBytesAfterResponse] = '\0';
		} else {
			resetResponseBuffer();
		}

		delete[] headerDataCopy;
		if (numExtraBytesAfterResponse > 0 && numBodyBytes > 0) delete[] bodyStart;
	} while (numExtraBytesAfterResponse > 0 && responseSuccess);
	if(needToResendCommand) return NeedToResendCommand;
	if(needToRedirectCommand) return NeedToRedirectCommand;
	if(bFullChannelResponse) return FullChannelResponse;
	if(!responseSuccess)return Failed;
	return returnValue;
	return ResponseSUCCESS;
}


Boolean CRTSPClient::handleSETUPResponse(char const* transportParamsStr)
{
	// Run through each of the parameters, looking for ones that we handle:
	int clientRTPPort = 0;
	int svrRTPPort = 0;

	char ClientAddressStr[256];
	memset(m_SrvAddressStr , 0 , 256);
	memset(ClientAddressStr , 0 , 256);

	Boolean isUMulticast = False;
	Boolean isRTP_UDP = False;
	Boolean isRTP_TCP = False;
	Boolean isRAW_TCP = False;

	char const* fields = transportParamsStr;
	char field[65536]; memset(field , 0, 65536);
	while (sscanf(fields, "%[^;]", field) == 1) {
		if (sscanf(field, "server_port=%hu", &svrRTPPort) == 1) {

		} else if (sscanf(field, "client_port=%hu", &clientRTPPort) == 1) {

		} else if (_strncasecmp(field, "source=", 7) == 0) {
			if(strlen(field + 7) < 256)strcpy(m_SrvAddressStr, field + 7);
		} else if (_strncasecmp(field, "destination=", 12) == 0) {
			if(strlen(field + 12) < 256)strcpy(ClientAddressStr, field + 12);
		} else if (strcmp(field, "unicast") == 0) {
			isUMulticast = True;
		} else if (strcmp(field, "RTP/AVP") == 0) {
			isRTP_UDP = True;
		} else if (strcmp(field, "RTP/AVP/UDP") == 0) {
			isRTP_UDP = True;
		} else if (strcmp(field, "RTP/AVP/TCP") == 0) {
			isRTP_TCP = True;
		}else if (strcmp(field, "RAW/RAW/TCP") == 0) {
	    	isRAW_TCP = True;
	    }
		fields += strlen(field);
		while (fields[0] == ';') ++fields; // skip over all leading ';' chars
		if (fields[0] == '\0') break;
	}
	if(isRAW_TCP || isRTP_TCP)
		return True;

	if( m_bSendingVideoSETUP )
	{
		m_SrvVideoUDPPort = svrRTPPort;
	}
	if( m_bSendingAudioSETUP )
	{
		m_SrvAudioUDPPort = svrRTPPort;
	}

	if (isUMulticast && isRTP_UDP && svrRTPPort && clientRTPPort /*&& strlen(m_ServerAddressStr) > 0*/)
		return True;
	else
		return False;
	return False;
}

Boolean CRTSPClient::handleSDPResponse(char * sdpBodyStr)
{
	bool bVideo = false;
	bool bAudio = false;

	Boolean reachedEndOfHeaders;
	char* lineStart = NULL;
	char* nextLineStart = getLine(sdpBodyStr);;
	const char* chParamsStr = NULL;
	const char* chControlStr = NULL;
	while (1) {
		reachedEndOfHeaders = True; // by default; may get changed below
		lineStart = nextLineStart;
		if (lineStart == NULL) break;

		nextLineStart = getLine(lineStart);
		if (lineStart[0] == '\0') break; // this is a blank line
		reachedEndOfHeaders = False;

		if (checkForHeader(lineStart, "i=", 2, chParamsStr)) {
		} else if (checkForHeader(lineStart, "m=video", 7, chControlStr)) {
			bVideo = true;
		} else if (checkForHeader(lineStart, "m=audio", 7, chControlStr)) {
			bAudio = true;
		} else if (checkForHeader(lineStart, "a=control:", 10, chControlStr)) {
			
			if( bVideo )
			{
				bVideo = false;
				m_VideoControlUrl = chControlStr;
				if( m_VideoControlUrl.length() > 4 &&
					m_VideoControlUrl[0]=='r' && 
					m_VideoControlUrl[1]=='t' && 
					m_VideoControlUrl[2]=='s' && 
					m_VideoControlUrl[3]=='p'
					)
				{
				}
				else
				{
					m_VideoControlUrl = m_url;
					m_VideoControlUrl += "/";
					m_VideoControlUrl += chControlStr;
				}
			}

			if( bAudio )
			{
				bAudio = false;
				m_AudioControlUrl = chControlStr;
				if( m_AudioControlUrl.length() > 4 &&
					m_AudioControlUrl[0]=='r' && 
					m_AudioControlUrl[1]=='t' && 
					m_AudioControlUrl[2]=='s' && 
					m_AudioControlUrl[3]=='p'
					)
				{
				}
				else
				{
					m_AudioControlUrl = m_url;
					m_AudioControlUrl += "/";
					m_AudioControlUrl += chControlStr;
				}
			}
		} else if (checkForHeader(lineStart, "Content-Base:", 13, chControlStr)) {
		}
	}
	if( bVideo && m_VideoControlUrl.length() <= 0 )
		m_VideoControlUrl = m_url;

	if (!reachedEndOfHeaders || chParamsStr == NULL)return Failed; // an error occurred
	std::string camerainfo = chParamsStr;
	int n = camerainfo.find_last_of('&');
	if (n <= 0 || n >= camerainfo.length() - 1)return False;
	int oldCameraID = m_cameraid;
	m_cameraid = atoi(camerainfo.substr(n + 1,camerainfo.length()).c_str());
	m_cameraname = camerainfo.substr(0,n);

	CConnectMC::OpenCameraPtr openCameraPtr_
		= g_connectMCptr_->FindOpenCameraByCameraID(oldCameraID);
	if( openCameraPtr_ != NULL){
		openCameraPtr_->ChangeCameraIDAndUrl( m_cameraid );
	}

	//m_log.Add("s_UpdateCameraNameCallBack = %d bengin", s_UpdateCameraNameCallBack);
	if(s_UpdateCameraNameCallBack != NULL)
		s_UpdateCameraNameCallBack(oldCameraID, m_cameraid, (unsigned char *)m_cameraname.c_str(), s_UpdateCameraCallBackUser);
	//m_log.Add("s_UpdateCameraNameCallBack end");

	return True;
}

Boolean CRTSPClient::handleAuthenticationFailure(char const* paramsStr) {
	if (paramsStr == NULL) return False; // There was no "WWW-Authenticate:" header; we can't proceed.

	// Fill in "fCurrentAuthenticator" with the information from the "WWW-Authenticate:" header:
	Boolean alreadyHadRealm = fCurrentAuthenticator.realm() != NULL;
	char* realm = strDupSize(paramsStr);
	char* nonce = strDupSize(paramsStr);
	Boolean success = True;
	if (sscanf(paramsStr, "Digest realm=\"%[^\"]\", nonce=\"%[^\"]\"", realm, nonce) == 2) {
		fCurrentAuthenticator.setRealmAndNonce(realm, nonce);
	} else if (sscanf(paramsStr, "Basic realm=\"%[^\"]\"", realm) == 1) {
		fCurrentAuthenticator.setRealmAndNonce(realm, NULL); // Basic authentication
	} else {
		success = False; // bad "WWW-Authenticate:" header
	}
	delete[] realm; delete[] nonce;

	if (alreadyHadRealm || fCurrentAuthenticator.username() == NULL || fCurrentAuthenticator.password() == NULL) {
		// We already had a 'realm', or don't have a username and/or password,
		// so the new "WWW-Authenticate:" header information won't help us.  We remain unauthenticated.
		success = False;
	}

	return success;
}

char* CRTSPClient::createAuthenticatorString(char const* cmd, char const* url) 
{
	Authenticator& auth = fCurrentAuthenticator; // alias, for brevity
	if (auth.realm() != NULL && auth.username() != NULL && auth.password() != NULL) {
		// We have a filled-in authenticator, so use it:
		char* authenticatorStr = NULL;
		if (auth.nonce() != NULL) { // Digest authentication
			char const* const authFmt =
				"Authorization: Digest username=\"%s\", realm=\"%s\", "
				"nonce=\"%s\", uri=\"%s\", response=\"%s\"\r\n";
			char const* response = auth.computeDigestResponse(cmd, url);
			unsigned authBufSize = strlen(authFmt)
				+ strlen(auth.username()) + strlen(auth.realm())
				+ strlen(auth.nonce()) + strlen(url) + strlen(response);
			authenticatorStr = new char[authBufSize];
			sprintf(authenticatorStr, authFmt,
				auth.username(), auth.realm(),
				auth.nonce(), url, response);
			auth.reclaimDigestResponse(response);
		}else
		{
			char const* const authFmt = "Authorization: Basic %s\r\n";
			unsigned usernamePasswordLength = strlen(auth.username()) + 1 + strlen(auth.password());
			char* usernamePassword = new char[usernamePasswordLength+1];
			sprintf(usernamePassword, "%s:%s", auth.username(), auth.password());

			char* response = base64Encode(usernamePassword, usernamePasswordLength);
			unsigned const authBufSize = strlen(authFmt) + strlen(response) + 1;
			authenticatorStr = new char[authBufSize];
			sprintf(authenticatorStr, authFmt, response);
			delete[] response; delete[] usernamePassword;
		}
		if( authenticatorStr == NULL )
			return strDup("");
		return authenticatorStr;
	}
	// We don't have a (filled-in) authenticator.
	return strDup("");
}

void CRTSPClient::resetResponseBuffer() {
	fResponseBytesAlreadySeen = 0;
	fResponseBufferBytesLeft = responseBufferSize;
	memset(fResponseBuffer, 0, responseBufferSize);
}

int CRTSPClient::sendSomeCommand(const char * url)
{
	m_icseq++;
	int i = sendOptionsCommand(url);
	if( i < 0 )return i;
	i = HandleIncomingData();
	if( i < 0 )return i;
	//int i = 0;

	m_icseq++;
	i = sendDescribeCommand(url);
	if( i < 0 )return i;
	i = HandleIncomingData();
	if( i < 0)return i;

	if(i == NeedToResendCommand)
	{
		m_icseq++;
		i = sendDescribeCommand(url);
		if( i < 0 )return i;
		i = HandleIncomingData();
		if( i < 0)return i;
	}

	if( i == NeedToResendCommand)
		return Failed;

	if( m_VideoControlUrl.length() > 0 )
	{
		m_bSendingVideoSETUP = true;

		m_icseq++;
		i = sendSetupCommand(m_VideoControlUrl.c_str());
		if(i < 0)return i;
		i = HandleIncomingData();
		if(i < 0)return i;

		m_bSendingVideoSETUP = false;
	}

	if( m_AudioControlUrl.length() > 0 )
	{
		m_bSendingAudioSETUP = true;

		m_icseq++;
		i = sendSetupCommand(m_AudioControlUrl.c_str());
		if(i < 0)return i;
		i = HandleIncomingData();
		if(i < 0)return i;

		m_bSendingAudioSETUP = false;
	}

	m_icseq++;
	return i;
}

int CRTSPClient::GetSaveFileInfo(char* ip, int port, const char * url, const char* Range_clock, char* resultStr)
{
	strcpy(this->m_url, url);

	int i = connectRtspSrv(ip,port);
	if( i < 0 )return i;

	m_icseq++;
	i = sendOptionsCommand(url);
	if( i < 0 )return i;
	i = HandleIncomingData();
	if( i < 0 )return i;

	m_icseq++;
	i = sendDescribeCommand(url, "SaveFileInfo", Range_clock);
	if( i < 0 )return i;
	i = HandleIncomingData();
	if( i < 0)return i;

	if(i == NeedToResendCommand)
	{
		m_icseq++;
		i = sendDescribeCommand(url, "SaveFileInfo", Range_clock);
		if( i < 0 )return i;
		i = HandleIncomingData();
		if( i < 0)return i;
	}

	if( i == NeedToResendCommand)
		return Failed;

	strcpy(resultStr, m_pSaveFileInfo);

	stopRTSPRequest();
	return i;
}

int CRTSPClient::connectRtspSrv(char* ip,int port)
{
	try
	{
		boost::asio::ip::tcp::endpoint endpoint(
			boost::asio::ip::address_v4::from_string(ip), port);
		rtsp_socket_.connect(endpoint);
		boost::asio::socket_base::receive_buffer_size recvbufsize(65536);
		rtsp_socket_.set_option(recvbufsize);

		m_brun = true;
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return ERROR_SOCKET_CONNECT;
	}
	return 0;
}

int CRTSPClient::RecvPlayResponse() //for TCP recv PLAY COMMAND
{
	try{
		int i = 0;
		for (i = 0; i < 2500; i++)
		{
			boost::asio::socket_base::bytes_readable command(true);
			rtsp_socket_.io_control(command);
			std::size_t bytes_readable = command.get();
			if( bytes_readable > 0 ) break;
			boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		}
		if( i == 2500 ) return ERROR_SOCKET_OVERTIME;

		char  buf[65536];
		memset(buf, 0, 65536);
		boost::system::error_code ec;
		int icur = 0;
		while(true)
		{
			if( icur >= 4 &&
				buf[icur - 4] == '\r' &&
				buf[icur - 3] == '\n' &&
				buf[icur - 2] == '\r' &&
				buf[icur - 1] == '\n')
			{
				memcpy((char*)&fResponseBuffer[fResponseBytesAlreadySeen], buf, icur);
				return handleResponseBytes(icur);
			}

			int bytesRead = rtsp_socket_.read_some( boost::asio::buffer(
				(char*)(buf + icur), 1), ec);
			if( ec || bytesRead <= 0 )return -2;
			icur += bytesRead;
			if(icur >= 65535)return -3;
		}
		return 0;
	}catch (std::exception& e){
		std::cerr << e.what() << std::endl;
		return ERROR_SOCKET_RECV;
	}
	return 0;
}

int CRTSPClient::stopRTSPRequest()
{
	if( !m_brun )return 0;
	m_brun = false;
	try{
		boost::system::error_code ec;
		sendTeardownCommand(m_url);
		rtsp_socket_.shutdown(boost::asio::socket_base::shutdown_both, ec);
		rtsp_socket_.close(ec);
		fCurrentAuthenticator.reset();
		resetResponseBuffer();
	}catch(...){}
	return 0;
}

bool CRTSPClient::parseRTSPURL(char const* url, char*& username, char*& password, char*& address, int& portNum, char*& urlSuffix)
{
	do {
		// Parse the URL as "rtsp://[<username>[:<password>]@]<server-address-or-name>[:<port>][/<stream-name>]"
		username = password = address = urlSuffix = NULL; // default return values
		portNum = 554; // default value

		char* prefix = "rtsp://";
		unsigned prefixLength = 7;
		if (_strncasecmp(url, prefix, prefixLength) != 0) {
			prefix = "onvif://";
			prefixLength = 8;
			if (_strncasecmp(url, prefix, prefixLength) != 0) 
				break;
		}

		unsigned const parseBufferSize = 100;
		char parseBuffer[parseBufferSize];
		char const* from = &url[prefixLength];

		// Check whether "<username>[:<password>]@" occurs next.
		// We do this by checking whether '@' appears before the end of the URL, or before the first '/'.
		char const* colonPasswordStart = NULL;
		char const* p;
		for (p = from; *p != '\0' && *p != '/'; ++p) {
			if (*p == ':' && colonPasswordStart == NULL) {
				colonPasswordStart = p;
			} else if (*p == '@') {
				// We found <username> (and perhaps <password>).  Copy them into newly-allocated result strings:
				if (colonPasswordStart == NULL) colonPasswordStart = p;

				char const* usernameStart = from;
				unsigned usernameLen = colonPasswordStart - usernameStart;
				username = new char[usernameLen + 1] ; // allow for the trailing '\0'
				for (unsigned i = 0; i < usernameLen; ++i) username[i] = usernameStart[i];
				username[usernameLen] = '\0';

				char const* passwordStart = colonPasswordStart;
				if (passwordStart < p) ++passwordStart; // skip over the ':'
				unsigned passwordLen = p - passwordStart;
				password = new char[passwordLen + 1]; // allow for the trailing '\0'
				for (unsigned j = 0; j < passwordLen; ++j) password[j] = passwordStart[j];
				password[passwordLen] = '\0';

				from = p + 1; // skip over the '@'
				break;
			}
		}

		// Next, parse <server-address-or-name>
		char* to = &parseBuffer[0];
		unsigned i;
		for (i = 0; i < parseBufferSize; ++i) {
			if (*from == '\0' || *from == ':' || *from == '/') {
				// We've completed parsing the address
				*to = '\0';
				break;
			}
			*to++ = *from++;
		}
		if (i == parseBufferSize) {
			//env.setResultMsg("URL is too long");
			break;
		}

		address = new char[strlen(parseBuffer) + 1];
		strcpy(address,parseBuffer);

		char nextChar = *from;
		if (nextChar == ':') {
			int portNumInt;
			if (sscanf(++from, "%d", &portNumInt) != 1) {
				//env.setResultMsg("No port number follows ':'");
				break;
			}
			if (portNumInt < 1 || portNumInt > 65535) {
				//env.setResultMsg("Bad port number");
				break;
			}
			portNum = (portNumBits)portNumInt;
			while (*from >= '0' && *from <= '9') ++from; // skip over port number
		}

		//// The remainder of the URL is the suffix:
		//if (urlSuffix != NULL) *urlSuffix = from;
		urlSuffix = new char[ strlen(from) + 1];
		strcpy(urlSuffix,from + 1);

		return true;
	} while (0);

	return false;
}