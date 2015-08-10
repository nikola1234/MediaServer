#include "string.h"
#include "stdio.h"

#include "Sender.h"
#include "RTSPClientSession.h"
#include "ServerMediasession.h"
#include "MRServerMediasession.h"

extern live555RTSP::RTSPServer* g_pRTSPServer;

//#define  DEBUG
namespace live555RTSP
{

static char* getLine(char* startOfLine);
unsigned int RTSPClientSession::s_fakeClientID = 1;

RTSPClientSession::RTSPClientSession(RTSPServer* ourServer, unsigned sessionId)
	:fOurServer(ourServer),fOurSessionId(sessionId),
	fIsMulticast(False), fSessionIsActive(True), fStreamAfterSETUP(False),
	fTCPStreamIdCount(0), fNumStreamStates(0), fRecursionCount(0),
	rtsp_socket_(ourServer->Get_io_service()),
	stop_(true),
	fstreamingMode(RTP_UDP),
	m_token(TOKEN_NULL),
	m_clientid(0),
	m_mediaid(0),
	m_UserLevel(0),
	m_bIsRTSP(true),
	m_bIsDownLoad(false)
{
	resetRequestBuffer();
	if(s_fakeClientID++ >= 16777216){ //2^24
		s_fakeClientID = 1;
		fOurServer->m_log.Add("sclientid_ too max error!");
		//exit(0);
	}
	m_icseq = rand()%1000000;
	memset(m_version, 0, 256);
	memset(m_szUserID, 0, 64);
	memset(m_szUserName, 0, 64);
	memset(m_strErrorInfo, 0, 256);
	memset(m_szBase64UserWithLevel, 0, 256);
}

RTSPClientSession::~RTSPClientSession(void)
{
	StopFromInner();
	fOurServer->m_log.Add("RTSPClientSession::~RTSPClientSession fOurSessionId = %d", fOurSessionId);
}

int RTSPClientSession::Start()
{
	stop_ = false;
	int iret = -1;
	incomingRequestHandler();
	iret = 0;

	fOurServer->m_log.Add("RTSPClientSession::Start fOurSessionId = %d", fOurSessionId);
	return iret;
}

int RTSPClientSession::StopFromInner()
{
	if(stop_)return 0;
	stop_ = true;

	//释放有严格顺序！！！StopFromInner() 有问题！(Sender.socket)
    fOurServer->RemoveRTSPClientSession(shared_from_this());
	//fOurServer->RemoveRTSPClientBySessionID(fOurSessionId);

	if( fOurServerMediaSessionPtr != NULL ){
		fOurServerMediaSessionPtr->RemoveRTPPackSenderBySessionID(fOurSessionId);
		if( fOurServerMediaSessionPtr->GetSenderCountNoLock() > 0 )
			fOurServerMediaSessionPtr->CloseClientSessionByUserName( m_szUserName );
		fOurServer->TryRemoveServerMediaSession(fOurServerMediaSessionPtr);
	}
	//fOurServer->m_log.Add("RTSPClientSession::StopFromInner id=%d", fOurSessionId);
	boost::system::error_code ec;
	rtsp_socket_.shutdown(boost::asio::socket_base::shutdown_both,ec);
	rtsp_socket_.close(ec);
	return 0;
}

int RTSPClientSession::Stop()
{
	if(stop_)return 0;
	stop_ = true;
	if( fOurServerMediaSessionPtr != NULL ){
		fOurServerMediaSessionPtr->RemoveRTPPackSenderBySessionID(fOurSessionId);
		fOurServer->TryRemoveServerMediaSession(fOurServerMediaSessionPtr);
	}
	boost::system::error_code ec;
	rtsp_socket_.shutdown(boost::asio::socket_base::shutdown_both, ec);
	rtsp_socket_.close(ec);
	return 0;
}

int RTSPClientSession::SendANNOUNCEMessage(int iCameraID, const char* chUnitName, const char* chCameraName)
{
	m_icseq++;

	if ( fOurServerMediaSessionPtr == NULL ) return -1;

	char* sdpDescription = NULL;
	//Then, assemble a SDP description for this session:
	sdpDescription = fOurServerMediaSessionPtr->generateSDPDescription(iCameraID, chUnitName, chCameraName);

	if (sdpDescription == NULL) return -1;

	unsigned sdpDescriptionSize = strlen(sdpDescription);

	snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"ANNOUNCE rtsp://127.0.0.1/ RTSP/1.0\r\n"
		"CSeq: %d\r\n"
		"%s"
		"Session: %08X"
		"Content-Type: application/sdp\r\n"
		"Content-Length: %d\r\n\r\n"
		"%s",
		m_icseq,
		dateHeader(),
		fOurSessionId,
		sdpDescriptionSize,
		sdpDescription);
	delete[] sdpDescription;

	try {
		rtsp_socket_.async_send(
			boost::asio::buffer(fResponseBuffer,strlen((char*)fResponseBuffer)),
			boost::bind(&RTSPClientSession::HandleSendRTSPResponse,shared_from_this(),
			boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));

		//int ilen = strlen((char*)fResponseBuffer), icur = 0;
		//while(ilen > icur){
		//	int isend = rtsp_socket_.send( boost::asio::buffer(fResponseBuffer + icur , ilen - icur) );
		//	if(isend <= 0){ Stop(); break; }
		//	icur += isend;
		//}
		return 0;

	}catch (...){ /*StopFromInner();*/ }

	return -1;
}

