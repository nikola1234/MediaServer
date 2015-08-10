#pragma once

#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include "string.h"

namespace MediaSave
{

class CStorageDir;
class CPackage
{
public:
#define PACKAGE_MAXNUM            512          //max = 512
#define PACKAGE_FILEMAXNUM        1024         //max = 65536

#define PACKAGE_DIRHEAD           512       
#define PACKAGE_INFOSIZE          64
#define PACKAGE_FILENUM_ONEINDEX  32           //32^   min = 32 (8Gb)

public:
	CPackage(void);
	~CPackage(void);

public:
	bool InitPackage(int packbeginid, int packageid, 
		std::string strpath, CStorageDir *pstoragedir);
	bool CreatePackageDirFiles();
	bool IsPackageHaveFull();

	void PrintPackageInfo();
	CStorageDir * GetStorageDir(){ return m_pStorageDir; };
private:
	bool WritePackageInfo(std::string path);
	bool ReadPackageInfo(std::string path);
	
	bool CreateFiles(int fileindex, std::string strpath);
	bool ReadFileIndexs();

private:
	int m_packageBeginID;
	int m_packageID;
	int m_FileNum;
	int m_PackageState;

	std::string m_storagePath;
	CStorageDir *m_pStorageDir;
};

}// end namespace

#endif // PACKAGE_H