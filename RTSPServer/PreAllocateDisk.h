#pragma once

#ifndef PREALLOCATEDISK_H
#define PREALLOCATEDISK_H

#include "Package.h"
#include "StorageDir.h"
#include "FileIndexList.h"
#include "MyLog.h"

namespace MediaSave
{

class CPreAllocateDisk
{

public:
#define STORAGE_MAXCOUNT        8
public:
	CPreAllocateDisk(void);
	~CPreAllocateDisk(void);

public:
	CFileIndexList& GetFileIndexList(){ return m_fileIndexList; };
	CFileIndexList::CFileIndexPtr GetFreeFileIndex();
	bool InitStorageDirArray(const char* storagePath);

	void PrintFileIndecList(){ m_fileIndexList.PrintFileIndecList(); };
	void PrintPackageArray();

	void SetIsPTZAutoBack(bool isPTZAutoBack){ m_isPTZAutoBack = isPTZAutoBack; };
	bool GetIsPTZAutoBack(){ return m_isPTZAutoBack; };
	std::string& GetStoragePath(){ return m_storagePath; };

private:
	bool IsHaveEnoughDisk(CStorageDir& storagedir_);
	bool ApplyPreAllocateDisk();

public:
	CMyLog m_log;

private:
	CStorageDir storagedir_array[STORAGE_MAXCOUNT];
	CFileIndexList m_fileIndexList;

	bool m_bPreAllocDisking;
	std::string m_storagePath;

	bool m_isPTZAutoBack;

	bool m_bCacheIsHaveEnoughDisk;
};

}// end namespace

#endif // PREALLOCATEDISK_H