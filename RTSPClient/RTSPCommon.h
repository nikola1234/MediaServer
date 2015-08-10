/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2012 Live Networks, Inc.  All rights reserved.
// Common routines used by both RTSP clients and servers
// C++ header

#ifndef _RTSP_COMMON_HH
#define _RTSP_COMMON_HH

//#ifndef _BOOLEAN_HH
//#include "Boolean.hh"
//#endif

//#ifndef _MEDIA_HH
//#include <Media.hh> // includes some definitions perhaps needed for Borland compilers?
//#endif

#ifdef   __BORLANDC__
#define Boolean bool
#define False false
#define True true
#else
typedef unsigned char Boolean;
#ifndef __MSHTML_LIBRARY_DEFINED__
#ifndef False
const Boolean False = 0;
#endif
#ifndef True
const Boolean True = 1;
#endif

#endif
#endif


#if defined(__WIN32__) || defined(_WIN32) || defined(_QNX4)
#define _strncasecmp _strnicmp
#define snprintf _snprintf
#else
#define _strncasecmp strncasecmp
#endif

#ifdef WIN32

/* Definitions of size-specific types: */
typedef __int64 int64_t;
typedef unsigned __int64 u_int64_t;
typedef unsigned u_int32_t;
typedef unsigned short u_int16_t;
typedef unsigned char u_int8_t;

#endif


typedef unsigned short portNumBits;


#define RTSP_PARAM_STRING_MAX 200
#define RTSP_BUFFER_SIZE (65536 + 512) // for incoming requests, and outgoing responses

Boolean parseRTSPRequestString(char const *reqStr, unsigned reqStrSize,
			       char *resultCmdName,
			       unsigned resultCmdNameMaxSize,
			       char* resultURLPreSuffix,
			       unsigned resultURLPreSuffixMaxSize,
			       char* resultURLSuffix,
			       unsigned resultURLSuffixMaxSize,
			       char* resultCSeq,
			       unsigned resultCSeqMaxSize,
			       unsigned& contentLength);

Boolean parseHTTPRequestString(char* fRequestBuffer,unsigned fRequestBytesAlreadySeen,
	char* resultCmdName, unsigned resultCmdNameMaxSize,
	char* urlSuffix, unsigned urlSuffixMaxSize,
	char* sessionCookie, unsigned sessionCookieMaxSize,
	char* acceptStr, unsigned acceptStrMaxSize);

Boolean parseRangeParam(char const* paramStr, double& rangeStart, double& rangeEnd, char* clock_str);
Boolean parseRangeHeader(char const* buf, double& rangeStart, double& rangeEnd, char* clock_str);

char const* dateHeader(); // A "Date:" header that can be used in a RTSP (or HTTP) response 

// Copyright (c) 1996-2012 Live Networks, Inc.  All rights reserved.
// A C++ equivalent to the standard C routine "strdup()".
// This generates a char* that can be deleted using "delete[]"
// Header

char* strDup(char const* str);
// Note: strDup(NULL) returns NULL

char* strDupSize(char const* str);
// Like "strDup()", except that it *doesn't* copy the original.
// (Instead, it just allocates a string of the same size as the original.)

char* getLine(char* startOfLine);

//char* base64_encode(const char* data, int data_len); 
//char* base64_decode(const char* data, int data_len); 

unsigned char* base64Decode(char* in, unsigned& resultSize,
	Boolean trimTrailingZeros = True);
// returns a newly allocated array - of size "resultSize" - that
// the caller is responsible for delete[]ing.

char* base64Encode(char const* orig, unsigned origLength);
// returns a 0-terminated string that
// the caller is responsible for delete[]ing.

#endif
