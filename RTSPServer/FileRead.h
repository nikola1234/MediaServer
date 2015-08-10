#pragma once

#ifndef FILEREAD_H
#define FILEREAD_H

#include "FileIndexList.h"
#include <fstream>
#include <list>
#include "PreAllocateDisk.h"
#include "FileWrite.h"

namespace MediaSave
{

class CFileRead
{
public:
	CFileRead(CPreAllocateDisk* pallocdisk);
	~CFileRead(void);

	bool InitFileRead( int cameraid );
	bool SetReadTime( std::string& str_begintime, std::string& str_endtime);
	int ReadData( char* data_buf, int &isecond );
	bool GetSaveFileInfo(std::string& fileTimeInfo);
private:
	bool FindFileIndexByTime( time_t time_ );
	int GetOffsetBySecond( int isecond );
	int GetSecondByOffset( int iOffset );
	bool FULLHeadData();
	bool GetNextFileIndex();

	bool SkipDataHead();

private:
	CPreAllocateDisk* m_pPreAllocDisk;
	int m_cameraid;

	std::fstream m_fileStream;
	CTimeNode* m_pTimeNode;

	CFileIndexList::CFileIndexPtr m_curFileIndexPtr; 
	/// file_index_list mutex
	std::list<CFileIndexList::CFileIndexPtr> m_fileIndexListForOne;

	unsigned char m_headData[HEAD_SIZE];
	int m_headLen;

	bool m_isHaveHeadData;
	bool m_isHavaSendHead;

	time_t m_curBegTime_t;
	time_t m_firstBegTime_t;
	time_t m_firstEndTime_t;
	int m_iCountSeconds;

	//boost::mutex m_fileReadMutex;
};

}// end namespace

#endif // FILEREAD_H