#include "FileWrite.h"
#include "PreAllocateDisk.h"
#include "ManageRecord.h"

#include <fstream>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace MediaSave
{

CFileWrite::CFileWrite(CPreAllocateDisk* pallocdisk)
	:m_cameraid(0),
	m_nodePos(1024),
	m_pPreAllocDisk(pallocdisk),
	m_isHaveHeadData(false),
	m_headLen(0),
	m_curRecvDataTime( time(NULL) - RECORD_TIMEOUT_MAX_SECONDS * 2 ) //在要写入数据时，获取文件
{

}

CFileWrite::~CFileWrite(void)
{
	Reset();
}

bool CFileWrite::InitFileWrite( int cameraid )
{
	m_cameraid = cameraid;
//	return GetFileStream();
	return true;
}

bool CFileWrite::Reset()
{
	try
	{
		if( m_fileIndexPtr != NULL )
		{
			m_nodePos = 1024;
			m_videoStream.close();
			m_indexStream.close();
			m_fileIndexPtr->SetCurState(CFileIndex::IDLE_FREE);
		}
		return true;
	}
	catch (const fs::filesystem_error& ex)
	{
		//std::cerr << ex.what() << '\n';
		m_pPreAllocDisk->m_log.Add("CFileWrite::Reset error = %s", ex.what());
		return false;
	}
}

int CFileWrite::WriteData(const char *data, int len)
{
	try
	{
		if(!m_isHaveHeadData)
		{
			unsigned int isign = 0;
			memcpy(&isign, data, sizeof(int));
			if (isign == 0xAAAB)
			{
				m_isHaveHeadData = true;
				memcpy(m_headData, data, len);
				m_headLen = len;
			}else
			{
				return 0;
			}
		}

		time_t now_t = time(NULL);
		int seconds = (int)difftime(now_t, m_curRecvDataTime);
		if( seconds > RECORD_TIMEOUT_MAX_SECONDS ){
			if( !GetFileStream() )return -1;
		}

		if(!m_videoStream.good() || m_videoStream.fail())return -1;
		if(!m_indexStream.good() || m_indexStream.fail())return -1;

		int filepos = (int) m_videoStream.tellp();
		if( ( filepos + len ) >= FILE_SIZE )
		{
			//m_pPreAllocDisk->m_log.Add("CFileWrite::GetFileStream begin cameraid = %d", this->m_cameraid);
			if( !GetFileStream() )return -1;
			//m_pPreAllocDisk->m_log.Add("CFileWrite::GetFileStream end cameraid = %d", this->m_cameraid);

			return WriteData( data, len );
		}

		m_curRecvDataTime = now_t;
		m_videoStream.write( data, len );
		return WriteFileIndexInfo();
		return 0;
	}
	catch (const fs::filesystem_error& ex)
	{
		//std::cerr << ex.what() << '\n';
		m_pPreAllocDisk->m_log.Add("CFileWrite::WriteData error = %s", ex.what());
		return -1;
	}
	return 0;
}

int CFileWrite::WriteFileIndexInfo()
{
	int filepos = (int) m_videoStream.tellp();
	int pos_ = filepos/DATASIZE_ONESECOND;
	if (pos_ != m_nodePos)
	{
		m_nodePos = pos_;
		if ( !m_fileIndexPtr->WriteIndexFileByfstream(filepos, m_indexStream) )return -1;
	}
	return 0;
}

bool CFileWrite::GetFileStream()
{
	m_curRecvDataTime = time(NULL);
	if( !Reset() ) return false;
	m_fileIndexPtr = m_pPreAllocDisk->GetFreeFileIndex();
	if(m_fileIndexPtr == NULL)return false;
	try
	{
		if(!WriteZeroData())return false;
		m_indexStream.open(m_fileIndexPtr->GetFileIndexPath().c_str(), std::ios_base::out|std::ios_base::in|std::ios_base::binary);
		m_videoStream.open(m_fileIndexPtr->GetFilePath().c_str(), std::ios_base::out|std::ios_base::in|std::ios_base::binary);
		if(m_isHaveHeadData)
			WriteData((const char *)m_headData, m_headLen );

		return m_fileIndexPtr->WriteFirstIndexTime(m_cameraid, m_indexStream);
		return true;
	}
	catch (const fs::filesystem_error& ex)
	{
		//std::cerr << ex.what() << '\n';
		m_pPreAllocDisk->m_log.Add("CFileWrite::GetFileStream error = %s", ex.what());
		return false;
	}
	return false;
}

bool CFileWrite::WriteZeroData()
{
	//if (m_fileIndexPtr->GetCameraID() != 0)
	//	return true;

	//try
	//{
	//	std::fstream ffile_stream_;	 
	//	ffile_stream_.open(m_fileIndexPtr->GetFilePath(),std::ios_base::in|std::ios_base::out|std::ios_base::binary);
	//	if(!ffile_stream_.good())
	//		return false;

	//	//char* buf_ = new char[1024*512];
	//	//ffile_stream_.rdbuf()->pubsetbuf(buf_,1024*512);

	//	char* buf = new char[1024*1024*2];
	//	memset(buf,0, 1024*1024);
	//	for (int i = 0; i < FILE_SIZE/(1024*1024); i ++)
	//		ffile_stream_.write((const char *)buf, 1024*1024);
	//	ffile_stream_.close();

	//	//delete[] buf_;

	//	delete[] buf;
	//	return true;
	//}catch(...)
	//{
	//	return false;
	//}

	return true;
}

bool CFileWrite::SwitchCamera( int inewCamera )
{
	m_cameraid = inewCamera;
	m_curRecvDataTime = time(NULL) - RECORD_TIMEOUT_MAX_SECONDS * 2;
	return Reset();
}

bool CFileWrite::CloseSaveFile()
{
	m_curRecvDataTime = time(NULL) - RECORD_TIMEOUT_MAX_SECONDS * 2;
	return Reset();
}

}// end namespace
