#include "FileRead.h"
#include "boost/date_time/posix_time/posix_time.hpp"

namespace MediaSave
{

CFileRead::CFileRead(CPreAllocateDisk* pallocdisk)
	: m_pPreAllocDisk(pallocdisk),
	m_cameraid(0),
	m_pTimeNode(NULL),
	m_isHaveHeadData(false),
	m_headLen(0),
	m_isHavaSendHead(false),
	m_iCountSeconds(0),
	m_firstBegTime_t(0),
	m_firstEndTime_t(0)
{
	m_pTimeNode = new CTimeNode[DATANODE_NUM];
	memset(m_headData, 0, HEAD_SIZE);
}

CFileRead::~CFileRead(void)
{
	m_fileStream.close();
	if (m_pTimeNode)
		delete[] m_pTimeNode;
}

bool CFileRead::InitFileRead( int cameraid )
{
	m_cameraid = cameraid;
	if(m_pPreAllocDisk == NULL)return false;

	CFileIndexList& fileindexlist = m_pPreAllocDisk->GetFileIndexList();
	int count = fileindexlist.FullFileIndexList(m_cameraid, m_fileIndexListForOne);
	if(count > 0)return true;
	return false;
}

bool CFileRead::SetReadTime(std::string& str_begintime, std::string& str_endtime)
{
	try
	{
		//boost::posix_time::ptime time_ = boost::posix_time::time_from_string(str_time);
		boost::posix_time::ptime begin_time_ = boost::posix_time::from_iso_string(str_begintime);
		boost::posix_time::ptime end_time_ = boost::posix_time::from_iso_string(str_endtime);

		//time_t t1 = time(NULL);
		//struct tm * ptm = gmtime(&t1);
		//time_t t2  = mktime(ptm);
		//int zonediff = (int)difftime(t1 , t2);

		tm beg_tm = boost::posix_time::to_tm(begin_time_);
		tm end_tm = boost::posix_time::to_tm(end_time_);
		time_t beg_time_t = mktime(&beg_tm)/* + zonediff*/;
		time_t end_time_t = mktime(&end_tm)/* + zonediff*/;

		if(m_firstBegTime_t == 0)
			m_firstBegTime_t = beg_time_t;

		if(m_firstEndTime_t == 0){
			m_firstEndTime_t = end_time_t;
			m_iCountSeconds = (int)difftime(end_time_t, beg_time_t);
			if(m_iCountSeconds <= 0)return false;
		}

		return FindFileIndexByTime(beg_time_t);
	}
	catch(...)
	{
		return false;
	}
	return false;
}

bool CFileRead::GetSaveFileInfo(std::string& fileTimeInfo)
{
	try{
		
		std::list<CFileIndexList::CFileIndexPtr>::iterator it =  m_fileIndexListForOne.begin();
		std::list<CFileIndexList::CFileIndexPtr>::iterator it_prev = it;

		time_t curTime_t = 0;
		fileTimeInfo = "";
		while(it != m_fileIndexListForOne.end())
		{
			if(fileTimeInfo.length() > 0)
			{
				int iSec1 = (int)difftime(m_firstEndTime_t, (*it_prev)->GetBeginTime());
				int iSec2 = (int)difftime(m_firstEndTime_t, (*it_prev)->GetEndTime());
				int iSec3 = (int)difftime(m_firstEndTime_t, (*it)->GetBeginTime());
				int iSec4 = (int)difftime(m_firstEndTime_t, (*it)->GetEndTime());

				if( iSec3 >= 0 && iSec4 <= 0 ){
					break;
				}
				if(iSec3 <= 0){
					it = it_prev;
					break;
				}
				int iSec5 = (int)difftime((*it)->GetBeginTime(), (*it_prev)->GetEndTime());
				if( iSec5 > 10 ){ //<= 10s ºöÂÔ
					int num1 = (int)difftime((*it_prev)->GetEndTime(), m_firstBegTime_t);
					int num2 = (int)difftime((*it)->GetBeginTime(), m_firstBegTime_t);
					fileTimeInfo += boost::lexical_cast<std::string>(num1) + ";" +
						boost::lexical_cast<std::string>(num2) + "-";
				}
			}

			int iSeconds1 = (int)difftime(m_firstBegTime_t, (*it_prev)->GetBeginTime());
			int iSeconds2 = (int)difftime(m_firstBegTime_t, (*it_prev)->GetEndTime());
			int iSeconds3 = (int)difftime(m_firstBegTime_t, (*it)->GetBeginTime());
			int iSeconds4 = (int)difftime(m_firstBegTime_t, (*it)->GetEndTime());

			if( (it == it_prev) && difftime((*it)->GetBeginTime(), m_firstBegTime_t) >= 0.0 )
			{
				int isec = (int)difftime((*it)->GetBeginTime(), m_firstBegTime_t);
				fileTimeInfo = boost::lexical_cast<std::string>(isec) + "-";
			}
			if(iSeconds3 >= 0 && iSeconds4 <= 0)
			{
				fileTimeInfo = "0-";
			}
			if(iSeconds2 >= 0 && iSeconds3 <= 0)
			{
				fileTimeInfo = boost::lexical_cast<std::string>(0 - iSeconds3) + "-";
			}

			it_prev = it;
			it++;
		}

		if(fileTimeInfo.length() <= 1)
			return false;

		if(it != m_fileIndexListForOne.end())
		{
			int seconds_num = 0;
			int iSec1 = (int)difftime(m_firstEndTime_t, (*it)->GetEndTime());
			if(iSec1 <= 0)
			{
				if( difftime(m_firstEndTime_t, (*it)->GetBeginTime()) > 0.1 )
				{
					seconds_num = (int)difftime(m_firstEndTime_t, m_firstBegTime_t);
					fileTimeInfo += boost::lexical_cast<std::string>(seconds_num);
				}
				else
					fileTimeInfo = "";
			}
			else
			{
				seconds_num = (int)difftime((*it)->GetEndTime(), m_firstBegTime_t);
				fileTimeInfo += boost::lexical_cast<std::string>(seconds_num);
			}
		}
		else
		{
			if( it_prev != m_fileIndexListForOne.end() )
			{
				int seconds_num = (int)difftime((*it_prev)->GetEndTime(), m_firstBegTime_t);
				fileTimeInfo += boost::lexical_cast<std::string>(seconds_num);
			}
		}
		if(fileTimeInfo.length() <= 1)
			return false;
		return true;
	}
	catch(...){ return false; }
	return true;
}

bool CFileRead::FindFileIndexByTime( time_t time_ )
{
	try{
		std::list<CFileIndexList::CFileIndexPtr>::iterator it =  m_fileIndexListForOne.begin();
		for (; it != m_fileIndexListForOne.end(); it++)
		{
			if(/*difftime((*it)->GetBeginTime(),time_) <= 0.0 && */
				difftime( (*it)->GetEndTime(), time_ ) >= 0.0)
			{
				if( m_curFileIndexPtr != (*it) )
				{
					m_curFileIndexPtr = (*it);
					m_fileStream.close();
					m_fileStream.open(m_curFileIndexPtr->GetFilePath().c_str(), std::ios_base::in|std::ios_base::binary);
					if(!m_fileStream.good() || m_fileStream.fail())return false;

					if(!m_curFileIndexPtr->GetTimeNodeArray(m_pTimeNode))return false;
					if(!FULLHeadData())return false;
				}

				if(m_curFileIndexPtr == NULL)return false;
				if(!m_fileStream.good() || m_fileStream.fail())return false;

				int diffseconds = ( int )difftime( time_, m_curFileIndexPtr->GetBeginTime() );
				int ioffset =  GetOffsetBySecond(diffseconds);
				if( ioffset < 0 || ioffset >= FILE_SIZE)return false;
				m_fileStream.seekg( ioffset, std::ios_base::beg );

				m_curBegTime_t = time_;
				m_isHavaSendHead = false;
				return true;
			}
		}
		return false;
	}
	catch(...){ return false; }
}

int CFileRead::GetOffsetBySecond(int isecond)
{
	int ioffset = -1;
	for (int i = 1; i < DATANODE_NUM; i++)
	{
		if(( m_pTimeNode[i - 1].timeindex <= isecond ) && ( m_pTimeNode[i].timeindex >= isecond ))
		{
			ioffset = m_pTimeNode[i].offset;
			break;
		}
	}
	return ioffset;
}

int CFileRead::GetSecondByOffset( int iOffset )
{
	int iSecond = 0;
	for (int i = 1; i < DATANODE_NUM; i++)
	{
		if(( m_pTimeNode[i - 1].offset <= iOffset ) && ( m_pTimeNode[i].offset >= iOffset ))
		{
			iSecond = m_pTimeNode[i].timeindex;
			break;
		}
	}
	return iSecond;
}

bool CFileRead::FULLHeadData()
{
	try
	{
		if(m_isHaveHeadData)return true;
		if(!m_fileStream.good() || m_fileStream.fail())return false;

		m_fileStream.seekg(0, std::ios_base::beg);
		m_fileStream.read((char*)m_headData, 12);
		if(m_fileStream.gcount() < 12)return false;

		int isign = 0, ilen = 0;
		memcpy(&isign , m_headData, sizeof(int));
		memcpy(&ilen, m_headData + sizeof(int), sizeof(int));
		if( ilen > HEAD_SIZE )return false;

		m_fileStream.read(( char *)(m_headData + 12), ilen);
		if(m_fileStream.gcount() < ilen)return false;
		else
		{
			m_isHaveHeadData = true;
			m_headLen = ilen + 12;
			return true;
		}
	}
	catch(...){ return false; }
}

int CFileRead::ReadData(char* data_buf, int &isecond)
{
	try{
		if(!m_isHavaSendHead){
			memcpy(data_buf, m_headData, m_headLen);
			m_isHavaSendHead = true;
			return m_headLen;
		}

		if( !m_fileStream.good() ||m_fileStream.fail() )return 0;
		if( m_curFileIndexPtr == NULL )return 0;

		int cur_pos = (int)m_fileStream.tellg();
		if ( cur_pos + 12 < m_curFileIndexPtr->GetEndOffset() )
		{
			m_fileStream.read(data_buf, 12);
			if(!m_fileStream.good() || m_fileStream.fail())return 0;

			int isign,ilen;
			memcpy(&isign, data_buf, sizeof(int));
			memcpy(&ilen, data_buf + sizeof(int), sizeof(int));
			if(isign == 0xAAAA || isign == 0xAAAB)
			{
				cur_pos = (int)m_fileStream.tellg();
				if(cur_pos + ilen <= m_curFileIndexPtr->GetEndOffset())
				{
					isecond = GetSecondByOffset( cur_pos );
					isecond +=  (int)difftime(m_curFileIndexPtr->GetBeginTime(), m_firstBegTime_t);
					if(isecond >= m_iCountSeconds ){
						m_pPreAllocDisk->m_log.Add("CFileRead::ReadData Success End!");
						return 0;
					}

					m_fileStream.read(data_buf + 12, ilen);
					int readlen = (int)m_fileStream.gcount();
					if(readlen == ilen){
						return readlen + 12;
					}
				}
			}else
			{
				std::cout << "isign != 0xAAAA" << std::endl; 
				m_pPreAllocDisk->m_log.Add("CFileRead::ReadData isign != 0xAAAA");
				return 0;
			}
		}
		if(!GetNextFileIndex())return 0;
		return ReadData(data_buf, isecond);
		return 0;
	}catch(...){  m_pPreAllocDisk->m_log.Add("CFileRead::ReadData catch error!"); return 0;  }
}

bool CFileRead::GetNextFileIndex()
{
	try
	{
		if(m_curFileIndexPtr == NULL)return false;
		std::list<CFileIndexList::CFileIndexPtr>::iterator it =  m_fileIndexListForOne.begin();
		for (; it != m_fileIndexListForOne.end(); it++)
		{
			if( m_curFileIndexPtr == (*it) )
			{
				it++;
				if(it !=  m_fileIndexListForOne.end() )
				{
					m_curFileIndexPtr = (*it);
					m_fileStream.close();
					m_fileStream.open(m_curFileIndexPtr->GetFilePath().c_str(), std::ios_base::in|std::ios_base::binary);
					if( !m_fileStream.good() || m_fileStream.fail() )return false;
					return SkipDataHead();
				}
				else
				{
					CFileIndexList::CFileIndexPtr Tmp;
					m_curFileIndexPtr = Tmp;

					m_fileStream.close();
					m_pPreAllocDisk->m_log.Add("CFileRead::GetNextFileIndex No Next File cameraID = %d !", m_cameraid);
					return false;
				}
			}
		}
	}
	catch(...)
	{ m_pPreAllocDisk->m_log.Add( "CFileRead::GetNextFileIndex Catch Error cameraID = %d !", m_cameraid ); return false; }
	return false;
}

bool CFileRead::SkipDataHead()
{
	try
	{
		if( !m_fileStream.good() || m_fileStream.fail() )return false;
		m_fileStream.seekg(0, std::ios_base::beg);

		char signhead[12];
		m_fileStream.read((char*)signhead, 12);
		if(m_fileStream.gcount() < 12)return false;

		int isign = 0, ilen = 0;
		memcpy(&isign , signhead, sizeof(int));
		memcpy(&ilen, signhead + sizeof(int), sizeof(int));
		m_fileStream.seekg(12 + ilen, std::ios_base::beg);
	}
	catch(...){ return false; }
	return true;
}

}// end namespace