void RTSPClientSession::incomingRequestHandler()
{
	if (fRequestBufferBytesLeft <= 0){
		StopFromInner();
		return;
	}
	try {
		rtsp_socket_.async_receive(
			boost::asio::buffer(&fRequestBuffer[fRequestBytesAlreadySeen], fRequestBufferBytesLeft),
			boost::bind(&RTSPClientSession::HandleRecvRTSPRequest,
			shared_from_this(),boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
	catch (...){ StopFromInner(); }
}

void RTSPClientSession::HandleRecvRTSPRequest(const boost::system::error_code& error, int recvsizes)
{
	if(stop_)return;
	if (!error)
	{
		int bend = false;
		do 
		{
			bend = false;

			if(!(fstreamingMode == RTP_TCP &&
				fRequestBuffer[0] == '$') )
				break;

			fRequestBufferBytesLeft -= recvsizes;
			fRequestBytesAlreadySeen += recvsizes;

			if( recvsizes < 4 ){
				incomingRequestHandler();
				return;
			}

			if( fRequestBuffer[1] != 0x01 )
				break;

			ServerMediasession::SenderPtr sender_ptr;
			if (fOurServerMediaSessionPtr != NULL)
				sender_ptr = fOurServerMediaSessionPtr->GetRTPPackSenderBySessionID(fOurSessionId);

			unsigned int rtcplen = fRequestBuffer[2]*256 + fRequestBuffer[3];
			if ( fRequestBytesAlreadySeen == rtcplen + 4 )
			{
				if (sender_ptr != NULL){
					sender_ptr->RecvRTCPData( fRequestBuffer + 4 ,rtcplen );
				}
				incomingRequestHandler();
				return;
			}
			else if (fRequestBytesAlreadySeen > rtcplen + 4)
			{
				if (sender_ptr != NULL){
					sender_ptr->RecvRTCPData( fRequestBuffer + 4 ,rtcplen );
				}

				memmove(fRequestBuffer, fRequestBuffer + rtcplen + 4, fRequestBytesAlreadySeen - rtcplen - 4);
				recvsizes = fRequestBytesAlreadySeen - rtcplen - 4;
				resetRequestBuffer();

				if(fRequestBuffer[0] == '$')
					bend = true;
				else
					break;
			}
			else if ((unsigned int)recvsizes < rtcplen + 4)
			{
				incomingRequestHandler();
				return;
			}

		} while (bend);

		if( m_bIsRTSP ) {
			handleRequestBytes(recvsizes);
			incomingRequestHandler();
		} else {
			incomingRequestHandler();
		}

	}else{
		std::cerr<< "RTSPClientSession::HandleRecvRTSPRequest : " << error.message() << std::endl;
		StopFromInner();
	}
}

void RTSPClientSession::HandleSendRTSPResponse(const boost::system::error_code& error, int sendsizes)
{
	if(stop_)return;

	if (!error){ }
	else{
		StopFromInner();
	}
}

void RTSPClientSession::resetRequestBuffer() {
	fRequestBytesAlreadySeen = 0;
	fRequestBufferBytesLeft = sizeof fRequestBuffer;
	fLastCRLF = &fRequestBuffer[-3]; // hack: Ensures that we don't think we have end-of-msg if the data starts with <CR><LF>
	//fBase64RemainderCount = 0;
}

void RTSPClientSession::handleRequestBytes(int newBytesRead) {
	int numBytesRemaining = 0;
	//++fRecursionCount;
	bool isResponse = false;
	bool isHttpRequest = false;
	do {
		if (newBytesRead <= 0 || (unsigned)newBytesRead >= fRequestBufferBytesLeft) {
			// Either the client socket has died, or the request was too big for us.
			// Terminate this connection:
			fSessionIsActive = False;
			break;
		}

		if ( !fSessionIsActive )
			break;

		Boolean endOfMsg = False;
		unsigned char* ptr = &fRequestBuffer[fRequestBytesAlreadySeen];

#ifdef DEBUG
		ptr[newBytesRead] = '\0';
		fprintf(stderr, "RTSPClientSession[%p]::handleRequestBytes() %s %d new bytes:%s\n",
			this, numBytesRemaining > 0 ? "processing" : "read", newBytesRead, ptr);
#endif

		// Look for the end of the message: <CR><LF><CR><LF>
		unsigned char *tmpPtr = fLastCRLF + 2;
		if (tmpPtr < fRequestBuffer) tmpPtr = fRequestBuffer;
		while (tmpPtr < &ptr[newBytesRead-1]) {
			if (*tmpPtr == '\r' && *(tmpPtr+1) == '\n') {
				if (tmpPtr - fLastCRLF == 2) { // This is it:
					endOfMsg = True;
					break;
				}
				fLastCRLF = tmpPtr;
			}
			++tmpPtr;
		}

		fRequestBufferBytesLeft -= newBytesRead;
		fRequestBytesAlreadySeen += newBytesRead;

		if (!endOfMsg) break; // subsequent reads will be needed to complete the request

		// Parse the request string into command name and 'CSeq', then handle the command:
		fRequestBuffer[fRequestBytesAlreadySeen] = '\0';
		char cmdName[RTSP_PARAM_STRING_MAX];
		char urlPreSuffix[RTSP_PARAM_STRING_MAX];
		char urlSuffix[RTSP_PARAM_STRING_MAX];
		char cseq[RTSP_PARAM_STRING_MAX];
		unsigned contentLength;
		*fLastCRLF = '\0'; // temporarily, for parsing
		Boolean parseSucceeded = parseRTSPRequestString((char*)fRequestBuffer, fRequestBytesAlreadySeen,
			cmdName, sizeof cmdName,
			urlPreSuffix, sizeof urlPreSuffix,
			urlSuffix, sizeof urlSuffix,
			cseq, sizeof cseq,
			contentLength);
		*fLastCRLF = '\r';
		if (parseSucceeded) {
			// If there was a "Content-Length:" header, then make sure we've received all of the data that it specified:
			if (ptr + newBytesRead < tmpPtr + 2 + contentLength) break; // we still need more data; subsequent reads will give it to us 

			if (strcmp(cmdName, "OPTIONS") == 0) {
				handleCmd_OPTIONS(cseq);
			} else if (strcmp(cmdName, "DESCRIBE") == 0) {
				handleCmd_DESCRIBE(cseq, urlPreSuffix, urlSuffix, (char const*)fRequestBuffer);
			} else if (strcmp(cmdName, "SETUP") == 0) {
				handleCmd_SETUP(cseq, urlPreSuffix, urlSuffix, (char const*)fRequestBuffer);
			} else if (strcmp(cmdName, "TEARDOWN") == 0
				|| strcmp(cmdName, "PLAY") == 0
				|| strcmp(cmdName, "PAUSE") == 0
				|| strcmp(cmdName, "GET_PARAMETER") == 0
				|| strcmp(cmdName, "SET_PARAMETER") == 0) {
					handleCmd_withinSession(cmdName, urlPreSuffix, urlSuffix, cseq, (char const*)fRequestBuffer);
			} else {
				handleCmd_notSupported(cseq);
			}
		} else {

			// The request was not (valid) RTSP, but check for a special case: HTTP commands (for setting up RTSP-over-HTTP tunneling):
			char sessionCookie[RTSP_PARAM_STRING_MAX];
			char acceptStr[RTSP_PARAM_STRING_MAX];
			*fLastCRLF = '\0'; // temporarily, for parsing
			parseSucceeded = parseHTTPRequestString(
				(char*)fRequestBuffer, fRequestBytesAlreadySeen,
				cmdName, sizeof cmdName,
				urlSuffix, sizeof urlPreSuffix,
				sessionCookie, sizeof sessionCookie,
				acceptStr, sizeof acceptStr);
			*fLastCRLF = '\r';
			if (parseSucceeded) {
				isHttpRequest = true;
				// Check that the HTTP command is valid for RTSP-over-HTTP tunneling: There must be a 'session cookie'.
				Boolean isValidHTTPCmd = True;
				if (strcmp(cmdName, "GET") == 0) {
					handleHTTPCmd_GET(urlSuffix);
				} else if (strcmp(cmdName, "POST") == 0) {
					isValidHTTPCmd = False;
				} else {
					isValidHTTPCmd = False;
				}
				if (!isValidHTTPCmd) {
					handleHTTPCmd_notSupported();
				}
			}else
			{
				int responseCode = 0;
				char* line = getLine((char *)fRequestBuffer);
				if (line != NULL &&
					sscanf(line, "RTSP/%*s%u", &responseCode) != 1 &&
					sscanf(line, "HTTP/%*s%u", &responseCode) != 1){
						isResponse = true;
				}

				handleCmd_bad(cseq);
			}
		}
		//发送响应函数
		if( !isResponse ){
			if( !isHttpRequest )
			{
				try{
					rtsp_socket_.async_send(
						boost::asio::buffer(fResponseBuffer,strlen((char*)fResponseBuffer)),
						boost::bind(&RTSPClientSession::HandleSendRTSPResponse,shared_from_this(),
						boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
				}catch (...){
					StopFromInner();
				}
			}
			else
			{
				try
				{
					int ilen = strlen((char*)fResponseBuffer), icur = 0;
					while(ilen > icur){
						int isend = rtsp_socket_.send( boost::asio::buffer(fResponseBuffer + icur , ilen - icur) );
						if(isend <= 0){ Stop(); break; }
						icur += isend;
					}
				} catch (...){ }
				StopFromInner();//http 短连接 
			}
		}

		if (strcmp(cmdName, "PLAY") == 0 && fOurServerMediaSessionPtr != NULL)
		{
			ServerMediasession::SenderPtr sender_ptr = 
				fOurServerMediaSessionPtr->GetRTPPackSenderBySessionID(fOurSessionId);
			if(sender_ptr != NULL)
				sender_ptr->SetSenderState(CSender::WAITFOR_SENDHAED);
		}

		if (strcmp(cmdName, "SETUP") == 0 && fStreamAfterSETUP) {
			// The client has asked for streaming to commence now, rather than after a
			// subsequent "PLAY" command.  So, simulate the effect of a "PLAY" command:
			handleCmd_withinSession("PLAY", urlPreSuffix, urlSuffix, cseq,
				(char const*)fRequestBuffer);
		}

		// Check whether there are extra bytes remaining in the buffer, after the end of the request (a rare case).
		// If so, move them to the front of our buffer, and keep processing it, because it might be a following, pipelined request.
		unsigned requestSize = (fLastCRLF+4-fRequestBuffer) + contentLength;
		numBytesRemaining = fRequestBytesAlreadySeen - requestSize;
		resetRequestBuffer(); // to prepare for any subsequent request

		if (numBytesRemaining > 0) {
			memmove(fRequestBuffer, &fRequestBuffer[requestSize], numBytesRemaining);
			newBytesRead = numBytesRemaining;
		}
	} while (numBytesRemaining > 0);

	//--fRecursionCount;
	if (!fSessionIsActive&&!stop_)
	{
		StopFromInner();
	}
}

// Handler routines for specific RTSP commands:
static char const* allowedCommandNames
	= "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER";

void RTSPClientSession::handleCmd_badMedia(char const* /*cseq*/) {
	// Don't do anything with "cseq", because it might be nonsense
	if( strlen(m_strErrorInfo) <= 0 )
		snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 561 Media Open Error\r\n%sAllow: %s\r\n\r\n",
		dateHeader(), allowedCommandNames);
	else
		snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 561 Media Open Error\r\n%sOpenErrorInfo:%s\r\nAllow: %s\r\n\r\n",
		dateHeader(), m_strErrorInfo, allowedCommandNames);
}

void RTSPClientSession::handleCmd_FullChannel(char const* cseq, std::list<int>& cameralist) {
	// Don't do anything with "cseq", because it might be nonsense
	std::string camerastr;
	std::list<int>::iterator it = cameralist.begin();
	for (; it != cameralist.end(); it++)
	{
		char cameraid[256];memset(cameraid, 0, 256);
		snprintf(cameraid, 256, "%d;",(*it));
		camerastr += cameraid; 
	}
	snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 563 VnmpFullChannel Error\r\nCSeq: %s\r\n%sVnmpFullCameraID:%s\r\nAllow: %s\r\n\r\n",
		cseq, dateHeader(), camerastr.c_str(), allowedCommandNames);
	//this->fOurServer->m_log.Add("RTSPClientSession::handleCmd_FullChannel fResponseBuffer = %s", fResponseBuffer);
}

void RTSPClientSession::handleCmd_bad(char const* /*cseq*/) {
	// Don't do anything with "cseq", because it might be nonsense
	snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 400 Bad Request\r\n%sAllow: %s\r\n\r\n",
		dateHeader(), allowedCommandNames);
}

void RTSPClientSession::handleCmd_notSupported(char const* cseq) {
	snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 405 Method Not Allowed\r\nCSeq: %s\r\n%sAllow: %s\r\n\r\n",
		cseq, dateHeader(), allowedCommandNames);
}

