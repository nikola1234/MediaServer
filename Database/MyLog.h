/************************************************************************ 
* 文件名：    log.h 
* 文件描述：  用于记录日志的类CLog  
* 版本号：    2.0 
* 日志标识：  0085
************************************************************************/ 

#ifndef _MYLOG_H
#define _MYLOG_H

#include "boost/thread.hpp"

class CMyLog
{
public:
	CMyLog();
	~CMyLog();

public:
	void InitLog(const char* lpszLogPath = "./");		
	void Add(const char* fmt, ...);		//输出文字，参数就跟printf一样
	
protected:

	enum {BUFSIZE = 4096};  //工作缓冲区
	char  m_tBuf[BUFSIZE];
	char m_strLogPath[256];
	boost::mutex mutex_;
};

#endif