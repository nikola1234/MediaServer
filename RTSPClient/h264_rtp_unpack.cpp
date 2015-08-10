
#include "stdafx.h"
#include "h264_rtp_unpack.h"

CH264_RTP_UNPACK::CH264_RTP_UNPACK ( HRESULT &hr, unsigned char H264PAYLOADTYPE ) 
: m_bSPSFound(false) 
, m_wSeq(0) 
, m_ssrc(0) 
{ 
	m_pBuf = new BYTE[BUF_SIZE] ; 
	if ( m_pBuf == NULL ) 
	{ 
		hr = E_OUTOFMEMORY ; 
		return ; 
	} 

	m_H264PAYLOADTYPE = H264PAYLOADTYPE ; 
	m_pEnd = m_pBuf + BUF_SIZE ; 
	m_pStart = m_pBuf ; 
	m_dwSize = 0 ; 
	hr = S_OK ; 
} 

CH264_RTP_UNPACK::~CH264_RTP_UNPACK(void) 
{ 
	delete [] m_pBuf ; 
} 


BYTE*   CH264_RTP_UNPACK::Parse_RTP_Packet ( BYTE *pBuf, unsigned short nSize, int *outSize ) 
{ 
 	if ( nSize <= 12 ) 
	{ 
		//g_MSNetSDKlog.Add("rtp size %d < 12",nSize);
		return NULL ;
	} 

 	BYTE *cp = (BYTE*)&m_RTP_Header ; 
 	cp[0] = pBuf[0] ; 
 	cp[1] = pBuf[1] ; 
 	m_RTP_Header.seq = pBuf[2] ; 
 	m_RTP_Header.seq <<= 8 ; 
 	m_RTP_Header.seq |= pBuf[3] ; 
 	m_RTP_Header.ts = pBuf[4] ; 
 	m_RTP_Header.ts <<= 8 ; 
 	m_RTP_Header.ts |= pBuf[5] ; 
 	m_RTP_Header.ts <<= 8 ; 
 	m_RTP_Header.ts |= pBuf[6] ; 
 	m_RTP_Header.ts <<= 8 ; 
 	m_RTP_Header.ts |= pBuf[7] ; 
 	m_RTP_Header.ssrc = pBuf[8] ; 
 	m_RTP_Header.ssrc <<= 8 ; 
 	m_RTP_Header.ssrc |= pBuf[9] ; 
 	m_RTP_Header.ssrc <<= 8 ; 
 	m_RTP_Header.ssrc |= pBuf[10] ; 
 	m_RTP_Header.ssrc <<= 8 ; 
 	m_RTP_Header.ssrc |= pBuf[11] ; 
 
 	BYTE *pPayload = pBuf + 12 ; 
 	DWORD PayloadSize = nSize - 12 ; 

 	// Check the RTP version number (it should be 2): 
 	if ( m_RTP_Header.v != RTP_VERSION ) 
 	{ 
		//g_MSNetSDKlog.Add("rtp version error");
 		return NULL ; 
 	} 
	//Check the Payload Type. 
	if ( m_RTP_Header.pt != m_H264PAYLOADTYPE ) 
	{ 
		//g_MSNetSDKlog.Add("Check the Payload Type = %d", m_RTP_Header.pt);
		//return NULL ; 
	} 

	if (m_RTP_Header.seq != (WORD)(m_wSeq+1) )
	{
		//g_MSNetSDKlog.Add("m_RTP_Header.seq != (WORD)(m_wSeq+1)");
		m_wSeq = m_RTP_Header.seq;
		//SetLostPacket();
		//return NULL;
	}
	int PayloadType = pPayload[0] & 0x1f ; 
	int NALType = PayloadType ; 
#ifdef HAVE_LOG
	if (pLog)
		pLog->Add("seq %d--%d fu NAL-%d ebit-0x%x",m_RTP_Header.seq,nSize,NALType,pPayload[1]);
#endif
	if (NALType == 0x07)
	{
		m_bSPSFound = true;
	}
	if (!m_bSPSFound)
	{
		//return NULL;
	}

	if(NALType > 0 && NALType < 24)
	{
		m_wSeq = m_RTP_Header.seq ;
//		if (NALType == 0x07 || NALType == 0x08)
		{
//			m_wSeq = m_RTP_Header.seq ; 

			pPayload -= 4 ; 
			*((DWORD*)(pPayload)) = 0x01000000 ; 
			*outSize = PayloadSize + 4 ; 
#ifdef HAVE_LOG
			if (pLog)
 				pLog->Add("naltype %d,head type %x-%x",NALType,pPayload[4],pPayload[5]);
#endif
			return pPayload ; 
		}
//  		else
//  			return NULL;
	}
	else if (NALType == 24)	//STAP-A   单一时间的组合包
	{
		//g_MSNetSDKlog.Add("NALType == 24");
	}
	else if (NALType == 25)	//STAP-B   单一时间的组合包
	{
		//g_MSNetSDKlog.Add("NALType == 25");
	}
	else if (NALType == 26)	//MTAP16   多个时间的组合包
	{
		//g_MSNetSDKlog.Add("NALType == 25");
	}
	else if (NALType == 27)	//MTAP24   多个时间的组合包
	{
		//g_MSNetSDKlog.Add("NALType == 27");;
	}
	else if (NALType == 28)	//FU-A分片包，解码顺序和传输顺序相同
	{
		BYTE fu_ind = pPayload[0];
		BYTE fu_head = pPayload[1];
		m_wSeq = m_RTP_Header.seq ; 
		if (fu_head & 0x80) //FU_A start
		{
			*((DWORD*)(m_pStart)) = 0x01000000 ; 
			m_pStart += 4 ; 
			m_dwSize += 4 ; 

			pPayload[1] = ( pPayload[0] & 0xE0 ) | (fu_head & 0x1f) ; 

			pPayload += 1 ; 
			PayloadSize -= 1 ; 
		} 
		else 
		{ 
			pPayload += 2 ; 
			PayloadSize -= 2 ; 
		}			

		if ( m_pStart + PayloadSize < m_pEnd ) 
		{ 
			//CopyMemory ( m_pStart, pPayload, PayloadSize ) ; 
			memcpy( m_pStart, pPayload, PayloadSize );
			m_dwSize += PayloadSize ; 
			m_pStart += PayloadSize ; 
		} 
		else // memory overflow 
		{ 
			SetLostPacket () ;
// 			if (pLog)
// 				pLog->Add("memory over flow");		
			return NULL ; 
		}
		if ( m_RTP_Header.m ) // frame end 
		{ 
			*outSize = m_dwSize ; 
// 			if (pLog)
// 				pLog->Add("frame size %d,head type %x-%x",m_dwSize,m_pBuf[4],m_pBuf[5]);
			m_pStart = m_pBuf ; 
			m_dwSize = 0 ; 
		
			return m_pBuf ; 
		} 
		else 
		{ 
			return NULL ; 
		}
	}
	else if (NALType == 29)
	{
		if (m_RTP_Header.m == 1)
		{ 
			;
		}
		else if (m_RTP_Header.m == 0)
		{
			;
		}
	}
	else if (NALType > 29)
	{
		//g_MSNetSDKlog.Add("NALType > 29");
		//if (pLog)
		//	pLog->Add("error: pocket not defined ");
		SetLostPacket();
		return NULL;
	}
	//////////////////////////////////////////////////////////////////////////	

// 	if (pLog)
// 		pLog->Add("Parse pocket success");
	return m_pStart;
 } 