void RTSPClientSession::handleCmd_notFound(char const* cseq) {
	snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 404 Stream Not Found\r\nCSeq: %s\r\n%s\r\n\r\n",
		cseq, dateHeader());
	fSessionIsActive = False; // triggers deletion of ourself after responding
}

void RTSPClientSession::handleCmd_unsupportedTransport(char const* cseq) {
	snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 461 Unsupported Transport\r\nCSeq: %s\r\n%s\r\n\r\n",
		cseq, dateHeader());
	fSessionIsActive = False; // triggers deletion of ourself after responding
}

void RTSPClientSession::handleCmd_OPTIONS(char const* cseq) {
	snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sPublic: %s\r\n\r\n",
		cseq, dateHeader(), allowedCommandNames);
}

static Boolean parseAcceptParamHeader(char const* buf, char* contextType)
{
	// First, find "Authorization:"
	while (1) {
		if (*buf == '\0') return False; // not found
		if (_strncasecmp(buf, "Accept:", 7) == 0) break;
		++buf;
	}

	// Then, run through each of the fields, looking for ones we handle:
	char const* fields = buf + 7;
	while (*fields == ' ') ++fields;
	if (strlen(fields) >= 256) return False;
	//strcpy(contextType, fields);
	if(sscanf(fields, "%[^;\r\n]", contextType) == 1)
		return True;
	return False;
}

static Boolean parseUserAgentHeader(char const* buf, char* version, int& token, int& clientid)
{
	// First, find "Authorization:"
	while (1) {
		if (*buf == '\0') return False; // not found
		if (_strncasecmp(buf, "User-Agent:VNMPNetSDK", 21) == 0) break;
		++buf;
	}

	// Then, run through each of the fields, looking for ones we handle:
	char const* fields = buf + 21;
	while (*fields == ' ') ++fields;
	char UserAgent[256]; memset(UserAgent, 0, 256);
	if (strlen(fields) >= 256) return False;

	if(sscanf(fields, "%[^;\r\n]", UserAgent) != 1)
		return False;
	//strcpy(UserAgent, fields);

	std::string strUserAgent = UserAgent;

	std::string::size_type t1 = strUserAgent.find_first_of(' ');
	std::string::size_type t2 = strUserAgent.find_last_of(' ');
	if(t1 == std::string::npos || t2 == std::string::npos || t1 == t2 ||t2 >= (strUserAgent.length() - 1))return False;

	strcpy(version, strUserAgent.substr(0, t1).c_str());
	std::string str = strUserAgent.substr( t1 + 1, ( t2 - t1 ) );
	str = strUserAgent.substr(t2 + 1, strUserAgent.length() - t2 );

	token = atoi(strUserAgent.substr( t1 + 1, ( t2 - t1 ) ).c_str());
	clientid = atoi(strUserAgent.substr(t2 + 1, strUserAgent.length() - t2 ).c_str());

	return True;
}

