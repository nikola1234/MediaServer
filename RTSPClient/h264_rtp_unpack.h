#pragma once
//// class CH264_RTP_UNPACK 

#include "Log.h"
//extern CLog g_MSNetSDKlog;

#ifndef WIN32
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef int                 INT;
typedef unsigned int        UINT;

typedef long HRESULT;

#define E_OUTOFMEMORY  (0x8007000EL)
#define S_OK           (0L)
#define S_FALSE        (1L)

#endif

//#define HAVE_LOG
#define ERROR_LOG
#define SUCCESS_LOG

static unsigned char cKeyHead_7[] = {
						 0x80, 0x60, 0xC8, 0xFA, 0x57, 0xFA, 0x44, 0xBF, 
						 0x64, 0x18, 0xD2, 0x77, 0x67, 0x64, 0x00, 0x28,
						 0xAD, 0x00, 0xCE, 0x50, 0x14, 0x01, 0x6E, 0xC0, 
						 0x44, 0x00, 0x00, 0x2E, 0xE0, 0x00, 0x0A, 0xFC,
						 0x81, 0x80, 0x00, 0x00, 0x42, 0x60, 0x00, 0x00,
						 0x0B, 0x10, 0x08, 0x8B, 0xDF, 0x8C, 0x00, 0x00,
						 0x03, 0x02, 0x13, 0x00, 0x00, 0x03, 0x00, 0x58, 
						 0x80, 0x44, 0x5E, 0xFC, 0x68 };

static unsigned char cKeyHead_8[] = {
						 0x80, 0x60, 0xC8, 0xFB, 0x57, 0xFA, 0x44, 0xBF, 
						 0x64, 0x18, 0xD2, 0x77, 0x68, 0xFE, 0x3C, 0xB0,
						 0x71, 0xD2 };
class CH264_RTP_UNPACK 
{ 
 
#define RTP_VERSION 2 
#define BUF_SIZE (1024 * 1024 * 2) 
 
    typedef struct 
    { 
        //LITTLE_ENDIAN 
        unsigned short   cc:4;/* CSRC count                 */ 
        unsigned short   x:1; /* header extension flag      */ 
        unsigned short   p:1; /* padding flag               */ 
        unsigned short   v:2; /* packet type                */ 
        unsigned short   pt:7;/* payload type               */ 
        unsigned short   m:1; /* marker bit                 */ 
 
        unsigned short    seq;/* sequence number            */ 
        unsigned long     ts; /* timestamp                  */ 
        unsigned long     ssrc;/*synchronization source     */ 
    } rtp_hdr_t; 
public: 
 
    CH264_RTP_UNPACK ( HRESULT &hr, unsigned char H264PAYLOADTYPE = 96 ); 
 
    ~CH264_RTP_UNPACK(void); 

	BYTE*  Parse_RTP_Packet ( BYTE *pBuf, unsigned short nSize, int *outSize ) ;
//pBuf为H264 RTP视频数据包，nSize为RTP视频数据包字节长度，outSize为输出视频数据帧字节长度。 
//返回值为指向视频数据帧的指针。输入数据可能被破坏。 
 
    void SetLostPacket() 
    { 
		//g_MSNetSDKlog.Add("SetLostPacket");
        m_bSPSFound = false ; 
        m_pStart = m_pBuf ; 
        m_dwSize = 0 ; 
// 		memset(cKeyHead7,0x00,200);
// 		memset(cKeyHead8,0x00,20);
    } 

// 	static unsigned char cKeyHead7[200];
// 	static unsigned char cKeyHead8[20];
	
private: 
    rtp_hdr_t m_RTP_Header ; 
 
    BYTE *m_pBuf ; 
 
    bool m_bSPSFound ; 
    BYTE *m_pStart ; 
    BYTE *m_pEnd ; 
    DWORD m_dwSize ; 
 
    WORD m_wSeq ; 
 
    BYTE m_H264PAYLOADTYPE ; 
    DWORD m_ssrc ; 
}; 
 
// class CH264_RTP_UNPACK end 
/*
//////////////////////////////////////////////////////////////////////////
//使用范例
HRESULT hr ; 
CH264_RTP_UNPACK unpack ( hr ) ; 
BYTE *pRtpData ; 
WORD inSize; 
int outSize ; 
BYTE *pFrame = unpack.Parse_RTP_Packet ( pRtpData, inSize, &outSize ) ; 
if ( pFrame != NULL ) 
{ 
// frame process 
// ... 
}
//////////////////////////////////////////////////////////////////////////
*/
