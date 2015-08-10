//#include "stdafx.h"
#include "H264_RTP_PACK.H"

CH264_RTP_PACK::CH264_RTP_PACK(unsigned long H264SSRC, unsigned char H264PAYLOADTYPE
							   ,unsigned short MAXRTPPACKSIZE) 
{ 
	m_MAXRTPPACKSIZE = MAXRTPPACKSIZE ; 
	if ( m_MAXRTPPACKSIZE > 10000 ) 
	{ 
		m_MAXRTPPACKSIZE = 10000 ; 
	}
	if ( m_MAXRTPPACKSIZE < 50 ) 
	{ 
		m_MAXRTPPACKSIZE = 50 ; 
	} 

	memset ( &m_RTP_Info, 0, sizeof(m_RTP_Info) ) ; 

	m_RTP_Info.rtp_hdr.pt = H264PAYLOADTYPE ; 
	m_RTP_Info.rtp_hdr.ssrc = H264SSRC ; 
	m_RTP_Info.rtp_hdr.v = RTP_VERSION ; 

	m_RTP_Info.rtp_hdr.seq = 0 ; 
} 

CH264_RTP_PACK::~CH264_RTP_PACK(void) 
{ 
} 

//#include "windows.h"
//#include "stdio.h"

//传入Set的数据必须是一个完整的NAL,起始码为0x00000001。 
//起始码之前至少预留12个字节，以避免内存COPY操作。 
//打包完成后，原缓冲区内的数据被破坏。 
bool CH264_RTP_PACK::Set ( unsigned char *NAL_Buf, unsigned long NAL_Size
						  , unsigned long Time_Stamp, bool End_Of_Frame ) 
{ 
//	TRACE(" CH264_RTP_PACK::Set = %d\n" ,NAL_Size);

	unsigned long startcode = StartCode(NAL_Buf) ; 

	if ( startcode != 0x01000000 ) 
	{ 
//		TRACE("error startcode = %x\n" ,startcode);
		return false ; 
	} 

	int type = NAL_Buf[4] & 0x1f ; 

	//char debuginfo[1024];
	//sprintf(debuginfo, "CH264_RTP_PACK::Set type = %d  size = %d \n", type, NAL_Size);
	//OutputDebugString(debuginfo);

	if ( type < 1 || type > 12 ) 
	{ 
//		TRACE("error nal.type = %d\n" ,type);
		return false ; 
	} 
//	TRACE("nal.type = %d\n" ,type);

	m_RTP_Info.nal.start = NAL_Buf ; 
	m_RTP_Info.nal.size = NAL_Size ; 
	m_RTP_Info.nal.eoFrame = End_Of_Frame ; 
	m_RTP_Info.nal.type = m_RTP_Info.nal.start[4] ; 
	m_RTP_Info.nal.end = m_RTP_Info.nal.start + m_RTP_Info.nal.size ; 

	m_RTP_Info.rtp_hdr.ts = Time_Stamp ; 

	m_RTP_Info.nal.start += 4 ;    // skip the syncword 

	if ( (m_RTP_Info.nal.size + 7) > m_MAXRTPPACKSIZE ) 
	{ 
		m_RTP_Info.FU_flag = true ; 
		m_RTP_Info.s_bit = 1 ; 
		m_RTP_Info.e_bit = 0 ; 

		m_RTP_Info.nal.start += 1 ;    // skip NAL header 
	} 
	else 
	{ 
		m_RTP_Info.FU_flag = false ; 
		m_RTP_Info.s_bit = m_RTP_Info.e_bit = 0 ; 
	} 

	m_RTP_Info.start = m_RTP_Info.end = m_RTP_Info.nal.start ; 
	m_bBeginNAL = true ; 

	return true ; 
}