void RTSPClientSession
	::handleCmd_DESCRIBE(char const* cseq,
	char const* urlPreSuffix, char const* urlSuffix,
	char const* fullRequestStr)
{
	char* sdpDescription = NULL;
	do {
		char urlTotalSuffix[RTSP_PARAM_STRING_MAX];
		if (strlen(urlPreSuffix) + strlen(urlSuffix) + 2 > sizeof urlTotalSuffix) {
			handleCmd_bad(cseq);
			break;
		}
		urlTotalSuffix[0] = '\0';
		if (urlPreSuffix[0] != '\0') {
			strcat(urlTotalSuffix, urlPreSuffix);
			strcat(urlTotalSuffix, "/");
		}
		strcat(urlTotalSuffix, urlSuffix);
		m_CameraID = atoi(urlSuffix);

		Boolean bParseUserAgent =  parseUserAgentHeader(fullRequestStr, m_version, m_token, m_clientid);
		if( bParseUserAgent && (m_token == TOKEN_AUTH || m_token == TOKEN_RECORDBYUSER) ){
			if (!authenticationOK("DESCRIBE", cseq, urlTotalSuffix, fullRequestStr)) break;
		}

		char contextType[256]; memset(contextType, 0, 256);
		char clock_str[256]; memset(clock_str, 0, 256);
		parseAcceptParamHeader(fullRequestStr, contextType);

		//生成SDP
		std::list<int> cameralist;
		fOurServerMediaSessionPtr = fOurServer->lookupServerMediaSession( urlTotalSuffix/*urlSuffix*/, cameralist, m_channleLevels, m_UserLevel);
		if( fOurServerMediaSessionPtr == NULL && cameralist.size() > 0  ){
			handleCmd_FullChannel(cseq, cameralist);
			break;
		}
		if ( fOurServerMediaSessionPtr == NULL ) {
			strcpy(m_strErrorInfo,"对不起！配置错误！");
			handleCmd_badMedia(cseq);
			break;
		}

		//if( bParseUserAgent && m_token == TOKEN_AUTH )
		//	fOurServerMediaSessionPtr->SetUserLevel(m_UserLevel);
		m_mediaid = fOurServerMediaSessionPtr->GetMediaID();
		//this->fOurServer->m_log.Add("RTSPClientSession::handleCmd_DESCRIBE contextType = %s", contextType);
		if(strcmp(contextType, "application/SaveFileInfo") == 0)
		{
			double rangeStart = 0.0, rangeEnd = 0.0;
			Boolean sawRangeHeader = parseRangeHeader(fullRequestStr, rangeStart, rangeEnd, clock_str);
			this->fOurServer->m_log.Add("RTSPClientSession::handleCmd_DESCRIBE clock_str = %s sawRangeHeader = %d", 
				clock_str, sawRangeHeader);
			if( sawRangeHeader && !fOurServerMediaSessionPtr->IsLive() )
			{
				MRServerMediasession* pMedia = (MRServerMediasession*)(fOurServerMediaSessionPtr.get());
				sdpDescription = pMedia->GetSaveFileInfo(clock_str);
				if(strlen(sdpDescription) <= 0){
					delete[] sdpDescription;
					sdpDescription = NULL;
				} else {
					sprintf(sdpDescription, "%s\r\n\r\n", sdpDescription);
				}
				this->fOurServer->m_log.Add("MRServerMediasession::GetSaveFileInfo = %s", sdpDescription);
			}
		} else {
			//Then, assemble a SDP description for this session:
			sdpDescription = fOurServerMediaSessionPtr->generateSDPDescription();
		}

		if (sdpDescription == NULL) {
			// This usually means that a file name that was specified for a
			// "ServerMediaSubsession" does not exist.
			snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
				"RTSP/1.0 404 File Not Found, Or In Incorrect Format\r\n"
				"CSeq: %s\r\n"
				"%s\r\n",
				cseq,
				dateHeader());
			break;
		}
		unsigned sdpDescriptionSize = strlen(sdpDescription);

		// Also, generate our RTSP URL, for the "Content-Base:" header
		// (which is necessary to ensure that the correct URL gets used in
		// subsequent "SETUP" requests).
		snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
			"RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
			"%s"
			"Content-Type: %s\r\n"
			"Content-Length: %d\r\n\r\n"
			"%s",
			cseq,
			dateHeader(),
			contextType,
			sdpDescriptionSize,
			sdpDescription);
	} while (0);

	delete[] sdpDescription;
}

static Boolean parseAuthorizationHeader(char const* buf,
	char const*& username,
	char const*& realm,
	char const*& nonce, char const*& uri,
	char const*& response)
{
	// Initialize the result parameters to default values:
	username = realm = nonce = uri = response = NULL;

	// First, find "Authorization:"
	while (1) {
		if (*buf == '\0') return False; // not found
		if (_strncasecmp(buf, "Authorization: Digest ", 22) == 0) break;
		++buf;
	}

	// Then, run through each of the fields, looking for ones we handle:
	char const* fields = buf + 22;
	while (*fields == ' ') ++fields;
	char* parameter = strDupSize(fields);
	char* value = strDupSize(fields);
	while (1) {
		value[0] = '\0';
		if (sscanf(fields, "%[^=]=\"%[^\"]\"", parameter, value) != 2 &&
			sscanf(fields, "%[^=]=\"\"", parameter) != 1) {
				break;
		}
		if (strcmp(parameter, "username") == 0) {
			username = strDup(value);
		} else if (strcmp(parameter, "realm") == 0) {
			realm = strDup(value);
		} else if (strcmp(parameter, "nonce") == 0) {
			nonce = strDup(value);
		} else if (strcmp(parameter, "uri") == 0) {
			uri = strDup(value);
		} else if (strcmp(parameter, "response") == 0) {
			response = strDup(value);
		}

		fields += strlen(parameter) + 2 /*="*/ + strlen(value) + 1 /*"*/;
		while (*fields == ',' || *fields == ' ') ++fields;
		// skip over any separating ',' and ' ' chars
		if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
	}
	delete[] parameter; delete[] value;
	return True;
}

