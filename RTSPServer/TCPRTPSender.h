#pragma once
#include "Sender.h"

namespace live555RTSP
{

class CTCPRTPSender
	: public CRTPSender
	, public boost::enable_shared_from_this<CTCPRTPSender>
{
public:
	CTCPRTPSender(live555RTSP::RTSPServer* ourserver);
	~CTCPRTPSender(void);

public:

	int InitRTPPackSender(
		unsigned int oursessionid, char* chip,
		unsigned short clientRTPPort,
		unsigned short clientRTCPPort,
		unsigned short& serverRTPPort, // out
		unsigned short& serverRTCPPort); // out

	void Stop();

	int RecvRTCPData(unsigned char *data, unsigned long len);

	RTSPServer::RTSPClientPtr GetRTSPClientPtr(){ return rtsp_client_ptr; };
private:
	bool SendRTP(unsigned char *data, size_t len);
	bool SendRTCP(unsigned char *data, size_t len);
	void StartRecvRTCP();

	void HandleSendRTCPData(const boost::system::error_code& error, int sendsize, unsigned char* data);
	void HandleSendRTPData(const boost::system::error_code& error, int sendsize, unsigned char *data);

private:
	unsigned short rtspport;
	RTSPServer::RTSPClientPtr rtsp_client_ptr;
};

}// end namespace