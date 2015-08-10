#pragma once

#ifndef FILEINDEX_H
#define FILEINDEX_H

#include "time.h"
#include "string.h"
#include <fstream>

#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem.hpp>
#include "StorageDir.h"

namespace fs = boost::filesystem;

namespace MediaSave
{

class CPackage;
class CTimeNode
{
public:
    int	timeindex;
	int offset;
	static int GetDataNodeSize(){ return sizeof(int)*2;};
};

class CFileIndex
	: public boost::enable_shared_from_this<CFileIndex>
{
public:
#define FILE_SIZE (1024*1024*256)    //max = 1024*1024*256
#define DATANODE_NUM 4096
#define DATASIZE_ONESECOND 65536

#define HEAD_SIZE (1024*2)

#define FILEINDEXHEAD_SIZE 256

	enum FILESTATE 
	{
		FILE_FREE = 0,
		FILE_FULL,
		FILE_ALARM,
		FILE_MANUAL
	};
	enum CURSTATE 
	{
		IDLE_FREE = 0,
		BUSYING,
		INVALID
	};

public:
	CFileIndex(void);
	~CFileIndex(void);

public:
	int GetFileID(){ return m_fileID; };
	static int GetFileIndexSize(){ return FILEINDEXHEAD_SIZE + DATANODE_NUM * CTimeNode::GetDataNodeSize(); };

	bool InitFileIndex( int packid, int fileindex, std::string storagepath, CPackage* ppack );
	bool CreateWriteIndexFile();
	bool ReadFillIndexFile();

	void SetFileIndexValid( bool Valid = true ){ if( !Valid ) m_curState = INVALID; };

	CPackage* GetPackage(){ return m_pPackage; };

	void SetFileIndexState(int fstate){ m_fileState = fstate; };
	int GetFileIndexState(){ return m_fileState; };

	void SetCurState(int state){ m_curState = state; };
	int GetCurState(){ return m_curState; };

	int GetEndOffset(){ return m_fileEndOffset; };
	int GetCameraID(){ return m_cameraID; };

	time_t GetBeginTime(){ return m_beginTime_t; };
	time_t GetEndTime(){ return m_endTime_t; };

	std::string GetFileIndexPath();
	std::string GetFilePath();
	
	bool GetTimeNodeArray(CTimeNode *ptimenode);

	//void SetCameraID(int camid) { cameraid = camid; };
	//void SetBeginTime(){ begintime = time(NULL); };

	bool WriteFirstIndexTime(int camid, std::fstream& fstream_ );
	bool WriteIndexFileByfstream( int filepos, std::fstream& fstream_ );

private:
	int m_fileID;
	int m_cameraID;
	int m_fileState;
	time_t m_beginTime_t;
	time_t m_endTime_t;
	int m_fileEndOffset;
	char m_reserve[128];

	std::string m_storagePath;

	CPackage* m_pPackage;
	int m_curState;
};

}// end namespace

#endif // FILEINDEX_H