Boolean RTSPClientSession
	::authenticationOK(char const* cmdName, char const* cseq,
	char const* urlSuffix, char const* fullRequestStr)
{
	// If we weren't set up with an authentication database, we're OK:
	//if (fOurServer.fAuthDB == NULL) return True;
	char const* username = NULL; char const* realm = NULL; char const* nonce = NULL;
	char const* uri = NULL; char const* response = NULL;

	Boolean success = False;
	do {
		// To authenticate, we first need to have a nonce set up
		// from a previous attempt:
		if (fCurrentAuthenticator.nonce() == NULL) break;

		// Next, the request needs to contain an "Authorization:" header,
		// containing a username, (our) realm, (our) nonce, uri,
		// and response string:
		if (!parseAuthorizationHeader(fullRequestStr,
			username, realm, nonce, uri, response)
			|| username == NULL
			|| realm == NULL || strcmp(realm, fCurrentAuthenticator.realm()) != 0
			|| nonce == NULL || strcmp(nonce, fCurrentAuthenticator.nonce()) != 0
			|| uri == NULL || response == NULL) {
				break;
		}
		success = True;//默认验证成功

		//if( strlen(username) < 64 ) strcpy(m_szUserName, username);
		//char passwd[128]; memset(passwd, 0, 128);
		//bool bRet = fOurServer->GetDevInfo().GetDataInfoPtr()->GetUserIDAndPassWD(username, passwd, m_szUserID);
		//if( !bRet ) break;
		//fCurrentAuthenticator.setUsernameAndPassword(username, passwd, False /*passwd_md5, True*/);
		//char const* ourResponse = fCurrentAuthenticator.computeDigestResponse(cmdName, uri);
		////取消比较
		////success = (strcmp(ourResponse, response) == 0);
		//fCurrentAuthenticator.reclaimDigestResponse(ourResponse); //释放ourResponse
	
	} while (0);

	delete[] (char*)username; delete[] (char*)realm; delete[] (char*)nonce;
	delete[] (char*)uri; delete[] (char*)response;
	if (success) return True;

	// If we get here, there was some kind of authentication failure.
	// Send back a "401 Unauthorized" response, with a new random nonce:
	fCurrentAuthenticator.setRealmAndRandomNonce("VNMP2013"/*fOurServer.fAuthDB->realm()*/);
	snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 401 Unauthorized\r\n"
		"CSeq: %s\r\n"
		"%s"
		"WWW-Authenticate: Digest realm=\"%s\", nonce=\"%s\"\r\n\r\n",
		cseq,
		dateHeader(),
		fCurrentAuthenticator.realm(), fCurrentAuthenticator.nonce());
	return False;
}

static void parseTransportHeader(char const* buf,
	StreamingMode& streamingMode,
	char*& streamingModeString,
	char*& destinationAddressStr,
	u_int8_t& destinationTTL,
	portNumBits& clientRTPPortNum, // if UDP
	portNumBits& clientRTCPPortNum, // if UDP
	unsigned char& rtpChannelId, // if TCP
	unsigned char& rtcpChannelId // if TCP
	) {
		// Initialize the result parameters to default values:
		streamingMode = RTP_UDP;
		streamingModeString = NULL;
		destinationAddressStr = NULL;
		destinationTTL = 255;
		clientRTPPortNum = 0;
		clientRTCPPortNum = 1;
		rtpChannelId = rtcpChannelId = 0xFF;

		portNumBits p1, p2;
		unsigned ttl, rtpCid, rtcpCid;

		// First, find "Transport:"
		while (1) {
			if (*buf == '\0') return; // not found
			if (*buf == '\r' && *(buf+1) == '\n' && *(buf+2) == '\r') return; // end of the headers => not found
			if (_strncasecmp(buf, "Transport: ", 11) == 0) break;
			++buf;
		}

		// Then, run through each of the fields, looking for ones we handle:
		char const* fields = buf + 11;
		char* field = strDupSize(fields);
		while (sscanf(fields, "%[^;\r\n]", field) == 1) {
			if (strcmp(field, "RTP/AVP/TCP") == 0) {
				streamingMode = RTP_TCP;
			} else if (strcmp(field, "RAW/RAW/UDP") == 0 ||
				strcmp(field, "MP2T/H2221/UDP") == 0) {
					streamingMode = RAW_UDP;
					streamingModeString = strDup(field);
			}else if( strcmp(field, "RAW/RAW/TCP") == 0 ){
				streamingMode = RAW_TCP;
			} else if (_strncasecmp(field, "destination=", 12) == 0) {
				delete[] destinationAddressStr;
				destinationAddressStr = strDup(field+12);
			} else if (sscanf(field, "ttl%u", &ttl) == 1) {
				destinationTTL = (u_int8_t)ttl;
			} else if (sscanf(field, "client_port=%hu-%hu", &p1, &p2) == 2) {
				clientRTPPortNum = p1;
				clientRTCPPortNum = streamingMode == RAW_UDP ? 0 : p2; // ignore the second port number if the client asked for raw UDP
			} else if (sscanf(field, "client_port=%hu", &p1) == 1) {
				clientRTPPortNum = p1;
				clientRTCPPortNum = streamingMode == RAW_UDP ? 0 : p1 + 1;
			} else if (sscanf(field, "interleaved=%u-%u", &rtpCid, &rtcpCid) == 2) {
				rtpChannelId = (unsigned char)rtpCid;
				rtcpChannelId = (unsigned char)rtcpCid;
			}

			fields += strlen(field);
			while (*fields == ';') ++fields; // skip over separating ';' chars
			if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
		}
		delete[] field;
}

static char* getLine(char* startOfLine) {
	// returns the start of the next line, or NULL if none.  Note that this modifies the input string to add '\0' characters.
	for (char* ptr = startOfLine; *ptr != '\0'; ++ptr) {
		// Check for the end of line: \r\n (but also accept \r or \n by itself):
		if (*ptr == '\r' || *ptr == '\n') {
			// We found the end of the line
			if (*ptr == '\r') {
				*ptr++ = '\0';
				if (*ptr == '\n') ++ptr;
			} else {
				*ptr++ = '\0';
			}
			return ptr;
		}
	}
	return NULL;
}

static Boolean parsePlayNowHeader(char const* buf) {
	// Find "x-playNow:" header, if present
	while (1) {
		if (*buf == '\0') return False; // not found
		if (_strncasecmp(buf, "x-playNow:", 10) == 0) break;
		++buf;
	}
	return True;
}

