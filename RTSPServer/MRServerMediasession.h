#pragma once
#include "ServerMediasession.h"
#include "FileRead.h"
#include <boost/thread.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

namespace live555RTSP
{

class CFileTimeNode
{
	public:
		CFileTimeNode(): m_time_t(0), m_offset(0) {}
		time_t m_time_t;
		int m_offset;
		static int GetNodeSize(){ return sizeof(time_t) + sizeof(int); };
};

#define IFRAME_MAX_DATASIZE  (1024*256)
#define TIMENODE_NUM         (MANUAL_RECORD_TIME * 3 )
class MRServerMediasession :
	public ServerMediasession
{
public:
	MRServerMediasession(RTSPServer* ourserver, CDataInfo::DBCAMERAINFO &cameradata, int coderid, std::string urlSuffix = "");
	~MRServerMediasession(void);

	virtual int startStream( unsigned clientSessionId, std::string time_param, float scale = 1.0);
	virtual int StopMediaServer();
	virtual int pauseStream();
	void handleReadSendDataThread();
	void handleReadFileSendThread();

	char* GetSaveFileInfo(std::string time_param);
	std::string GetCurPlayBackTime();

private:
	MediaSave::CFileRead m_fileRead;
	boost::shared_ptr<boost::thread> m_thread_ptr;

	bool m_isThreadPause;
	bool m_isThreadStop;
	float m_scale;

	boost::mutex m_fileReadMutex;

	std::string m_urlSuffix;
	std::string m_storagePath;

	int m_iIFrameDataSize; //获取到图像到达显示 所需要的数据大小
	CFileTimeNode* m_pArrayTimeNode;

	boost::posix_time::ptime m_begin_ptime;
	int m_fileCurPos;
	int m_curSeconds;

	std::fstream m_fileStream;
};

} // end namespace