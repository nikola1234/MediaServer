#pragma once

#ifndef FILEWRITE_H
#define FILEWRITE_H
#include "FileIndexList.h"
#include <fstream>

namespace MediaSave
{

class CPreAllocateDisk;
class CFileWrite
{
public:
	CFileWrite(CPreAllocateDisk* pallocdisk);
	~CFileWrite(void);

	bool InitFileWrite( int cameraid );
	int WriteData(const char *data, int len);

	bool SwitchCamera( int inewCamera );
	bool CloseSaveFile();
private:
	bool GetFileStream();
	int WriteFileIndexInfo();
	bool WriteZeroData();
	bool Reset();

private:
	CPreAllocateDisk* m_pPreAllocDisk;
	CFileIndexList::CFileIndexPtr m_fileIndexPtr;

	std::fstream m_indexStream;
	std::fstream m_videoStream;

	int m_cameraid;
	int m_nodePos;      //Ë÷ÒýºÅ

	unsigned char m_headData[HEAD_SIZE];
	int m_headLen;
	bool m_isHaveHeadData;

	time_t m_curRecvDataTime;
};

}// end namespace

#endif // FILEWRITE_H