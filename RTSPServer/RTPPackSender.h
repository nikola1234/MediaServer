#pragma once

#ifndef RTPPACKSENDER_H
#define RTPPACKSENDER_H

#include "Sender.h"
#include "UdpPortVector.h"

#include <queue>
#include "boost/thread.hpp"

namespace live555RTSP
{

class CUDPRTPSender 
	: public CRTPSender
	, public boost::enable_shared_from_this<CUDPRTPSender>
{

public:
	CUDPRTPSender(live555RTSP::RTSPServer* ourserver);
	~CUDPRTPSender(void);

	int InitRTPPackSender(
		unsigned int oursessionid, char* chip,
		unsigned short clientRTPPort,
		unsigned short clientRTCPPort,
		unsigned short& serverRTPPort, // out
		unsigned short& serverRTCPPort); // out

	void Stop();
	int RecvRTCPData(unsigned char *data, unsigned long len);
	RTSPServer::RTSPClientPtr GetRTSPClientPtr(){ RTSPServer::RTSPClientPtr t; return t; };

private:
	bool SendRTP(unsigned char *data, size_t len);
	bool SendRTCP(unsigned char *data, size_t len);
	void StartRecvRTCP();

	void HandleSendRTPData(const boost::system::error_code& error, int sendsize, unsigned char* data);
	void HandleSendRTCPData(const boost::system::error_code& error, int sendsize, unsigned char* data);
	void HandleRecvRTCPData(const boost::system::error_code& error, int recvsizes);

public:
	static UDPPortVector s_udpportvector;

private:
	boost::asio::ip::udp::socket rtp_socket_;
	boost::asio::ip::udp::socket rtcp_socket_;

	unsigned short rtcpserverport;
	unsigned short rtpserverport;

	unsigned short rtcpclientport;
	unsigned short rtpclientport;

	boost::mutex m_queque_mutex;
	std::queue<boost::asio::mutable_buffers_1> m_willSendBufFifo;
};

} // end namespace

#endif // RTPPACKSENDER_H