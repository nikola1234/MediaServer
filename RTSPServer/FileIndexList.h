#pragma once

#ifndef FILEINDEXLIST_H
#define FILEINDEXLIST_H

#include "FileIndex.h"
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace MediaSave
{

class CFileIndexList
{
public:
	typedef boost::shared_ptr<CFileIndex> CFileIndexPtr;

	typedef boost::shared_lock<boost::shared_mutex> readLock;
	typedef boost::unique_lock<boost::shared_mutex> writeLock;
public:
	CFileIndexList(void);
	~CFileIndexList(void);

	CFileIndexPtr CreateFileIndex();
	void RemoveFileIndex(CFileIndexPtr fileindex_ptr);

	CFileIndexPtr GetFreeFileIndex();
	bool IsHaveFreeFiles();

	int FullFileIndexList(int cameraid, std::list<CFileIndexPtr>& file_index_list);
	void PrintFileIndecList();

	void SetCacheHaveFreeFile(){ m_bCacheIsHaveFreeFile = true; };

private:
	/// file_index_list mutex
	std::list<CFileIndexPtr> m_fileIndexList;
	/// rtsp_client_list mutex
	boost::shared_mutex m_fileindexlist_mutex;

	bool m_bCacheIsHaveFreeFile;
};

bool CompareFileIndex(CFileIndexList::CFileIndexPtr& file_index_ptr1, CFileIndexList::CFileIndexPtr& file_index_ptr2);


}// end namespace

#endif // FILEINDEXLIST_H