void RTSPClientSession
	::handleCmd_SETUP(char const* cseq,
	char const* urlPreSuffix, char const* urlSuffix,
	char const* fullRequestStr) 
{
	// Normally, "urlPreSuffix" should be the session (stream) name, and "urlSuffix" should be the subsession (track) name.
	// However (being "liberal in what we accept"), we also handle 'aggregate' SETUP requests (i.e., without a track name),
	// in the special case where we have only a single track.  I.e., in this case, we also handle:
	//    "urlPreSuffix" is empty and "urlSuffix" is the session (stream) name, or
	//    "urlPreSuffix" concatenated with "urlSuffix" (with "/" inbetween) is the session (stream) name.
	char const* streamName = urlPreSuffix; // in the normal case
	char const* trackId = urlSuffix; // in the normal case

	do {
		// First, make sure the specified stream name exists:

		if (fOurServerMediaSessionPtr == NULL) {
			handleCmd_notFound(cseq);
			break;
		}

		// Look for a "Transport:" header in the request string, to extract client parameters:
		StreamingMode streamingMode;
		char* streamingModeString = NULL; // set when RAW_UDP streaming is specified
		char* clientsDestinationAddressStr;
		u_int8_t clientsDestinationTTL;
		portNumBits clientRTPPortNum, clientRTCPPortNum;
		unsigned char rtpChannelId, rtcpChannelId;
		parseTransportHeader(fullRequestStr, streamingMode, streamingModeString,
			clientsDestinationAddressStr, clientsDestinationTTL,
			clientRTPPortNum, clientRTCPPortNum,
			rtpChannelId, rtcpChannelId);

		fstreamingMode = streamingMode;
		unsigned short clientRTPPort(clientRTPPortNum);
		unsigned short clientRTCPPort(clientRTCPPortNum);

		//if (streamingMode == RTP_TCP && rtpChannelId == 0xFF ||
		//	streamingMode != RTP_TCP ) {
		//		// An anomolous situation, caused by a buggy client.  Either:
		//		//     1/ TCP streaming was requested, but with no "interleaving=" fields.  (QuickTime Player sometimes does this.), or
		//		//     2/ TCP streaming was not requested, but we're doing RTSP-over-HTTP tunneling (which implies TCP streaming).
		//		// In either case, we assume TCP streaming, and set the RTP and RTCP channel ids to proper values:
		//		streamingMode = RTP_TCP;
		//		rtpChannelId = fTCPStreamIdCount; rtcpChannelId = fTCPStreamIdCount+1;
		//}
		//fTCPStreamIdCount += 2;

		// Next, check whether a "Range:" header is present in the request.
		// This isn't legal, but some clients do this to combine "SETUP" and "PLAY":
		//double rangeStart = 0.0, rangeEnd = 0.0;
		//fStreamAfterSETUP = parseRangeHeader(fullRequestStr, rangeStart, rangeEnd) || parsePlayNowHeader(fullRequestStr);

		// Then, get server parameters from the 'subsession':
		u_int8_t destinationTTL = 255;
		delete[] clientsDestinationAddressStr;

		unsigned short serverRTPPort(0);
		unsigned short serverRTCPPort(0);
		// Make sure that we transmit on the same interface that's used by the client (in case we're a multi-homed server):
		// NOTE: The following might not work properly, so we ifdef it out for now:

		boost::system::error_code ec;
		boost::asio::ip::tcp::endpoint clientendpoint = rtsp_socket_.remote_endpoint(ec);
		boost::asio::ip::address ipaddress = clientendpoint.address();
		unsigned short clintrtsp_port = clientendpoint.port();

		std::string clientipaddr = ipaddress.to_string(ec);

		if(fstreamingMode == RTP_TCP)
			clientRTPPort = clientRTCPPort = clintrtsp_port;

		int iret = fOurServerMediaSessionPtr->getStreamParameters(
			fstreamingMode,
			fOurSessionId, clientipaddr.c_str(),
			clientRTPPort, clientRTCPPort,
			serverRTPPort, serverRTCPPort);

		if(iret < 0){
			if( strlen( fOurServerMediaSessionPtr->GetStartStreamResultInfo() ) > 0  )
				strcpy( m_strErrorInfo, fOurServerMediaSessionPtr->GetStartStreamResultInfo() );
			handleCmd_badMedia( cseq );
			break;
		}

		switch (streamingMode)
		{
		case RTP_UDP:
			{
				snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
					"RTSP/1.0 200 OK\r\n"
					"CSeq: %s\r\n"
					"%s"
					"Transport: RTP/AVP;unicast;destination=%s;source=%s;client_port=%d-%d;server_port=%d-%d\r\n"
					"Session: %08X\r\n\r\n",
					cseq,
					dateHeader(),
					clientipaddr.c_str(), fOurServer->getRTSPAddress().c_str(),
					//ntohs(clientRTPPort), ntohs(clientRTCPPort),
					//ntohs(serverRTPPort), ntohs(serverRTCPPort),
					(clientRTPPort), (clientRTCPPort),
					(serverRTPPort), (serverRTCPPort),
					fOurSessionId);
				break;
			}
		case RTP_TCP:
			{
				snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
					"RTSP/1.0 200 OK\r\n"
					"CSeq: %s\r\n"
					"%s"
					"Transport: RTP/AVP/TCP;unicast;destination=%s;source=%s;interleaved=0-1\r\n"
					"Session: %08X\r\n\r\n",
					cseq,
					dateHeader(),
					clientipaddr.c_str(), fOurServer->getRTSPAddress().c_str(),
					fOurSessionId);
				break;
			}
		case RAW_TCP:
			{
				snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
					"RTSP/1.0 200 OK\r\n"
					"CSeq: %s\r\n"
					"%s"
					"Transport: RAW/RAW/TCP;unicast;destination=%s;source=%s;interleaved=0\r\n"
					"Session: %08X\r\n\r\n",
					cseq,
					dateHeader(),
					clientipaddr.c_str(), fOurServer->getRTSPAddress().c_str(),
					fOurSessionId);
				break;
			}
		case RAW_UDP: 
			{
				handleCmd_unsupportedTransport(cseq);
				break;
			}
		}
		delete[] streamingModeString;
	} while (0);
}

void RTSPClientSession
	::handleCmd_withinSession(char const* cmdName,
	char const* urlPreSuffix, char const* urlSuffix,
	char const* cseq, char const* fullRequestStr) {
		if (fOurServerMediaSessionPtr == NULL) {
			handleCmd_notFound(cseq);
		}

		if (strcmp(cmdName, "TEARDOWN") == 0) {
			handleCmd_TEARDOWN(cseq);
		} else if (strcmp(cmdName, "PLAY") == 0) {
			handleCmd_PLAY(cseq, fullRequestStr);
		} else if (strcmp(cmdName, "PAUSE") == 0) {
			handleCmd_PAUSE(cseq);
		} else if (strcmp(cmdName, "GET_PARAMETER") == 0) {
			handleCmd_GET_PARAMETER(cseq, fullRequestStr);
		} else if (strcmp(cmdName, "SET_PARAMETER") == 0) {
			handleCmd_SET_PARAMETER(cseq, fullRequestStr);
		}
}

void RTSPClientSession
	::handleCmd_TEARDOWN(char const* cseq) {
		snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
			"RTSP/1.0 200 OK\r\nCSeq: %s\r\n%s\r\n\r\n",
			cseq, dateHeader());
		fSessionIsActive = False; // triggers deletion of ourself after responding
		if( fOurServerMediaSessionPtr != NULL ){
			fOurServerMediaSessionPtr->stopStream(fOurSessionId);
		}
}

