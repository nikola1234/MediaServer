#include "FileIndexList.h"
#include "boost/date_time/posix_time/posix_time.hpp"

namespace MediaSave
{

CFileIndexList::CFileIndexList(void)
	:m_bCacheIsHaveFreeFile(true)
{

}

CFileIndexList::~CFileIndexList(void)
{

}

CFileIndexList::CFileIndexPtr CFileIndexList::CreateFileIndex()
{
	CFileIndexPtr fileindex_ptr(new CFileIndex());

	writeLock writelock_(m_fileindexlist_mutex);
	m_fileIndexList.push_back(fileindex_ptr);

	return fileindex_ptr;
}

void CFileIndexList::RemoveFileIndex(CFileIndexPtr fileindex_ptr)
{
	writeLock writelock_(m_fileindexlist_mutex);
	m_fileIndexList.remove(fileindex_ptr);
}

CFileIndexList::CFileIndexPtr CFileIndexList::GetFreeFileIndex()
{
	CFileIndexPtr fileindex_ptr;
	readLock readlock_(m_fileindexlist_mutex);
	std::list<CFileIndexPtr>::iterator it = m_fileIndexList.begin();
	for (; it != m_fileIndexList.end();it++)
	{
		if ((*it)->GetCurState() != CFileIndex::IDLE_FREE)
			continue;

		if(fileindex_ptr == NULL)
		{
			fileindex_ptr = (*it);
		}
		else if( difftime(fileindex_ptr->GetBeginTime(),(*it)->GetBeginTime()) > 0.0 )
		{
			fileindex_ptr = (*it);
		}
	}
	return fileindex_ptr;
}

bool CFileIndexList::IsHaveFreeFiles()
{
	//缓存结果，快速判断是否需要申请硬盘空间
	if( !m_bCacheIsHaveFreeFile )return m_bCacheIsHaveFreeFile;

	int countfree = 0;
	readLock readlock_(m_fileindexlist_mutex);
	std::list<CFileIndexPtr>::iterator it = m_fileIndexList.begin();
	for (; it != m_fileIndexList.end();it++)
	{
		if ( (*it)->GetFileIndexState() == CFileIndex::FILE_FREE )
			countfree++;
		if( countfree >= 8 )
			return true;
	}
	m_bCacheIsHaveFreeFile = false;
	return false;
}

int CFileIndexList::FullFileIndexList(int cameraid, std::list<CFileIndexPtr>& fileIndexList)
{
	int count = 0;
	fileIndexList.clear();
	readLock readlock_(m_fileindexlist_mutex);
	std::list<CFileIndexPtr>::iterator it = m_fileIndexList.begin();
	for (; it != m_fileIndexList.end();it++)
	{
		if ((*it)->GetCameraID() == cameraid)
		{
			count++;
			fileIndexList.push_back(*it);
		}
	}
	if(count >= 2)
		fileIndexList.sort(CompareFileIndex);
	return count;
}

void CFileIndexList::PrintFileIndecList()
{
	time_t t1 = time(NULL);
	struct tm * ptm = gmtime(&t1);
	time_t t2  = mktime(ptm);
	int zonediff = (int)difftime(t1 , t2);

	readLock readlock_(m_fileindexlist_mutex);

	boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());	
	std::string tmplogpath = "log\\SaveFilelist " + boost::gregorian::to_iso_extended_string((t.date())) + ".log";

	std::fstream file1;         
	file1.open(tmplogpath.c_str(), std::ios_base::out|std::ios_base::app); 

	file1 << std::endl << "FileIndecList NUM = " << m_fileIndexList.size() << std::endl;
	std::list<CFileIndexPtr>::iterator it = m_fileIndexList.begin();

	int count = 1;
	for (; it != m_fileIndexList.end();it++)
	{
		if ((*it)->GetCameraID() != 0)
		{
			file1 << count++ << "  : cameraid = " << (*it)->GetCameraID()
				<< " fileid = " << (*it)->GetFileID()
				<< "  Begin time = " << boost::posix_time::to_iso_extended_string( boost::posix_time::from_time_t((*it)->GetBeginTime() + zonediff) ) 
				<< "  End time = " <<  boost::posix_time::to_iso_extended_string( boost::posix_time::from_time_t((*it)->GetEndTime() + zonediff)) 
				<< "  End offset = " << (*it)->GetEndOffset() << "  filestate =  " << (*it)->GetFileIndexState() << std::endl;
		}
	}
	file1 << std::endl;
	file1 << std::endl;
	file1.close();
}

bool CompareFileIndex(CFileIndexList::CFileIndexPtr& file_index_ptr1, CFileIndexList::CFileIndexPtr& file_index_ptr2)
{
	double timediff = difftime(file_index_ptr1->GetBeginTime(), file_index_ptr2->GetBeginTime());
	if(timediff < 0.0)
		return true;
	return false;
}

}// end namespace
