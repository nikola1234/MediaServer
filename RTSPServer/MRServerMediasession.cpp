#include "MRServerMediasession.h"
#include "RTSPClient.h"

#include "ManageRecord.h"
//#include "windows.h"
//#include "HIKAnalyze/AnalyzeInterface_New.h"

namespace live555RTSP
{
MRServerMediasession::MRServerMediasession(RTSPServer* ourserver, CDataInfo::DBCAMERAINFO &cameradata, int coderid, std::string urlSuffix)
	: ServerMediasession(ourserver, cameradata, coderid)
	,m_fileRead(ourserver->GetPreAllocateDisk())
	,m_isThreadPause(true)
	,m_isThreadStop(false)
	,m_scale(1.0)
	,m_iIFrameDataSize(IFRAME_MAX_DATASIZE)
	,m_urlSuffix(urlSuffix)
	,m_pArrayTimeNode(NULL)
	,m_fileCurPos(0)
	,m_curSeconds(0)
{
	m_bliveMediaSession = false;

	if( ourserver->GetPreAllocateDisk() == NULL )
		m_isThreadStop = true;

	if( m_urlSuffix.size() > 0 && ourserver->GetPreAllocateDisk() != NULL)
	{
		m_storagePath = ourserver->GetPreAllocateDisk()->GetStoragePath();

		try
		{
			m_pArrayTimeNode = new CFileTimeNode[ TIMENODE_NUM];
			int pos1 = m_urlSuffix.find_first_of('_');
			int pos2 = m_urlSuffix.find_first_of('.');
			if( pos1 > 0 && pos2 > 0 ){
				std::string strBeginTime = m_urlSuffix.substr(pos1 + 1, pos2 - pos1 - 1 );

				boost::posix_time::ptime t = boost::posix_time::from_iso_string( strBeginTime );
				int totalSecs = t.time_of_day().total_seconds();
				m_begin_ptime = t;
				//boost::posix_time::time_duration time_d(0,0,10);
				//std::string str1 = boost::posix_time::to_iso_string(t);
				//t += time_d;
				//std::string str2 = boost::posix_time::to_iso_string(t);

				std::string strIndexPath = m_storagePath + "_MANUAL_RECORD_\\"; 
				strIndexPath += m_urlSuffix.substr( 0, pos1 ) + ".index";

				std::fstream fstream_;
				fstream_.open(strIndexPath.c_str(), std::ios_base::in|std::ios_base::binary);
				if( fstream_.good() && !fstream_.fail() )
				{
					fstream_.seekg( (sizeof(time_t) + sizeof(int)) * totalSecs, std::ios_base::beg );
				}

				for( int i = 0; i < TIMENODE_NUM; i++ )
				{
					fstream_.read((char*)(&m_pArrayTimeNode[i].m_time_t), sizeof(time_t));
					if( fstream_.gcount() < sizeof(time_t) )break;
					fstream_.read((char*)(&m_pArrayTimeNode[i].m_offset), sizeof(int));
					if( fstream_.gcount() < sizeof(int) )break;
				}
				fstream_.close();
			}
		}catch (...){}
		return;
	}

	if(  !m_fileRead.InitFileRead(this->GetCurCameraID())  )
		return;
	m_thread_ptr = ( boost::shared_ptr<boost::thread> )( new boost::thread(
		boost::bind(&MRServerMediasession::handleReadSendDataThread, this)) );
}

MRServerMediasession::~MRServerMediasession(void)
{
	StopMediaServer();
	if( m_pArrayTimeNode != NULL )
		delete[] m_pArrayTimeNode;
}

int MRServerMediasession::startStream(unsigned clientSessionId, std::string time_param, float scale)
{
	if ( m_isThreadStop ) {
		strcpy(GetStartStreamResultInfo(), "存储没有配置或者启动！");
		return -1;
	}

	if( m_urlSuffix.size() > 0  ) //文件读写
	{
		boost::unique_lock< boost::mutex > lock_( m_fileReadMutex );
		if(!m_fileStream.is_open())
		{
			bool bRet = true;
			std::string strFullPath = m_storagePath + "_MANUAL_RECORD_\\" + m_urlSuffix;
			try{
				m_fileStream.open(strFullPath.c_str(), std::ios_base::in|std::ios_base::binary);
				if(!m_fileStream.good() ||m_fileStream.fail() ) bRet = false;
			}catch(...){ bRet = false; }
			if ( !bRet ){
				strcpy( GetStartStreamResultInfo(), "对不起，存储文件不存在，或者读写失败！" );
				return -1;
			}else
			{
				m_thread_ptr = ( boost::shared_ptr<boost::thread> )(
					new boost::thread( boost::bind(&MRServerMediasession::handleReadFileSendThread, this)) );
			}
		}
		else
		{
			if( time_param.length() >= 32 )
			{
				std::string begin_str = time_param.substr( 0, 16 );
				std::string end_str = time_param.substr( 17, 16 );

				boost::posix_time::ptime t = boost::posix_time::from_iso_string( begin_str );
				boost::posix_time::time_duration time_d = t - m_begin_ptime;
				int iseconds = time_d.total_seconds();
				if( iseconds >= 0 && iseconds < TIMENODE_NUM )
				{
					m_fileStream.seekg( m_pArrayTimeNode[iseconds].m_offset, std::ios_base::beg );
				}
			}
		}
	}

	//time_param = 20130712T095830Z-20130712T100520Z
	if( time_param.length() >= 32 && m_urlSuffix.size() == 0 ) //24小时存储
	{
		std::string begin_str = time_param.substr( 0, 16 );
		std::string end_str = time_param.substr( 17, 16 );

		boost::unique_lock< boost::mutex > lock_( m_fileReadMutex );
		//if( !m_fileRead.SetReadTime(begin_str, end_str) )
		//	/*return -1*/;
		m_fileRead.SetReadTime( begin_str, end_str );
		try{
			if( m_begin_ptime.is_not_a_date_time() )
				m_begin_ptime = boost::posix_time::from_iso_string( begin_str );
		}catch(...){}
	}

	//m_mediastate = WORKING;
	m_isThreadPause = false;
	m_scale = scale;
	m_iIFrameDataSize = 0;
	return 0;
}

char* MRServerMediasession::GetSaveFileInfo(std::string time_param)
{
	char* pSaveFileinfo = new char[65536];
	memset(pSaveFileinfo, 0, 65536);

	if( time_param.length() >= 32 ){
		std::string begin_str = time_param.substr(0,16);
		std::string end_str = time_param.substr(17,16);
		std::string fileinfo;
		m_fileRead.SetReadTime(begin_str, end_str);
		if(m_fileRead.GetSaveFileInfo(fileinfo))
			strcpy(pSaveFileinfo, fileinfo.c_str());
	}
	
	return pSaveFileinfo;
}

std::string MRServerMediasession::GetCurPlayBackTime()
{
	std::string strT = "";

	if( m_urlSuffix.size() > 0 && m_pArrayTimeNode != NULL )
	{
		m_curSeconds = 0;
		for( int i = 0; i < TIMENODE_NUM - 1; i++ )
		{
			if( m_pArrayTimeNode[i].m_offset <= m_fileCurPos &&
				m_pArrayTimeNode[i + 1].m_offset >= m_fileCurPos )
			{
				m_curSeconds = i;
				break;
			}
		}
	}

	try{
		boost::posix_time::time_duration time_d(0,0,m_curSeconds);
		boost::posix_time::ptime t = m_begin_ptime + time_d;
		strT = boost::posix_time::to_iso_string(t) + "Z";
	}catch(...){};

	return strT;
}

int MRServerMediasession::pauseStream()
{
	m_isThreadPause = true;
	return 0;
}

int MRServerMediasession::StopMediaServer()
{
	//if(m_mediastate != WORKING)return -1;
	//m_mediastate = STOP;
	m_isThreadPause = true;
	m_isThreadStop = true;
	if(m_thread_ptr != NULL)
		m_thread_ptr->join();
	return 0;
}

void MRServerMediasession::handleReadSendDataThread()
{
	boost::shared_ptr<char> databuf = boost::shared_ptr<char>(new char[RAWDATALEN]);
	char* pdata = (char *)databuf.get();

	//HANDLE hAnalyzeHandle = NULL;
	//PACKET_INFO_EX packetinfo;
	//memset(&packetinfo,0,sizeof(PACKET_INFO_EX));

	int preClocks = -1;

	int s_count = 0;
	bool bHead = false;
	while(!m_isThreadStop)
	{
		if(m_isThreadPause){
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
			continue;
		}
		if(m_isThreadStop)break;

		int isecond = 0;
		int iReadLen =  0;

		boost::unique_lock<boost::mutex> lock_(m_fileReadMutex);
		iReadLen = m_fileRead.ReadData(pdata, isecond);
		lock_.unlock();
		m_curSeconds = isecond;

		s_count++;
		if( iReadLen > 0 )
		{
			int isign = 0, dwDataType = 0, ilen = 0;
			memcpy(&isign, pdata, sizeof(int));
			memcpy(&ilen, pdata + sizeof(int), sizeof(int));
			memcpy(&dwDataType, pdata + sizeof(int)*2, sizeof(int));

			int clocks = (dwDataType >> 16)&0xFFFF;
			if(preClocks < 0) preClocks = clocks;
			int betClocks = 0;
			if( clocks >= preClocks ){
				betClocks = clocks - preClocks;
			}else{
				betClocks = (65536 - preClocks) + clocks;
			}

			if ( isign == 0xAAAB ){
				bHead = true;
				dwDataType = (1<<16) + (dwDataType&0xFFFF);
				//if(hAnalyzeHandle != NULL)
				//	HIKANA_Destroy(hAnalyzeHandle);
				//hAnalyzeHandle = HIKANA_CreateStreamEx( 1024*1024, (unsigned char*)(pdata + 12) );
			}else{
				dwDataType = (dwDataType&0xFFFF);
			}
			betClocks = int(((float)((float)betClocks/CLOCKS_PER_SEC)) * 1000);

			float x_f = m_scale;
			if( m_scale < 0.01 )x_f = 1.0;
			double _scale = 1.0 / x_f;
			int i_scale = 0;
			if( m_scale < 1.0 )
				i_scale = (int)_scale;
			else
				i_scale = (int)m_scale;

			switch(i_scale){
			case 1: i_scale = 1;break;
			case 2: i_scale = 2;break;
			case 3: i_scale = 4;break;
			case 4: i_scale = 8;break;
			case 5: i_scale = 16;break;
			case 6: i_scale = 32;break;
			case 7: i_scale = 64;break;
			default:i_scale = 1;break;
			}
			if( m_scale < 0.01 )i_scale = 0;

			//if( m_scale > 1.0 && i_scale >= 16 )
			//{
			//	HIKANA_InputData(hAnalyzeHandle, (unsigned char*)pdata + 12, ilen);
			//	int nret = 0;
			//	if((nret = HIKANA_GetOnePacketEx(hAnalyzeHandle,&packetinfo)) == 0)
			//	{
			//		if( packetinfo.nPacketType != 1 || (s_count%(i_scale/16)) != 0 )continue;

			//		memcpy(pdata + 12, packetinfo.pPacketBuffer, packetinfo.dwPacketSize);
			//		iReadLen = packetinfo.dwPacketSize + 12;
			//	}
			//}

			//if( bHead )
			//{
			//	HIKANA_InputData(hAnalyzeHandle, (unsigned char*)pdata + 12, ilen);
			//	int nret = 0;
			//	if( isign != 0xAAAB ){
			//		if((nret = HIKANA_GetOnePacketEx(hAnalyzeHandle,&packetinfo)) == 0)
			//		{
			//			if( packetinfo.nPacketType != 1 )continue;
			//			bHead = false;
			//			memcpy(pdata + 12, packetinfo.pPacketBuffer, packetinfo.dwPacketSize);
			//			iReadLen = packetinfo.dwPacketSize + 12;
			//		}
			//	}
			//}

			SendData(dwDataType, iReadLen - 12, (unsigned char*)pdata + 12, isecond);

			//if( m_scale > 0.1 )
			//	boost::this_thread::sleep( boost::posix_time::milliseconds( (int)(40.0 / m_scale) ) );

			//char debuginfo[1024];
			//sprintf(debuginfo, "MRServerMediasession isecond = %d betClocks = %d i_scale = %d \n", isecond, betClocks, i_scale);
			//OutputDebugString(debuginfo);

			if(  betClocks < 5 || betClocks > 500 )betClocks = 0;

			if( m_iIFrameDataSize < IFRAME_MAX_DATASIZE ){ //加快图像启动显示速度
				m_iIFrameDataSize += iReadLen - 12;
				betClocks = 0;
			}

			if( i_scale == 0 ) { i_scale = 1; betClocks = 0; };//解决除零错误0xc0000094。
			if( m_scale < 1.0 )
				boost::this_thread::sleep( boost::posix_time::milliseconds( betClocks * i_scale ) );
			else
				boost::this_thread::sleep( boost::posix_time::milliseconds( betClocks / i_scale ) );
			preClocks = clocks;
		}else{
			m_isThreadPause = true;
			boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		}
	}
	//if(hAnalyzeHandle != NULL)
	//	HIKANA_Destroy(hAnalyzeHandle);
}

void MRServerMediasession::handleReadFileSendThread()
{
	boost::shared_ptr<char> databuf = boost::shared_ptr<char>(new char[RAWDATALEN]);
	char* pdata = (char *)databuf.get();

	int preClocks = -1;
	int s_count = 0;
	bool bHead = false;

	while(!m_isThreadStop)
	{
		if(m_isThreadPause){
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
			continue;
		}
		if(m_isThreadStop)break;

		int isecond = 0;
		int iReadLen =  0;

		try{
			boost::unique_lock< boost::mutex > lock_( m_fileReadMutex );
			m_fileStream.read(pdata, 12);
			int readlen = (int)m_fileStream.gcount();
			if( readlen < 12 ) break;
			if(!m_fileStream.good() || m_fileStream.fail())break;

			int isign,ilen;
			memcpy(&isign, pdata, sizeof(int));
			memcpy(&ilen, pdata + sizeof(int), sizeof(int));
			if(isign == 0xAAAA || isign == 0xAAAB)
			{
				m_fileStream.read(pdata + 12, ilen);
				m_fileCurPos = (int)m_fileStream.tellg();
				int readlen = (int)m_fileStream.gcount();
				if(readlen != ilen)break;
				if(!m_fileStream.good() || m_fileStream.fail())break;
				iReadLen = 12 + ilen;
			}else
			{
				break;
			}
		}catch(...){}

		s_count++;
		if( iReadLen > 0 )
		{
			int isign = 0, dwDataType = 0, ilen = 0;
			memcpy(&isign, pdata, sizeof(int));
			memcpy(&ilen, pdata + sizeof(int), sizeof(int));
			memcpy(&dwDataType, pdata + sizeof(int)*2, sizeof(int));

			int clocks = (dwDataType >> 16)&0xFFFF;
			if(preClocks < 0) preClocks = clocks;
			int betClocks = 0;
			if( clocks >= preClocks ){
				betClocks = clocks - preClocks;
			}else{
				betClocks = (65536 - preClocks) + clocks;
			}

			if ( isign == 0xAAAB ){
				bHead = true;
				if ( (dwDataType&0xFFFF) == 20483 )
					dwDataType = (1<<16);
				else
				dwDataType = (1<<16) + (dwDataType&0xFFFF);
			}else{
				if ( (dwDataType&0xFFFF) == 20483 )
					dwDataType = 0;
				else
				dwDataType = (dwDataType&0xFFFF);
			}
			betClocks = int(((float)((float)betClocks/CLOCKS_PER_SEC)) * 1000);

			float x_f = m_scale;
			if( m_scale < 0.01 )x_f = 1.0;
			double _scale = 1.0 / x_f;
			int i_scale = 0;
			if( m_scale < 1.0 )
				i_scale = (int)_scale;
			else
				i_scale = (int)m_scale;

			switch(i_scale){
			case 1: i_scale = 1;break;
			case 2: i_scale = 2;break;
			case 3: i_scale = 4;break;
			case 4: i_scale = 8;break;
			case 5: i_scale = 16;break;
			case 6: i_scale = 32;break;
			case 7: i_scale = 64;break;
			default:i_scale = 1;break;
			}
			if( m_scale < 0.01 )i_scale = 0;

			if ( (dwDataType&0xFFFF) == 0 )
				HandleH264PackData(dwDataType, iReadLen - 12, (unsigned char*)pdata + 12, isecond);
			else
				SendData(dwDataType, iReadLen - 12, (unsigned char*)pdata + 12);
				//SendData(dwDataType, iReadLen - 12, (unsigned char*)pdata + 12, isecond);

			if(  betClocks < 5 || betClocks > 500 )betClocks = 0;

			if( m_iIFrameDataSize < IFRAME_MAX_DATASIZE ){ //加快图像启动显示速度
				m_iIFrameDataSize += iReadLen - 12;
				betClocks = 0;
			}

			if( i_scale == 0 ) { i_scale = 1; betClocks = 0; };//解决除零错误0xc0000094。
			if( m_scale < 1.0 )
				boost::this_thread::sleep( boost::posix_time::milliseconds( betClocks * i_scale ) );
			else
				boost::this_thread::sleep( boost::posix_time::milliseconds( betClocks / i_scale ) );
			preClocks = clocks;
		}else{
			m_isThreadPause = true;
			boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		}
	}
	m_fileStream.close();
}

} // end namespace