static Boolean parseScaleHeader(char const* buf, float& scale) {
	// Initialize the result parameter to a default value:
	scale = 1.0;
	// First, find "Scale:"
	while (1) {
		if (*buf == '\0') return False; // not found
		if (_strncasecmp(buf, "Scale: ", 7) == 0) break;
		++buf;
	}

	// Then, run through each of the fields, looking for ones we handle:
	char const* fields = buf + 7;
	while (*fields == ' ') ++fields;
	float sc;
	if (sscanf(fields, "%f", &sc) == 1) {
		scale = sc;
	} else {
		return False; // The header is malformed
	}

	return True;
}

void RTSPClientSession
	::handleCmd_PLAY(char const* cseq,
	char const* fullRequestStr) {
		// Parse the client's "Scale:" header, if any:
		float scale;
		Boolean sawScaleHeader = parseScaleHeader(fullRequestStr, scale);

		char buf[100];
		char* scaleHeader;
		if (!sawScaleHeader) {
			buf[0] = '\0'; // Because we didn't see a Scale: header, don't send one back
		} else {
			sprintf(buf, "Scale: %f\r\n", scale);
		}
		scaleHeader = strDup(buf);

		// Parse the client's "Range:" header, if any:
		char clock_str[256]; memset(clock_str, 0, 256);
		double rangeStart = 0.0, rangeEnd = 0.0;
		Boolean sawRangeHeader = parseRangeHeader(fullRequestStr, rangeStart, rangeEnd, clock_str);

		// Create the "Range:" header that we'll send back in our response.
		// (Note that we do this after seeking, in case the seeking operation changed the range start time.)
		char* rangeHeader;
		if (!sawRangeHeader) {
			buf[0] = '\0'; // Because we didn't see a Range: header, don't send one back
		} else if (rangeEnd == 0.0 && scale >= 0.0) {
			sprintf(buf, "Range: npt=%.3f-\r\n", rangeStart);
		} else {
			sprintf(buf, "Range: npt=%.3f-%.3f\r\n", rangeStart, rangeEnd);
		}
		rangeHeader = strDup(buf);

		int nret  = 0;
		if( fOurServerMediaSessionPtr != NULL )
		{
			if ( scale < 0.01 ) m_bIsDownLoad = true;
			nret = fOurServerMediaSessionPtr->startStream( fOurSessionId, clock_str, scale );
			m_ptime_open = boost::posix_time::second_clock::local_time();
		}

		if( fOurServerMediaSessionPtr != NULL )
		{
			if( nret == -13){
				handleCmd_FullChannel( cseq, fOurServerMediaSessionPtr->GetFullCameraList() );
				delete[] rangeHeader;
				delete[] scaleHeader;
				return;
			}

			if( nret < 0 ){
				if( strlen( fOurServerMediaSessionPtr->GetStartStreamResultInfo() ) > 0  )
					strcpy( m_strErrorInfo, fOurServerMediaSessionPtr->GetStartStreamResultInfo() );
				handleCmd_badMedia( cseq );
				delete[] rangeHeader;
				delete[] scaleHeader;
				return;
			}
		}

		// Fill in the response:
		snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: %s\r\n"
			"%s"
			"%s"
			"%s"
			"Session: %08X\r\n\r\n",
			cseq,
			dateHeader(),
			scaleHeader,
			rangeHeader,
			fOurSessionId);

		delete[] rangeHeader;
		delete[] scaleHeader;
}

void RTSPClientSession
	::handleCmd_PAUSE( char const* cseq) {
		if( fOurServerMediaSessionPtr != NULL )
			fOurServerMediaSessionPtr->pauseStream();

		snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
			"RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sSession: %08X\r\n\r\n",
			cseq, dateHeader(), fOurSessionId);
}

void RTSPClientSession
	::handleCmd_GET_PARAMETER(char const* cseq,
	char const* /*fullRequestStr*/) {
		// By default, we implement "GET_PARAMETER" just as a 'keep alive', and send back an empty response.
		// (If you want to handle "GET_PARAMETER" properly, you can do so by defining a subclass of "RTSPServer"
		// and "RTSPServer::RTSPClientSession", and then reimplement this virtual function in your subclass.)
		snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
			"RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sSession: %08X\r\n\r\n",
			cseq, dateHeader(), fOurSessionId);
}

Boolean RTSPClientSession::checkForHeader(char const* line, char const* headerName,
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

void RTSPClientSession
	::handleCmd_SET_PARAMETER(char const* cseq,
	char const* fullRequestStr) 
{
	int cmd = -1, param1 = -1, param2 = -1;
	char* lineStart = (char*)fullRequestStr;
	char* nextLineStart = getLine(lineStart);

	// Scan through the headers, handling the ones that we're interested in:
	Boolean reachedEndOfHeaders;
	const char* headerParamsStr; 
	while (1) {
		reachedEndOfHeaders = True; // by default; may get changed below
		lineStart = nextLineStart;
		if (lineStart == NULL) break;

		nextLineStart = getLine(lineStart);
		if (lineStart[0] == '\0') break; // this is a blank line
		reachedEndOfHeaders = False;

		if (checkForHeader(lineStart, "VnmpControl:", 12, headerParamsStr)) 
		{
			char const* fields = headerParamsStr;
			char field[65536]; memset(field , 0, 65536);
			while (sscanf(fields, "%[^;]", field) == 1) {
				if (sscanf(field, "CMD=%d", &cmd) == 1) {

				} else if (sscanf(field, "Parameter1=%d", &param1) == 1) {

				} else if (sscanf(field, "Parameter2=%d", &param2) == 1) {

				}
				fields += strlen(field);
				while (fields[0] == ';') ++fields; // skip over all leading ';' chars
				if (fields[0] == '\0') break;
			}
		} 
	}

	if (!reachedEndOfHeaders || cmd == -1 || param1 == -1 || param2 == -1){
		snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
			"RTSP/1.0 562 VnmpControl Error\r\nCSeq: %s\r\n%sSession: %08X\r\n\r\n",
			cseq, dateHeader(), fOurSessionId);
		return;
	}

	int iRet = 0;
	if( fOurServerMediaSessionPtr != NULL && fOurServerMediaSessionPtr->IsLive() 
		&& cmd == CAMERA_COMMAND_MONITOR_CAMERA_SELECT && fOurServerMediaSessionPtr->GetCurCameraID() == param1 )
	{
		iRet = -3;
		iRet = fOurServerMediaSessionPtr->switchCamera( param1, param2, m_channleLevels );

		if( iRet == -3 )
			strcpy( m_strErrorInfo, "切换失败，权限不足或者被高权限占用！" );
		//if(iRet >= 0)SendANNOUNCEMessage();
		if( iRet >= 0 )fOurServer->SendMSGUpdateCameraName( m_mediaid );
	}

	if( fOurServerMediaSessionPtr != NULL && fOurServerMediaSessionPtr->IsLive() && cmd != CAMERA_COMMAND_MONITOR_CAMERA_SELECT )
	{
		fOurServerMediaSessionPtr->CameraControl(cmd, param1, param2);
	}

	std::string strCurPalyBackTime("");
	if( !fOurServerMediaSessionPtr->IsLive() && cmd == PLAY_BACK_GETTIME )
	{
		MRServerMediasession* pMedia = (MRServerMediasession*)(fOurServerMediaSessionPtr.get());
		strCurPalyBackTime = "CurPlayBackTime:";
		strCurPalyBackTime += pMedia->GetCurPlayBackTime();
		strCurPalyBackTime += "\r\n";
	}

	if( iRet >= 0 )
		snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 200 OK\r\nCSeq: %s\r\n%s%sSession: %08X\r\n\r\n",
		cseq, dateHeader(), strCurPalyBackTime.c_str(), fOurSessionId);
	else if(iRet == -2)
		snprintf( (char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 565 ReOpen Error\r\nCSeq: %s\r\n%sSession: %08X\r\n\r\n",
		cseq, dateHeader(), fOurSessionId );
	else if(iRet == -1)
		snprintf( (char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 564 Switch Error\r\nCSeq: %s\r\n%sSession: %08X\r\n\r\n",
		cseq, dateHeader(), fOurSessionId );
	else if(iRet == -3)
		snprintf( (char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 566 Switch Error\r\nCSeq: %s\r\n%sSession: %08X\r\n\r\n",
		cseq, dateHeader(), fOurSessionId );
	else
		snprintf( (char*)fResponseBuffer, sizeof fResponseBuffer,
		"RTSP/1.0 564 Switch Error\r\nCSeq: %s\r\n%sSession: %08X\r\n\r\n",
		cseq, dateHeader(), fOurSessionId );

	return;
}

void RTSPClientSession::handleHTTPCmd_notSupported() {
	snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"HTTP/1.0 405 Method Not Allowed\r\n%s\r\n\r\n", dateHeader());
}

void RTSPClientSession::handleHTTPCmd_notFound() {
	snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"HTTP/1.0 404 Not Found\r\n%s\r\n\r\n", dateHeader());
}

