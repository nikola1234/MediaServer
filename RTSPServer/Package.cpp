#include "Package.h"
#include "StorageDir.h"
#include "FileIndex.h"
#include "FileIndexList.h"
#include "StorageDir.h"
#include "PreAllocateDisk.h"
#include "FileWrite.h"

#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
namespace fs = boost::filesystem;

namespace MediaSave
{

CPackage::CPackage(void)
	:m_packageBeginID(0),
	m_packageID(0),
	m_FileNum(0),
	m_PackageState(0)
{
}

CPackage::~CPackage(void)
{
}

bool CPackage::InitPackage(int packbeginid, int packageid,
	std::string strpath, CStorageDir *pstoragedir)
{
	try
	{
		m_packageBeginID = packbeginid;
		m_packageID = packageid;
		m_storagePath = strpath;
		this->m_pStorageDir = pstoragedir;

		fs::path des_path_(strpath + CStorageDir::s_allocInfoFileName);
		if (!fs::exists(des_path_))
			return false;

		if(ReadPackageInfo(strpath + CStorageDir::s_allocInfoFileName))
		{
			if(m_FileNum == 0)
			{
				m_packageBeginID = packbeginid;
				m_packageID = packageid;
				return CreatePackageDirFiles();
			}
			else if( m_FileNum > 0 )
			{
				return ReadFileIndexs();
			}
		}
	}
	catch (const fs::filesystem_error& ex)
	{
		//std::cerr << ex.what() << '\n';
		m_pStorageDir->GetPreAllocateDisk()->m_log.Add("CPackage::InitPackage error = %s", ex.what());
		return false;
	}

	return false;
}

bool CPackage::WritePackageInfo(std::string path)
{
	try
	{
		std::fstream fstream_;
		fstream_.open(path.c_str(),std::ios_base::out|std::ios_base::in|std::ios_base::binary);
		if(!fstream_.good() || fstream_.fail())return false;

		fstream_.seekp(PACKAGE_DIRHEAD + PACKAGE_INFOSIZE*(m_packageID - m_packageBeginID) ,std::ios_base::beg);

		fstream_.write((const char*)&m_packageID, sizeof(int));
		fstream_.write((const char*)&m_FileNum, sizeof(int));
		fstream_.write((const char*)&m_PackageState, sizeof(int));

		fstream_.close();
		return true;
	}
	catch(const fs::filesystem_error& ex)
	{ 
		//std::cerr << ex.what() << '\n';
		m_pStorageDir->GetPreAllocateDisk()->m_log.Add("CPackage::WritePackageInfo error = %s", ex.what());
		return false;
	}
	return false;
}

bool CPackage::ReadPackageInfo(std::string path)
{
	try
	{
		std::fstream fstream_;
		fstream_.open(path.c_str(), std::ios_base::in|std::ios_base::binary);
		if( !fstream_.good() || fstream_.fail() )return false;

		fstream_.seekg(PACKAGE_DIRHEAD + PACKAGE_INFOSIZE*(m_packageID - m_packageBeginID), std::ios_base::beg);

		fstream_.read(( char*)&m_packageID, sizeof(int));
		fstream_.read(( char*)&m_FileNum, sizeof(int));
		fstream_.read(( char*)&m_PackageState, sizeof(int));

		fstream_.close();
		return true;
	}
	catch(const fs::filesystem_error& ex)
	{
		//std::cerr << ex.what() << '\n';
		m_pStorageDir->GetPreAllocateDisk()->m_log.Add("CPackage::ReadPackageInfo error = %s", ex.what());
		return false; 
	}
	return false;
}

bool CPackage::CreatePackageDirFiles()
{
	try
	{
		char package_namebuf[256];
		memset(package_namebuf, 0, 256);
		sprintf(package_namebuf,"package%03d",m_packageID);

		fs::path path_(m_storagePath + package_namebuf);
		if(fs::exists(path_) && fs::is_directory(path_)){}
		else{
			if(!fs::is_directory(path_))
				fs::remove(path_);
			fs::create_directory(path_);
		}

		int ibeginfilenum = m_FileNum;
		int i = ibeginfilenum;
		for ( ; i < ibeginfilenum + PACKAGE_FILENUM_ONEINDEX; i++ )
		{
			CreateFiles(i, m_storagePath + package_namebuf + "\\");
		}

		return WritePackageInfo(m_storagePath + "allocfiles.des");
	}
	catch (const fs::filesystem_error& ex)
	{
		//std::cerr << ex.what() << '\n';
		m_pStorageDir->GetPreAllocateDisk()->m_log.Add("CPackage::CreatePackageDirFiles error = %s", ex.what());
		return false;
	}
	return false;
}

bool CPackage::CreateFiles(int fileindex, std::string strpath)
{
	try
	{
		char file_namebuf[256];
		memset(file_namebuf, 0, 256);
		sprintf(file_namebuf, "rec%05d.264", fileindex);

		fs::path path_(strpath + file_namebuf);
		if(fs::exists(path_))return false;

		std::fstream fstream_;
		fstream_.open((strpath + file_namebuf).c_str(), std::ios_base::out|std::ios_base::binary);
		if(!fstream_.good() || fstream_.fail())return false;

		fstream_.seekp(FILE_SIZE - 1, std::ios_base::beg);
		fstream_.write("\0", 1);
		fstream_.close( );

		m_FileNum++;

		CFileIndexList::CFileIndexPtr fileindex_ptr = 
			m_pStorageDir->GetPreAllocateDisk()->GetFileIndexList().CreateFileIndex();
		fileindex_ptr->InitFileIndex( m_packageID, fileindex, m_storagePath, this );
		return fileindex_ptr->CreateWriteIndexFile();
	}
	catch (const fs::filesystem_error& ex)
	{
		//std::cerr << ex.what() << '\n';
		m_pStorageDir->GetPreAllocateDisk()->m_log.Add("CPackage::CreateFiles error = %s", ex.what());
		return false;
	}
	return false;
}

bool CPackage::ReadFileIndexs()
{
	for (int i = 0; i < m_FileNum; i++)
	{
		CFileIndexList::CFileIndexPtr fileindex_ptr = 
			m_pStorageDir->GetPreAllocateDisk()->GetFileIndexList().CreateFileIndex();
		fileindex_ptr->InitFileIndex(m_packageID, i, m_storagePath, this);
		fileindex_ptr->SetFileIndexValid(fileindex_ptr->ReadFillIndexFile());
	}
	return true;
}

bool CPackage::IsPackageHaveFull()
{
	if( m_FileNum >= PACKAGE_FILEMAXNUM )
		return true;
	else
		return false;
}

void CPackage::PrintPackageInfo()
{
	boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());	
	std::string tmplogpath = "log\\SaveFilelist " + boost::gregorian::to_iso_extended_string((t.date())) + ".log";

	std::fstream file1;         
	file1.open(tmplogpath.c_str(),std::ios_base::out|std::ios_base::app); 

	file1 << "package_beginid = " << m_packageBeginID << " packageid = " << m_packageID
		<< " filenum = " << m_FileNum << " packagestate = " << m_PackageState 
		<< " storage_path = " << m_storagePath  << std::endl;

	file1.close();
}

}// end namespace
