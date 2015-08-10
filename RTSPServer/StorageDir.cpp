#include "StorageDir.h"
#include "PreAllocateDisk.h"

#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
namespace fs = boost::filesystem;

namespace MediaSave
{

const char* CStorageDir::s_allocInfoFileName = "allocfiles.des";

CStorageDir::CStorageDir(void)
	:m_storageDirID(0),
	m_packageNum(0),
	m_packageBeginID(0),
	m_packageEndID(0),
	m_isStorageDirFull(0),
	m_isHave(0)
{
	memset(m_storagePath, 0, 256);
}

CStorageDir::~CStorageDir(void)
{
}

bool CStorageDir::InitStorageDir(int dirid, int packbeginid,
	std::string strpath, CPreAllocateDisk *pdisk)
{
	m_isHave = 1;
	m_storageDirID = dirid;
	m_packageBeginID = packbeginid;
	strcpy(m_storagePath, strpath.c_str());
	m_pPreAllocateDisk = pdisk;
	try
	{
		fs::path path_(m_storagePath);
		if(fs::exists(path_) && fs::is_directory(path_)){}
		else
		{
			if(!fs::is_directory(path_))
				fs::remove(path_);
			fs::create_directory(path_);
		}
		
		fs::path des_path_(strpath + s_allocInfoFileName);
		if (fs::exists(des_path_))
		{
			return ReadStorageDirInfo(strpath + s_allocInfoFileName);
		}
		else
		{
			bool bret = CreateStorageDirInfo(strpath + s_allocInfoFileName);
			if( bret )
				return WriteStorageDirInfo(strpath + s_allocInfoFileName);
			else
				return false;
		}
	}
	catch (const fs::filesystem_error& ex)
	{
		//std::cerr << ex.what() << '\n';
		m_pPreAllocateDisk->m_log.Add("CStorageDir::InitStorageDir error = %s", ex.what());
		return false;
	}

	return false;
}

bool CStorageDir::CreateStorageDirInfo(std::string path)
{
	try
	{
		boost::shared_ptr<char> chBuf = 
			boost::shared_ptr<char>(new char[PACKAGE_DIRHEAD + PACKAGE_INFOSIZE * PACKAGE_MAXNUM]);
		memset(chBuf.get(), 0 ,PACKAGE_DIRHEAD + PACKAGE_INFOSIZE * PACKAGE_MAXNUM);

		std::fstream fstream_;
		fstream_.open(path.c_str(), std::ios_base::out|std::ios_base::binary);
		if(!fstream_.good() || fstream_.fail())return false;

		fstream_.seekp(0, std::ios_base::beg);
		fstream_.write(chBuf.get(), PACKAGE_DIRHEAD + PACKAGE_INFOSIZE * PACKAGE_MAXNUM);
		fstream_.close();

		return true;
	}
	catch(...)
	{
		return false;
	}
	return false;
}

bool CStorageDir::WriteStorageDirInfo(std::string path)
{
	try
	{
		std::fstream fstream_;
		fstream_.open(path.c_str(), std::ios_base::out|std::ios_base::in|std::ios_base::binary);
		if(!fstream_.good() || fstream_.fail())return false;

		fstream_.seekp(0, std::ios_base::beg);
		fstream_.write((const char*)&m_storageDirID, sizeof(int));
		fstream_.write((const char*)&m_packageNum, sizeof(int));
		fstream_.write((const char*)&m_packageBeginID, sizeof(int));
		fstream_.write((const char*)&m_packageEndID, sizeof(int));
		fstream_.write((const char*)&m_isStorageDirFull, sizeof(int));
		fstream_.write((const char*)&m_isHave, sizeof(int));
		fstream_.write((const char*)m_storagePath, 256);
		fstream_.close();
		return true;
	}
	catch(...){ return false; }
	return false;
}

bool CStorageDir::ReadStorageDirInfo(std::string path)
{
	try
	{
		std::fstream fstream_;
		fstream_.open(path.c_str(),std::ios_base::in|std::ios_base::binary);
		if(!fstream_.good() || fstream_.fail())return false;

		fstream_.seekg(0, std::ios_base::beg);
		fstream_.read(( char*)&m_storageDirID, sizeof(int));
		fstream_.read(( char*)&m_packageNum, sizeof(int));
		fstream_.read(( char*)&m_packageBeginID, sizeof(int));
		fstream_.read(( char*)&m_packageEndID, sizeof(int));
		fstream_.read(( char*)&m_isStorageDirFull, sizeof(int));
		fstream_.read(( char*)&m_isHave, sizeof(int));
		fstream_.read(( char*)m_storagePath, 256);
		fstream_.close();
		return true;
	}
	catch(...){ return false; }
	return false;
}

 bool CStorageDir::InitPackageArray()
{
	for (int i = 0; i < m_packageNum; i++)
	{
		m_packageArray[i].InitPackage(m_packageBeginID, m_packageBeginID + i, m_storagePath,this);
	}
	if(m_packageNum == 0)
		return CreatePackageDir();
	return true;
}

bool CStorageDir::CreatePackageDir()
{
	if (m_packageNum >= PACKAGE_MAXNUM)
		return false;
	std::string strpath(m_storagePath);
	bool bret = m_packageArray[m_packageNum].InitPackage(m_packageBeginID , m_packageBeginID + m_packageNum, m_storagePath,this);
	if(bret)
	{
		m_packageNum += 1;
		m_packageEndID = m_packageBeginID + m_packageNum -1;
		return WriteStorageDirInfo(strpath + s_allocInfoFileName);
	}
	return bret;
}

bool  CStorageDir::CreatePackageFiles()
{
	for(int i = 0; i < m_packageNum; i++)
	{
		if (!m_packageArray[i].IsPackageHaveFull())
		{
			return m_packageArray[i].CreatePackageDirFiles();
		}
	}
	return CreatePackageDir();
	return false;
}

bool CStorageDir::IsStorageHaveFull()
{
	if( m_packageNum >= PACKAGE_MAXNUM )
		return true;
	else
		return false;
}

void CStorageDir::PrintPackageArray()
{
	boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());	
	std::string tmplogpath = "log\\SaveFilelist " + boost::gregorian::to_iso_extended_string((t.date())) + ".log";

	std::fstream file1;         
	file1.open(tmplogpath.c_str(),std::ios_base::out|std::ios_base::app); 

	file1 << "StoragePath = " <<  m_storagePath << "storage_dirid = " << m_storageDirID
		<< " package_num = " << m_packageNum << "package_beginid = " << m_packageBeginID 
		<< " package_endid = " << m_packageEndID << " is_storagedir_full " << m_isStorageDirFull
		<< "  is_have = " << m_isHave << std::endl;
	file1 << "---------------------------------------------------------------" << std::endl;
	for(int i = 0; i < m_packageNum; i++)
	{
		m_packageArray[i].PrintPackageInfo();
	}
	file1 << "---------------------------------------------------------------" << std::endl;

	file1.close();
}

}// end namespace
