#include <memory.h>
#pragma once
#define RTP_VERSION 2 

typedef struct NAL_msg_s 
{ 
	bool eoFrame ; 
	unsigned char type;// NAL type 
	unsigned char *start;// pointer to first location in the send buffer 
	unsigned char *end;// pointer to last location in send buffer 
	unsigned long size ; 
} NAL_MSG_t; 

typedef struct 
{ 
	//LITTLE_ENDIAN 
	unsigned short   cc:4; /* CSRC count                 */ 
	unsigned short   x:1;  /* header extension flag      */ 
	unsigned short   p:1;  /* padding flag               */ 
	unsigned short   v:2;  /* packet type                */ 
	unsigned short   pt:7; /* payload type               */ 
	unsigned short   m:1;  /* marker bit                 */ 
	
	unsigned short    seq; /* sequence number            */ 
	unsigned long     ts;  /* timestamp                  */ 
	unsigned long     ssrc;/* synchronization source     */ 
} rtp_hdr_t; 

typedef struct tagRTP_INFO 
{ 
	NAL_MSG_t    nal;    // NAL information 
	rtp_hdr_t    rtp_hdr;// RTP header is assembled here 
	int hdr_len;// length of RTP header 
	
	unsigned char *pRTP; // pointer to where RTP packet has beem assembled 
	unsigned char *start;// pointer to start of payload 
	unsigned char *end;  // pointer to end of payload 
	
	unsigned int s_bit;  // bit in the FU header 
	unsigned int e_bit;  // bit in the FU header 
	bool FU_flag;        // fragmented NAL Unit flag 
} RTP_INFO; 

class CH264_RTP_PACK 
{ 
public: 
    CH264_RTP_PACK(unsigned long H264SSRC, unsigned char H264PAYLOADTYPE = 96
		, unsigned short MAXRTPPACKSIZE = 1200);
	~CH264_RTP_PACK(void);

    bool Set ( unsigned char *NAL_Buf, unsigned long NAL_Size
		, unsigned long Time_Stamp, bool End_Of_Frame);
	unsigned char* Get ( unsigned short *pPacketSize );
private: 
    unsigned int StartCode( unsigned char *cp ) ;
private: 
    RTP_INFO m_RTP_Info ; 
    bool m_bBeginNAL ; 
    unsigned short m_MAXRTPPACKSIZE ; 
};