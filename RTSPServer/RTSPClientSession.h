#pragma once

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/system/error_code.hpp>

#include "RTSPServer.h"
#include "RTSPCommon.h"
#include "DigestAuthentication.hh"

namespace live555RTSP
{

typedef enum StreamingMode {
		RTP_UDP = 0,
		RTP_TCP,
		RAW_TCP,
		RAW_UDP,
		TCP_RTMP
} StreamingMode;

typedef enum TOKEN_TYPE {
	TOKEN_NULL = 0,
	TOKEN_AUTH,
	TOKEN_RECORD,
	TOKEN_CHECKVIDEO,
	TOKEN_RECORDBYUSER
} TOKEN_TYPE;

class ServerMediasession;
class RTSPClientSession
	: public boost::enable_shared_from_this<RTSPClientSession>
{
public:
	RTSPClientSession(RTSPServer* ourServer, unsigned sessionId);
	~RTSPClientSession(void);

public:
	int Start();
	int Stop();

	boost::asio::ip::tcp::socket& GetrtspSocket(){ return rtsp_socket_; }
	static unsigned int GetStaticFakeClientNUM(){ return s_fakeClientID; }
	unsigned int GetOurSessionID(){ return fOurSessionId; }

	int SendANNOUNCEMessage(int iCameraID = 0, const char* chUnitName = NULL, const char* chCameraName = "");

	int GetMediaID(){ return m_mediaid; };
	char* GetUserName(){ return m_szUserName; };
	int GetTokenType(){ return m_token; };

	int GetUserLevel(){ return m_UserLevel; };
	int GetCameraID(){ return m_CameraID; };
	boost::posix_time::ptime &GetOpenTime(){ return m_ptime_open; };

	RTSPServer::ServerMediaPtr GetServerMediaPtr(){ return fOurServerMediaSessionPtr; };

	bool GetIsDownLoad(){ return m_bIsDownLoad; };
private:
	int StopFromInner();
	void handleRequestBytes(int newBytesRead);
	void resetRequestBuffer();

	void HandleRecvRTSPRequest(const boost::system::error_code& error, int recvsizes);
	void HandleSendRTSPResponse(const boost::system::error_code& error, int recvsizes);

	void incomingRequestHandler();

protected:
// Make the handler functions for each command virtual, to allow subclasses to redefine them:
	 virtual void handleCmd_badMedia(char const* cseq);
	 virtual void handleCmd_FullChannel(char const* cseq, std::list<int>& cameralist);

	 virtual void handleCmd_bad(char const* cseq);
	 virtual void handleCmd_notSupported(char const* cseq);
	 virtual void handleCmd_notFound(char const* cseq);
	 virtual void handleCmd_unsupportedTransport(char const* cseq);
	 virtual void handleCmd_OPTIONS(char const* cseq);
	 virtual void handleCmd_DESCRIBE(char const* cseq,
		 char const* urlPreSuffix, char const* urlSuffix,
		 char const* fullRequestStr);
	 virtual void handleCmd_SETUP(char const* cseq,
		 char const* urlPreSuffix, char const* urlSuffix,
		 char const* fullRequestStr);
	 virtual void handleCmd_withinSession(char const* cmdName,
		 char const* urlPreSuffix, char const* urlSuffix,
		 char const* cseq, char const* fullRequestStr);

	 virtual void handleCmd_TEARDOWN( char const* cseq );
	 virtual void handleCmd_PLAY( char const* cseq, char const* fullRequestStr );
	 virtual void handleCmd_PAUSE( char const* cseq);
	 virtual void handleCmd_GET_PARAMETER( char const* cseq, char const* fullRequestStr );
	 virtual void handleCmd_SET_PARAMETER( char const* cseq, char const* fullRequestStr );

	 void handleHTTPCmd_GET(char const* urlSuffix);
	 void handleHTTPCmd_notSupported();
	 void handleHTTPCmd_notFound();

	 Boolean checkForHeader(char const* line, char const* headerName,
		 unsigned headerNameLength, char const*& headerParams) ;

	 Boolean authenticationOK(char const* cmdName, char const* cseq,
		 char const* urlSuffix,
		 char const* fullRequestStr);

	 int HttpRequest(std::string strIp, int port, std::string strPath);

private:
	RTSPServer* fOurServer;
	RTSPServer::ServerMediaPtr fOurServerMediaSessionPtr;
	unsigned fOurSessionId;

	unsigned char fRequestBuffer[RTSP_BUFFER_SIZE];
	unsigned fRequestBytesAlreadySeen, fRequestBufferBytesLeft;
	unsigned char* fLastCRLF;
	unsigned char fResponseBuffer[RTSP_BUFFER_SIZE];
	Boolean fIsMulticast, fSessionIsActive, fStreamAfterSETUP;

	unsigned char fTCPStreamIdCount; // used for (optional) RTP/TCP
	unsigned fNumStreamStates;
	unsigned fRecursionCount;
	
	Authenticator fCurrentAuthenticator; // used if access control is needed

	boost::asio::ip::tcp::socket rtsp_socket_;
	bool stop_;
	static unsigned int s_fakeClientID;

	StreamingMode fstreamingMode;
	int m_icseq;

	char m_version[256];
	int m_token;
	int m_clientid;

	int m_mediaid;

	char m_szUserID[64];
	char m_szUserName[64];
	int m_UserLevel;
	int m_CameraID;

	char m_szBase64UserWithLevel[256];

	char m_strErrorInfo[256];
	boost::posix_time::ptime m_ptime_open;
	std::vector<int> m_channleLevels;

	bool m_bIsRTSP;
	bool m_bIsDownLoad;
};

}// end namespace