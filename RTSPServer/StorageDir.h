#pragma once

#ifndef STORAGEDIR_H
#define STORAGEDIR_H

#include "Package.h"
#include <string>

namespace MediaSave
{
class CPreAllocateDisk;
class CStorageDir
{

public:
	CStorageDir(void);
	~CStorageDir(void);

public:
	bool InitStorageDir(int dirid , int packbeginid ,
		std::string strpath, CPreAllocateDisk *pdisk);
	bool InitPackageArray();
	bool CreatePackageDir();
	bool CreatePackageFiles();
	CPreAllocateDisk* GetPreAllocateDisk(){ return m_pPreAllocateDisk; };

	bool IsStorageHaveFull();
	void PrintPackageArray();

private:
	bool CreateStorageDirInfo(std::string path);
	bool WriteStorageDirInfo(std::string path);
	bool ReadStorageDirInfo(std::string path);

public:
	static const char* s_allocInfoFileName;

private:
	int m_storageDirID;
	int m_packageNum;
	int m_packageBeginID;
	int m_packageEndID;
	int m_isStorageDirFull;
	int m_isHave;
	char m_storagePath[256];

	CPackage m_packageArray[PACKAGE_MAXNUM];
	CPreAllocateDisk *m_pPreAllocateDisk;
};

}// end namespace

#endif // STORAGEDIR_H