//循环调用Get获取RTP包，直到返回值为NULL 
unsigned char* CH264_RTP_PACK::Get ( unsigned short *pPacketSize ) 
{ 
	if ( m_RTP_Info.end == m_RTP_Info.nal.end ) 
	{ 
		*pPacketSize = 0 ; 
		return 0; 
	} 

	if ( m_bBeginNAL ) 
	{ 
		m_bBeginNAL = false ; 
	} 
	else
	{ 
		m_RTP_Info.start = m_RTP_Info.end;// continue with the next RTP-FU packet 
	} 

	int bytesLeft = m_RTP_Info.nal.end - m_RTP_Info.start ; 
	int maxSize = m_MAXRTPPACKSIZE - 12 ; // sizeof(basic rtp header) == 12 bytes 
	if ( m_RTP_Info.FU_flag ) 
		maxSize -= 2 ; 

	if ( bytesLeft > maxSize ) 
	{
		// limit RTP packetsize to 1472 bytes
		m_RTP_Info.end = m_RTP_Info.start + maxSize ;
	} 
	else 
	{ 
		m_RTP_Info.end = m_RTP_Info.start + bytesLeft ; 
	} 

	if ( m_RTP_Info.FU_flag ) 
	{    // multiple packet NAL slice 
		if ( m_RTP_Info.end == m_RTP_Info.nal.end ) 
		{ 
			m_RTP_Info.e_bit = 1 ; 
		} 
	}
	// should be set at EofFrame
	m_RTP_Info.rtp_hdr.m =    m_RTP_Info.nal.eoFrame ? 1 : 0 ;
	if ( m_RTP_Info.FU_flag && !m_RTP_Info.e_bit ) 
	{ 
		m_RTP_Info.rtp_hdr.m = 0 ; 
	} 

	if( (m_RTP_Info.nal.type & 0x1f) > 5 || (m_RTP_Info.nal.type & 0x1f) < 1)
		m_RTP_Info.rtp_hdr.m = 0;

	//TRACE("m_RTP_Info.rtp_hdr.m = %d nal.type = %d\n" , m_RTP_Info.rtp_hdr.m, m_RTP_Info.nal.type & 0x1f);

	m_RTP_Info.rtp_hdr.seq++ ; 

	unsigned char *cp = m_RTP_Info.start ; 
	cp -= ( m_RTP_Info.FU_flag ? 14 : 12 ) ; 
	m_RTP_Info.pRTP = cp ; 

	unsigned char *cp2 = (unsigned char *)&m_RTP_Info.rtp_hdr ; 
	cp[0] = cp2[0] ; 
	cp[1] = cp2[1] ; 

	cp[2] = ( m_RTP_Info.rtp_hdr.seq >> 8 ) & 0xff ; 
	cp[3] = m_RTP_Info.rtp_hdr.seq & 0xff ; 

	cp[4] =(unsigned char) ( m_RTP_Info.rtp_hdr.ts >> 24 ) & 0xff ; 
	cp[5] =(unsigned char) ( m_RTP_Info.rtp_hdr.ts >> 16 ) & 0xff ; 
	cp[6] =(unsigned char) ( m_RTP_Info.rtp_hdr.ts >>  8 ) & 0xff ; 
	cp[7] =(unsigned char) m_RTP_Info.rtp_hdr.ts & 0xff ; 

	cp[8] = (unsigned char) ( m_RTP_Info.rtp_hdr.ssrc >> 24 ) & 0xff ; 
	cp[9] = (unsigned char) ( m_RTP_Info.rtp_hdr.ssrc >> 16 ) & 0xff ; 
	cp[10] =(unsigned char) ( m_RTP_Info.rtp_hdr.ssrc >>  8 ) & 0xff ; 
	cp[11] =(unsigned char) m_RTP_Info.rtp_hdr.ssrc & 0xff ; 
	m_RTP_Info.hdr_len = 12 ; 
	/*! 
	* \n The FU indicator octet has the following format: 
	* \n 
	* \n      +---------------+ 
	* \n MSB  |0|1|2|3|4|5|6|7|  LSB 
	* \n      +-+-+-+-+-+-+-+-+ 
	* \n      |F|NRI|  Type   | 
	* \n      +---------------+ 
	* \n 
	* \n The FU header has the following format: 
	* \n 
	* \n      +---------------+ 
	* \n      |0|1|2|3|4|5|6|7| 
	* \n      +-+-+-+-+-+-+-+-+ 
	* \n      |S|E|R|  Type   | 
	* \n      +---------------+ 
	*/ 
	if ( m_RTP_Info.FU_flag ) 
	{ 
		// FU indicator  F|NRI|Type 
		cp[12] = ( m_RTP_Info.nal.type & 0xe0 ) | 28 ;//Type is 28 for FU_A 
		//FU header        S|E|R|Type 
		cp[13] = ( m_RTP_Info.s_bit << 7 )
			| ( m_RTP_Info.e_bit << 6 ) | ( m_RTP_Info.nal.type & 0x1f ) ;
		//R = 0, must be ignored by receiver 

		//char debuginfo[1024];
		//sprintf(debuginfo, "CH264_RTP_PACK::Get s_bit = %d e_bit = %d \n", m_RTP_Info.s_bit, m_RTP_Info.e_bit);
		//OutputDebugString(debuginfo);

		m_RTP_Info.s_bit = m_RTP_Info.e_bit= 0 ; 
		m_RTP_Info.hdr_len = 14 ; 
	} 
	m_RTP_Info.start = &cp[m_RTP_Info.hdr_len] ;    // new start of payload 

	*pPacketSize = m_RTP_Info.hdr_len + ( m_RTP_Info.end - m_RTP_Info.start ) ; 

	//char debuginfo[1024];
	//sprintf(debuginfo, "CH264_RTP_PACK::Get m_RTP_Info.rtp_hdr.m = %d \n", m_RTP_Info.rtp_hdr.m);
	//OutputDebugString(debuginfo);

	return m_RTP_Info.pRTP ; 
}

unsigned int CH264_RTP_PACK::StartCode( unsigned char *cp ) 
{ 
	unsigned int d32 ;
	d32 = cp[3] ; 
	d32 <<= 8 ; 
	d32 |= cp[2] ; 
	d32 <<= 8 ; 
	d32 |= cp[1] ; 
	d32 <<= 8 ; 
	d32 |= cp[0] ; 
	return d32 ;
} 