void RTSPClientSession::handleHTTPCmd_GET(char const* urlSuffix) {
	std::string strHtml = "NOT FOUND!";
	std::string strContentType = "text/html";

	if( strlen(urlSuffix) == 0 )
		fOurServer->PrintHttpInfo(strHtml);
	if( strcmp(urlSuffix, "ClientNUM") == 0 )
		fOurServer->PrintClientNUM(strHtml);
	if( strcmp(urlSuffix, "CapturePicture") == 0 )
	{
		char configValue[256]; memset(configValue, 0, 256);
		int i = fOurServer->GetDevInfo().GetDataInfoPtr()->GetConfig("CapturePicture", configValue);
		if( i >= 0 && strcmp(configValue,"on") == 0 ){
			i = fOurServer->GetDevInfo().GetDataInfoPtr()->GetConfig("CapturePort", configValue);
			int iCapturePort = atoi(configValue); 

			i = fOurServer->GetDevInfo().GetDataInfoPtr()->GetConfig("CapturePicTime", configValue);
			int iIntervalTime = atoi(configValue);

			if( i >= 0 && iCapturePort > 0 && iCapturePort < 65535 && iIntervalTime > 0 ){
				strHtml = "CapturePicture&";
				strHtml += boost::lexical_cast<std::string>(iCapturePort) + "&";
				strHtml += boost::lexical_cast<std::string>(iIntervalTime);
			}
			else
			{
				strHtml = "CapturePicture&ErrorInfo";
			}
		}
		else
		{
			strHtml = "CapturePicture&ErrorInfo";
		}
	}

	std::string strUrlSuffix = urlSuffix;
	std::string strCmd,strParam;
	std::size_t paramLen = strUrlSuffix.find_first_of('?');
	if( paramLen > 0 && paramLen < strUrlSuffix.length())
	{
		strCmd = strUrlSuffix.substr(0, paramLen);
		strParam = strUrlSuffix.substr(paramLen + 1, strUrlSuffix.length() - paramLen - 1);
		if( strCmd.compare("getDeviceInfo") == 0 )
			fOurServer->GetDeviceInfoByHttp(strParam, strHtml);
	}

	if( strHtml.substr( 0, 5 ).compare("<?xml") == 0 )
		strContentType = "text/xml";

	snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		"HTTP/1.0 200 OK \r\n"
		"Server: MS \r\n"
		"Mime-version: 1.0 \r\n"
		"Pragma: no-cache \r\n"
		"%s"
		"Content-Length: %d \r\n"
		"Content-Type:%s\r\n\r\n"
		"%s",
		dateHeader(), strHtml.size(), strContentType.c_str(), strHtml.c_str());
}

int RTSPClientSession::HttpRequest(std::string strIp, int port, std::string strPath)
{
  try
  {
	using boost::asio::ip::tcp;

    boost::asio::io_service io_service;

	boost::asio::ip::tcp::endpoint endpoint(
		boost::asio::ip::address_v4::from_string(strIp.c_str()), port);

    // Try each endpoint until we successfully establish a connection.
    tcp::socket socket_(io_service);
	socket_.connect(endpoint);

    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << strPath << " HTTP/1.0\r\n";
    request_stream << "Host: " << strIp << ":" << port << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    // Send the request.
    boost::asio::write(socket_, request);

    // Read the response status line. The response streambuf will automatically
    // grow to accommodate the entire line. The growth may be limited by passing
    // a maximum size to the streambuf constructor.
    boost::asio::streambuf response;
    boost::asio::read_until(socket_, response, "\r\n");

    // Check that response is OK.
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
      return -1;
    }
    if (status_code != 200)
    {
      return -1;
    }

    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(socket_, response, "\r\n\r\n");

    // Process the response headers.
    std::string header;
    while (std::getline(response_stream, header) && header != "\r")
      std::cout << header << "\n";
    std::cout << "\n";

    // Write whatever content we already have to output.
    if (response.size() > 0)
      std::cout << &response;

    // Read until EOF, writing data to output as we go.
    boost::system::error_code error;
    while (boost::asio::read(socket_, response, boost::asio::transfer_at_least(1), error))
		;

    //std::cout << &response;
	boost::asio::streambuf::const_buffers_type bufs_ = response.data();
	std::string responseStr(boost::asio::buffers_begin(bufs_), boost::asio::buffers_begin(bufs_) + response.size());
	//strResponse = responseStr;
	fOurServer->m_log.Add("HttpGet responseStr = %s", responseStr.c_str());

	if (error != boost::asio::error::eof)return -1;

	return 0;
    //if (error != boost::asio::error::eof)
    //  throw boost::system::system_error(error);
  } catch (std::exception& e) {
	  fOurServer->m_log.Add("HttpGet ip = %s port = %d error = %s", strIp.c_str(), port, e.what());
  }

  return 0;
}


}// end namespace