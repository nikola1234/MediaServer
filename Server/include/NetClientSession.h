#ifndef _NET_CLIENT_SESSION_H_
#define _NET_CLIENT_SESSION_H_

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/system/error_code.hpp>
#include "NetServer.h"
#include "CmdDefine.h"
#include "CameraManage.h"

#define  BUFFER_SIZE  (10*1024)
#define  BUFFER_SIZE_MIN  (512)


class NetServer;
class NetClientSession
: public boost::enable_shared_from_this<NetClientSession>
{
public:

	NetClientSession(NetServer * ourServer, unsigned int sessionId);
	~NetClientSession();

public:
    boost::asio::ip::tcp::socket& GetClientSocket(){ return client_socket_; }
    int Start();
    unsigned int GetOurSessionID(){ return fOurSessionId; }
	uint8 GetOurClientType(){ return m_DeivceType; }
    int SendMessage(char *buff,int size);

	void mcu_operate_alarm(uint32 ID,uint8 flag);
	void mcu_operate_alarm_ack(char* buffer ,int size);

private:
	void incomingAcceptHandler();

	void mcu_register(char* buffer ,int size);
	void mcu_register_ack();


	int client_register_ack();
	
	void push_camera_data_ack1(ST_VDCS_VIDEO_PUSH_CAM & AddCamera);
	void push_camera_data_ack2(ST_VDCS_VIDEO_PUSH_CAM & AddCamera,string &url);
	int push_camera_data(char* buffer ,int size);


	void push_camera_param_ack(int ret);
	int push_camera_param(char * buffer , int size);	

	int delete_camera_ack(T_VDCS_VIDEO_CAMERA_DELETE* pt_CamDel,int ret);
	int delete_camera(char * buffer , int size);	
	
	int device_status_ack(char * buffer,int size);
	int warn_info_ack(char * buffer,int size);
	
	int ReciveData_GetParam(char* buffer ,int size);
	void HandleRecvReponse(const boost::system::error_code& error, int recvsizes);

	
	void HandleSendResponse(const boost::system::error_code& error, int sendsizes);

public:
	
	uint8 m_DeivceType;

private:
	NetServer* fOurServer;
	unsigned int fOurSessionId;
	char fAcceptBuffer[BUFFER_SIZE];
	char fBuffer[BUFFER_SIZE_MIN];
	unsigned int PKGNUM;
	unsigned int totalRecive;

	boost::asio::ip::tcp::socket client_socket_;
	int m_clientid;
	bool fSessionIsActive;
	bool stop_;
};

#endif
