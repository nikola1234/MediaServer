#pragma once

#ifndef RAWSENDER_H
#define RAWSENDER_H

#include "Sender.h"
#include <queue>
#include "boost/thread.hpp"

namespace live555RTSP
{

class CRAWSender
	: public CSender
	, public boost::enable_shared_from_this<CRAWSender>
{
public:
	CRAWSender(live555RTSP::RTSPServer* ourserver);
	~CRAWSender(void);
public:
	int InitRTPPackSender(
		unsigned int oursessionid, char* chip,
		unsigned short clientRTPPort,
		unsigned short clientRTCPPort,
		unsigned short& serverRTPPort, // out
		unsigned short& serverRTCPPort); // out

	int Start();
	int SendData(unsigned long dwDataType, unsigned char *data, unsigned long len, int timestampinc);
	int RecvRTCPData(unsigned char *data, unsigned long len){ return false; };
	void Stop();

	unsigned int GetSessionID(){ return foursessionid; };
	int GetRTPSenderState(){ return rtpsenderstate; };
	void SetSenderState(int sender_state){ rtpsenderstate = sender_state; };
	RTSPServer::RTSPClientPtr GetRTSPClientPtr(){ return rtsp_client_ptr; };

protected:
	bool SendRTP(unsigned char *data, size_t len){ return false; };
	bool SendRTCP(unsigned char *data, size_t len){ return false; };
	void StartRecvRTCP(){ return ; };

	void HandleSendData(const boost::system::error_code& error, int sendsize, unsigned char *data);

	int RealSendData(unsigned long dwDataType, unsigned char *data, unsigned long len, int timestampinc);
protected:
	live555RTSP::RTSPServer* fourserver;

	int rtpsenderstate;
	unsigned int foursessionid;

	bool stop_;

	RTSPServer::RTSPClientPtr rtsp_client_ptr;

	boost::shared_ptr<char> m_pH264PackBuf;
	int m_iH264BufCurSize;

#ifndef WIN32
	boost::mutex m_queque_mutex;
	std::queue<boost::asio::mutable_buffers_1> m_willSendBufFifo;
#endif

};

} //end namespace

#endif // RAWSENDER_H