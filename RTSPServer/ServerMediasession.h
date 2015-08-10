#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/atomic.hpp>

#include "RTSPServer.h"
#include "RTSPClientSession.h"
#include "H264_RTP_PACK.H"

#include "MediaServerDLL.h"

namespace live555RTSP
{

typedef struct _STH264SPSPPSdata
{
	unsigned int	nSpsLen;
	unsigned char	Sps[1024];
	unsigned int	nPpsLen;
	unsigned char	Pps[1024];
} STH264SPSPPSdata;

class CSender;
class ServerMediasession
	: public boost::enable_shared_from_this<ServerMediasession>
{
public:
	enum MEDIASTATE
	{
		CREATE = 10001,
		OPENING,
		WORKING,
		STOP
	};

    #define HEAD_SIZE (1024*2)

	typedef boost::shared_ptr<CSender> SenderPtr;
	typedef boost::shared_lock<boost::shared_mutex> readLock;
	typedef boost::unique_lock<boost::shared_mutex> writeLock;

public:
	ServerMediasession(RTSPServer* ourserver,CDataInfo::DBCAMERAINFO &cameradata, int coderid = 0);
	virtual ~ServerMediasession(void);

	char* generateSDPDescription(int iCameraID = 0, const char* chUnitName = NULL, const char* chCameraName = "");
	int getStreamParameters(
		StreamingMode streammode,
		unsigned clientSessionId, // in
		const char* clientAddress, // in
		unsigned short& clientRTPPort, // in
		unsigned short& clientRTCPPort, // in
		unsigned short& serverRTPPort, // out
		unsigned short& serverRTCPPort // out
		);

	virtual int startStream( unsigned clientSessionId, std::string time_param = "", float scale = 1.0 );
	virtual int pauseStream(){ return 0; };
	int stopStream(unsigned clientSessionId);
	virtual int StopMediaServer();
	int switchCamera(int ioldCamera, int inewCamera, std::vector<int>& channleLevels);
	int CameraControl(int cmd, int param1, int param2);

	void RemoveRTPPackSender(SenderPtr rtpsenderptr);
	void StopSenderBySessionID(unsigned int sessionid);
	void RemoveRTPPackSenderBySessionID(unsigned int sessionid);
	SenderPtr GetRTPPackSenderBySessionID(unsigned int sessionid);

	int GetSenderCount();
	int GetSenderCountNoLock() { return m_senderList.size(); };

	unsigned int GetFirstSenderSessionid();
	void CloseClientSessionByUserName(char* UserName);

	int GetMediaID(){ return m_mediaid; };
	int GetCurCameraID(){ return m_cameradata.CameraID; };
	int GetCurCoderID(){ return m_coderid; };
	int GetDevMatrixID(){ return m_cameradata.DevID; };
	int GetisMatrix(){ return m_cameradata.isMatrix; };
	int GetDevType(){ return m_cameradata.DevType; };
	CDataInfo::DBCAMERAINFO* GetCameraDataPtr(){ return &m_cameradata; };

	std::string& GetUrlSuffix(){ return m_strURLSuffix; };

	static int GetFreeMediaID(){ return s_MRMediaIDBegin++; };
	bool GetIsNotCoder(){ return m_mediaid == m_cameradata.CameraID; };
	bool IsLive(){ return m_bliveMediaSession; };
	time_t GetCurRecvDataTime(){ return m_cur_recvdata_time; };
	bool IsChannelControl(){ return m_bChannelControl; };

	std::list<int>& GetFullCameraList(){return m_fullCameraList;};
	bool IsHaveHeadDate(){ return m_isHaveHeadData; };
	char* GetStartStreamResultInfo(){ return m_strResult; };
	void handleRecvVideoDataThread();

	boost::shared_ptr<STH264SPSPPSdata> GetH264SPSPPSData(){ return m_pH264SPSPPSdata; };

protected:
	int SendData(unsigned long  dwDataType, unsigned long bufsize, unsigned char *buffer, int timestampinc = -1);
	int HandleH264PackData(unsigned long  dwDataType, unsigned long bufsize, unsigned char *buffer, int timestampinc = -1);

	static bool __stdcall video_capture_callbackEx(unsigned long dwDataType, unsigned long bufsize, unsigned char *buffer, int user);
	int FullHeadData(unsigned long  dwDataType, unsigned long bufsize, unsigned char *buffer);

	int SendH264RTPPack(unsigned long  dwDataType, unsigned long bufsize, unsigned char *buffer, int timestampinc = -1);

	void GetAndSendRTPPackData(unsigned long  dwDataType, int timestampinc = -1);
	void HandleH264NaluData(unsigned long  dwDataType, unsigned char* pH264NalBuf, unsigned long iH264NalLen, int timestampinc);

protected:
	bool m_bliveMediaSession;
	boost::atomic<int> m_mediastate;

	int m_coderid;
	int m_mediaid;

	char m_strResult[256];
	std::string m_strURLSuffix;

private:
	CDataInfo::DBCAMERAINFO m_cameradata;
	static RTSPServer *m_pRTSPServer;

	/// RTPPackSender List
	std::list<SenderPtr> m_senderList;
	/// RTPPackSender Mutex
	boost::shared_mutex m_senderMutex;

	unsigned char m_headData[HEAD_SIZE];
	int m_headLen;
	bool m_isHaveHeadData;

	static boost::atomic<int> s_MRMediaIDBegin;

	std::list<int> m_fullCameraList;

	time_t m_cur_recvdata_time;
	boost::shared_ptr<CH264_RTP_PACK> m_pH264_RTP_PACK;
	boost::shared_ptr<char> m_pH264PackBuf;
	boost::shared_ptr<char> m_pFrameBufPtr;
	int m_iTSPSHeadBufCurSize;              //Head TS buf Size
	int m_iH264NalCurSize;                  //H264 Nal bud size
	int m_iFrameCurSize;                    //H264 TS buf size

//	int m_iDemuxIndex;

	bool m_bRun;
	boost::shared_ptr<boost::thread> m_pHandleRecvDataThread;
	bool m_bChannelControl;

	boost::shared_ptr<STH264SPSPPSdata> m_pH264SPSPPSdata;
};

}// end namespace