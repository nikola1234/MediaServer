#pragma once

#ifndef UDPORTVECTOR_H
#define UDPORTVECTOR_H

#include <boost/thread.hpp>
#include <vector>

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>

#ifdef WIN32
#include <boost/interprocess/windows_shared_memory.hpp>
#else
#include <boost/interprocess/xsi_shared_memory.hpp>
#endif


namespace live555RTSP
{

class UDPPortVector
{

#define NO_SHARA_MEM

//#define BACKUP_UDPPORT 2000
#define BACKUP_UDPPORT 2001
#define BEGIN_UDPPORT  10001

#define PROC_MAX_NUM  25
#define PORT_INC_NUM  1500

#define CHENCK_SECONDS 60

	typedef struct _PROC_DATA
	{
		time_t cur_time;
	}PROC_DATA;

public:
	typedef struct _UDPPORT_
	{
		unsigned short	udpport;
		bool    bfree;
		_UDPPORT_()
		{
			udpport = 0;
			bfree = true;
		}
	} UDPPORT;

public:
	UDPPortVector()
		: s_udp_port_begin(0),
		s_udp_port_end(0),
		m_brun(true)
#ifndef NO_SHARA_MEM
		,m_sharedmem(boost::interprocess::open_or_create, "VNMP_UDP_PORT", 
		boost::interprocess::read_write, ( sizeof(PROC_DATA) * PROC_MAX_NUM ) )
#endif
	{
		try {

#ifndef NO_SHARA_MEM
			// access the mapped region using get_address
			boost::interprocess::mapped_region mmap( m_sharedmem , boost::interprocess::read_write );

			time_t now_t = time(NULL);
			int i = 0;

			PROC_DATA* pproc_data = (PROC_DATA *)mmap.get_address();
			for (i = 0; i < PROC_MAX_NUM; i++)
			{
				if ( difftime(now_t, pproc_data[i].cur_time) > (double)CHENCK_SECONDS ) //ten minute
				{
					pproc_data[i].cur_time = now_t;
					break;
				}
			}
			if( i < PROC_MAX_NUM )
			{
				s_udp_port_begin = BEGIN_UDPPORT + i * PORT_INC_NUM;
				thread_ = boost::thread(&UDPPortVector::KeepLiveThread , this , i);
			}
#endif
			if (s_udp_port_begin == 0)
			{
				srand( (unsigned)time( NULL ) );
#ifndef NO_SHARA_MEM
				s_udp_port_begin = BACKUP_UDPPORT + rand()%80 * 100;
#else
				s_udp_port_begin = BACKUP_UDPPORT + rand()%32 * PORT_INC_NUM;
#endif
			}
			std::cout << "get udp port begin = " << s_udp_port_begin << std::endl;
		} catch (boost::interprocess::interprocess_exception& e) {
			std::cout << "error = " << e.what() << " code = " << e.get_error_code() << std::endl;
		}

		boost::lock_guard<boost::mutex> lock_(udpport_mutex_);
		for (int i = s_udp_port_begin; i <= s_udp_port_begin + PORT_INC_NUM ; i += 2 )
		{
			UDPPORT udpport_;
			udpport_.udpport = i;
			udpport_.bfree = true;
			udpportvector.push_back(udpport_);
		}
	}

	~UDPPortVector()
	{
		m_brun = false;
		thread_.join();
		//WriteShareMem((s_udp_port_begin - BEGIN_UDPPORT) / PORT_INC_NUM , 0);
	}

	unsigned short GetFreeUDPPort()
	{
		boost::lock_guard<boost::mutex> lock_(udpport_mutex_);
		for (std::vector<UDPPORT>::size_type i = 0; i < udpportvector.size() ; i++ )
		{	
			if(udpportvector[i].bfree){
				udpportvector[i].bfree = false;
				return udpportvector[i].udpport;
			}
		}
		srand( (unsigned)time( NULL ) );
		int port = BACKUP_UDPPORT + rand()%8000;
		return port;
	}

	void FreeUDPPort(unsigned short port)
	{
		boost::lock_guard<boost::mutex> lock_(udpport_mutex_);
		for (std::vector<UDPPORT>::size_type i = 0; i < udpportvector.size() ; i++ )
		{	
			if(udpportvector[i].udpport == port){
				udpportvector[i].bfree = true;
				return;
			}
		}
	}

	void KeepLiveThread(int i)
	{
		int icount = 0;
		while(m_brun){
			WriteShareMem(i, time(NULL));
			icount = 0;
			while(m_brun && icount++ < (CHENCK_SECONDS / 2)){
				boost::this_thread::sleep(boost::posix_time::seconds(1));
			}
		}
	}

	void WriteShareMem(int i, time_t t)
	{
#ifndef NO_SHARA_MEM
		try {
			boost::interprocess::mapped_region mmap( m_sharedmem , boost::interprocess::read_write );
			PROC_DATA* pproc_data = (PROC_DATA *)mmap.get_address();
			pproc_data[i].cur_time = t;
		} catch (boost::interprocess::interprocess_exception& e) {
			std::cout << "error = " << e.what() << " code = " << e.get_error_code() << std::endl;
		}
#endif
	}

private:
	std::vector<UDPPORT> udpportvector;
	boost::mutex udpport_mutex_;

	int s_udp_port_begin;
	int s_udp_port_end;

	boost::thread thread_;
	bool m_brun;

#ifndef NO_SHARA_MEM

#ifdef WIN32
	boost::interprocess::windows_shared_memory m_sharedmem;
#else
	//boost::interprocess::xsi_shared_memory m_sharedmem;
	boost::interprocess::shared_memory_object m_sharedmem;
#endif

#endif
};

}// end namespace

#endif // UDPORTVECTOR_H