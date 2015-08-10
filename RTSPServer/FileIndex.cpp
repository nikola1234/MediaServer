#include "FileIndex.h"
#include "PreAllocateDisk.h"

namespace MediaSave
{

CFileIndex::CFileIndex(void)
	:m_fileID(0),
	m_cameraID(0),
	m_fileState(FILE_FREE),
	m_beginTime_t(0),
	m_endTime_t(0),
	m_fileEndOffset(0),
	m_curState(IDLE_FREE)
{
	memset(m_reserve,0,128);
}

CFileIndex::~CFileIndex(void)
{
}

bool CFileIndex::InitFileIndex( int packid, int fileindex , std::string storagepath, CPackage* ppack )
{
	m_fileID = packid*100000 + fileindex;
	this->m_storagePath = storagepath;
	m_pPackage = ppack;
	return true;
}

bool CFileIndex::CreateWriteIndexFile()
{
	try
	{
		int fileindexsize = GetFileIndexSize();
		boost::shared_ptr<char> chBuf = boost::shared_ptr<char>(new char[fileindexsize]);
		memset(chBuf.get(), 0 ,fileindexsize);

		fs::path path_(GetFileIndexPath());
		if(fs::exists(path_)){}
		else{
			std::fstream fstream_;
			fstream_.open(path_.c_str(), std::ios_base::out|std::ios_base::binary);
			if(!fstream_.good() || fstream_.fail() )return false;

			fstream_.seekp(0,std::ios_base::beg);
			for(int j = 0; j < PACKAGE_FILENUM_ONEINDEX; j++){
				fstream_.write((const char*)chBuf.get(), fileindexsize);
			}
			fstream_.close();
		}

		std::fstream fstream_;
		fstream_.open(path_.c_str(), std::ios_base::out|std::ios_base::in|std::ios_base::binary);
		if(!fstream_.good() || fstream_.fail())return false;

		fstream_.seekp(( (m_fileID%100000) % PACKAGE_FILENUM_ONEINDEX ) * fileindexsize,std::ios_base::beg);

		fstream_.write((const char*)&m_fileID, sizeof(int));
		fstream_.write((const char*)&m_cameraID, sizeof(int));
		fstream_.write((const char*)&m_fileState, sizeof(int));
		fstream_.write((const char*)&m_beginTime_t, sizeof(time_t));
		fstream_.write((const char*)&m_endTime_t, sizeof(time_t));
		fstream_.write((const char*)&m_fileEndOffset, sizeof(int));
		fstream_.write((const char*)m_reserve, 128);

		fstream_.close();
		
		return true;
	}
	catch (const fs::filesystem_error& ex)
	{
		m_pPackage->GetStorageDir()->GetPreAllocateDisk()->m_log.Add("CFileIndex::CreateWriteIndexFile error = %s", ex.what());
		//std::cerr << ex.what() << '\n';
		return false;
	}
	return false;
}

bool CFileIndex::WriteFirstIndexTime( int camid, std::fstream& fstream_ )
{
	try
	{
		m_curState = BUSYING;
		m_cameraID = camid; 
		m_beginTime_t = time(NULL);
		m_fileEndOffset = 0;
		m_endTime_t = time(NULL);
		m_fileState = FILE_FULL;

		if( !fstream_.good() || fstream_.fail() )return false;

		int fileindexsize = GetFileIndexSize();
		boost::shared_ptr<char> chBuf = boost::shared_ptr<char>(new char[fileindexsize]);
		memset(chBuf.get(), 0 ,fileindexsize);

		fstream_.seekp(( (m_fileID%100000) % PACKAGE_FILENUM_ONEINDEX ) * GetFileIndexSize(), std::ios_base::beg);
		fstream_.write(chBuf.get(), fileindexsize); // clear zero

		fstream_.seekg(( (m_fileID%100000) % PACKAGE_FILENUM_ONEINDEX ) * GetFileIndexSize(), std::ios_base::beg);
		fstream_.write((const char*)&m_fileID, sizeof(int));
		fstream_.write((const char*)&m_cameraID, sizeof(int));
		fstream_.write((const char*)&m_fileState, sizeof(int));
		fstream_.write((const char*)&m_beginTime_t, sizeof(time_t));
		fstream_.write((const char*)&m_endTime_t, sizeof(time_t));
		fstream_.write((const char*)&m_fileEndOffset, sizeof(int));

		return true;
	}
	catch (const fs::filesystem_error& ex)
	{
		//std::cerr << ex.what() << '\n';
		m_pPackage->GetStorageDir()->GetPreAllocateDisk()->m_log.Add("CFileIndex::WriteFirstIndexTime error = %s", ex.what());
		return false;
	}
	return false;
}

bool CFileIndex::WriteIndexFileByfstream(int filepos, std::fstream& fstream_)
{
	try
	{
		m_fileEndOffset = filepos;
		m_endTime_t = time(NULL);

		if(!fstream_.good() || fstream_.fail())return false;
		fstream_.seekp(((m_fileID%100000)%PACKAGE_FILENUM_ONEINDEX) * GetFileIndexSize() + sizeof(int) * 3 + sizeof(time_t),
			std::ios_base::beg);
		//fstream_.write((const char*)&fileid, sizeof(int));
		//fstream_.write((const char*)&cameraid, sizeof(int));
		//fstream_.write((const char*)&f_state, sizeof(int));
		//fstream_.write((const char*)&begintime, sizeof(time_t));

		fstream_.write((const char*)&m_endTime_t, sizeof(time_t));
		fstream_.write((const char*)&m_fileEndOffset, sizeof(int));

		int node_pos = filepos/DATASIZE_ONESECOND;
		int seconds = (int)difftime(m_endTime_t,m_beginTime_t);
		if( node_pos < 0 || node_pos >= DATANODE_NUM ) return false;

		int findex_pos = ((m_fileID%100000)%PACKAGE_FILENUM_ONEINDEX) * GetFileIndexSize() +
			FILEINDEXHEAD_SIZE + CTimeNode::GetDataNodeSize() * node_pos;
		fstream_.seekp(findex_pos, std::ios_base::beg);
		fstream_.write((const char*)&seconds, sizeof(int));
		fstream_.write((const char*)&filepos, sizeof(int));

		return true;
	}
	catch (const fs::filesystem_error& ex)
	{
		//std::cerr << ex.what() << '\n';
		m_pPackage->GetStorageDir()->GetPreAllocateDisk()->m_log.Add("CFileIndex::WriteIndexFileByfstream error = %s", ex.what());
		return false;
	}
	return false;
}

bool CFileIndex::ReadFillIndexFile()
{
	try
	{
		int fileindexsize = GetFileIndexSize();

		char file_namebuf[256];
		memset(file_namebuf, 0, 256);
		sprintf(file_namebuf,"package%03d\\index%03d.bin", m_fileID/100000, (m_fileID%100000)/PACKAGE_FILENUM_ONEINDEX);

		fs::path path_(m_storagePath + file_namebuf);

		int fileID = 0;
		std::fstream fstream_;
		fstream_.open(path_.c_str(), std::ios_base::in|std::ios_base::binary);
		if(!fstream_.good()||fstream_.fail())return false;

		fstream_.seekg(((m_fileID%100000)%PACKAGE_FILENUM_ONEINDEX)*fileindexsize, std::ios_base::beg);
		fstream_.read(( char*)&fileID, sizeof(int));
		if( !fstream_.good()||fstream_.fail() )return false;
		if(m_fileID != fileID)return false;

		fstream_.read(( char*)&m_cameraID, sizeof(int));
		fstream_.read(( char*)&m_fileState, sizeof(int));
		fstream_.read(( char*)&m_beginTime_t, sizeof(time_t));
		fstream_.read(( char*)&m_endTime_t, sizeof(time_t));
		fstream_.read(( char*)&m_fileEndOffset, sizeof(int));
		fstream_.read(( char*)m_reserve, 128);

		fstream_.close();
		return true;
	}
	catch (const fs::filesystem_error& ex)
	{
		//std::cerr << ex.what() << '\n';
		m_pPackage->GetStorageDir()->GetPreAllocateDisk()->m_log.Add("CFileIndex::ReadFillIndexFile error = %s", ex.what());
		return false;
	}
	return false;
}

std::string CFileIndex::GetFileIndexPath()
{
	char file_namebuf[256];
	memset(file_namebuf, 0, 256);
	sprintf(file_namebuf,"package%03d\\index%03d.bin", m_fileID/100000, (m_fileID%100000)/PACKAGE_FILENUM_ONEINDEX);

	std::string path_(m_storagePath + file_namebuf);
	return path_;
}

std::string CFileIndex::GetFilePath()
{
	char file_namebuf[256];
	memset(file_namebuf, 0, 256);
	sprintf(file_namebuf,"package%03d\\rec%05d.264", m_fileID/100000, m_fileID%100000);
	std::string path_(m_storagePath + file_namebuf);
	return path_;
}

bool CFileIndex::GetTimeNodeArray(CTimeNode *ptimenode)
{
	try
	{
		fs::path path_(GetFileIndexPath());

		std::fstream fstream_;
		fstream_.open(path_.c_str(),std::ios_base::in|std::ios_base::binary);
		if(!fstream_.good() || fstream_.fail())return false;
		fstream_.seekg(((m_fileID%100000)%PACKAGE_FILENUM_ONEINDEX) * GetFileIndexSize() + FILEINDEXHEAD_SIZE,std::ios_base::beg);

		unsigned char* buf = new unsigned char[ CTimeNode::GetDataNodeSize() * DATANODE_NUM ];
		fstream_.read((char *)buf, CTimeNode::GetDataNodeSize() * DATANODE_NUM);
		if(!fstream_.good() || fstream_.fail())return false;
		for ( int i = 0; i < DATANODE_NUM; i++ )
		{
			memcpy(&ptimenode[i].timeindex,buf + CTimeNode::GetDataNodeSize()*i , sizeof(int));
			memcpy(&ptimenode[i].offset,buf + CTimeNode::GetDataNodeSize()*i + sizeof(int), sizeof(int));
		}
		delete[] buf;

		fstream_.close();
		return true;
	}
	catch (const fs::filesystem_error& ex)
	{
		//std::cerr << ex.what() << '\n';
		m_pPackage->GetStorageDir()->GetPreAllocateDisk()->m_log.Add("CFileIndex::GetTimeNodeArray error = %s", ex.what());
		return false;
	}
	return false;
}

}// end namespace
