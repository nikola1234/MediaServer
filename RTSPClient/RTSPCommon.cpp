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
// Implementation

#include "RTSPCommon.h"
//#include "Locale.hh"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> // for "strftime()" and "gmtime()"

Boolean parseRTSPRequestString(char const* reqStr,
			       unsigned reqStrSize,
			       char* resultCmdName,
			       unsigned resultCmdNameMaxSize,
			       char* resultURLPreSuffix,
			       unsigned resultURLPreSuffixMaxSize,
			       char* resultURLSuffix,
			       unsigned resultURLSuffixMaxSize,
			       char* resultCSeq,
			       unsigned resultCSeqMaxSize,
			       unsigned& contentLength) {
  // This parser is currently rather dumb; it should be made smarter #####
  contentLength = 0; // default value

  // Read everything up to the first space as the command name:
  Boolean parseSucceeded = False;
  unsigned i;
  for (i = 0; i < resultCmdNameMaxSize-1 && i < reqStrSize; ++i) {
    char c = reqStr[i];
    if (c == ' ' || c == '\t') {
      parseSucceeded = True;
      break;
    }
    resultCmdName[i] = c;
  }
  resultCmdName[i] = '\0';
  if (!parseSucceeded) return False;

  // Skip over the prefix of any "rtsp://" or "rtsp:/" URL that follows:
  unsigned j = i+1;
  while (j < reqStrSize && (reqStr[j] == ' ' || reqStr[j] == '\t')) ++j; // skip over any additional white space
  for (; (int)j < (int)(reqStrSize-8); ++j) {
    if ((reqStr[j] == 'r' || reqStr[j] == 'R')
	&& (reqStr[j+1] == 't' || reqStr[j+1] == 'T')
	&& (reqStr[j+2] == 's' || reqStr[j+2] == 'S')
	&& (reqStr[j+3] == 'p' || reqStr[j+3] == 'P')
	&& reqStr[j+4] == ':' && reqStr[j+5] == '/') {
      j += 6;
      if (reqStr[j] == '/') {
	// This is a "rtsp://" URL; skip over the host:port part that follows:
	++j;
	while (j < reqStrSize && reqStr[j] != '/' && reqStr[j] != ' ') ++j;
      } else {
	// This is a "rtsp:/" URL; back up to the "/":
	--j;
      }
      i = j;
      break;
    }
  }

  // Look for the URL suffix (before the following "RTSP/"):
  parseSucceeded = False;
  for (unsigned k = i+1; (int)k < (int)(reqStrSize-5); ++k) {
    if (reqStr[k] == 'R' && reqStr[k+1] == 'T' &&
	reqStr[k+2] == 'S' && reqStr[k+3] == 'P' && reqStr[k+4] == '/') {
      while (--k >= i && reqStr[k] == ' ') {} // go back over all spaces before "RTSP/"
      unsigned k1 = k;
      while (k1 > i && reqStr[k1] != '/') --k1;

      // The URL suffix comes from [k1+1,k]
      // Copy "resultURLSuffix":
      if (k - k1 + 1 > resultURLSuffixMaxSize) return False; // there's no room
      unsigned n = 0, k2 = k1+1;
      while (k2 <= k) resultURLSuffix[n++] = reqStr[k2++];
      resultURLSuffix[n] = '\0';

      // The URL 'pre-suffix' comes from [i+1,k1-1]
      // Copy "resultURLPreSuffix":
      if (k1 - i > resultURLPreSuffixMaxSize) return False; // there's no room
      n = 0; k2 = i + 1;
      while (k2 <= k1 - 1) resultURLPreSuffix[n++] = reqStr[k2++];
      resultURLPreSuffix[n] = '\0';

      i = k + 7; // to go past " RTSP/"
      parseSucceeded = True;
      break;
    }
  }
  if (!parseSucceeded) return False;

  // Look for "CSeq:", skip whitespace,
  // then read everything up to the next \r or \n as 'CSeq':
  parseSucceeded = False;
  for (j = i; (int)j < (int)(reqStrSize-5); ++j) {
    if (reqStr[j] == 'C' && reqStr[j+1] == 'S' && reqStr[j+2] == 'e' &&
	reqStr[j+3] == 'q' && reqStr[j+4] == ':') {
      j += 5;
      while (j < reqStrSize && (reqStr[j] ==  ' ' || reqStr[j] == '\t')) ++j;
      unsigned n;
      for (n = 0; n < resultCSeqMaxSize-1 && j < reqStrSize; ++n,++j) {
	char c = reqStr[j];
	if (c == '\r' || c == '\n') {
	  parseSucceeded = True;
	  break;
	}

	resultCSeq[n] = c;
      }
      resultCSeq[n] = '\0';
      break;
    }
  }
  if (!parseSucceeded) return False;

  // Also: Look for "Content-Length:" (optional)
  for (j = i; (int)j < (int)(reqStrSize-15); ++j) {
    if (reqStr[j] == 'C' && reqStr[j+1] == 'o' && reqStr[j+2] == 'n' && reqStr[j+3] == 't' && reqStr[j+4] == 'e' &&
	reqStr[j+5] == 'n' && reqStr[j+6] == 't' && reqStr[j+7] == '-' && (reqStr[j+8] == 'L' || reqStr[j+8] == 'l') &&
	reqStr[j+9] == 'e' && reqStr[j+10] == 'n' && reqStr[j+11] == 'g' && reqStr[j+12] == 't' && reqStr[j+13] == 'h' &&
	reqStr[j+14] == ':') {
      j += 15;
      while (j < reqStrSize && (reqStr[j] ==  ' ' || reqStr[j] == '\t')) ++j;
      unsigned num;
      if (sscanf(&reqStr[j], "%u", &num) == 1) {
	contentLength = num;
      }
    }
  }
  return True;
}

static void lookForHeader(char const* headerName, char const* source, unsigned sourceLen, char* resultStr, unsigned resultMaxSize) {
	resultStr[0] = '\0';  // by default, return an empty string
	unsigned headerNameLen = strlen(headerName);
	for (int i = 0; i < (int)(sourceLen-headerNameLen); ++i) {
		if (strncmp(&source[i], headerName, headerNameLen) == 0 && source[i+headerNameLen] == ':') {
			// We found the header.  Skip over any whitespace, then copy the rest of the line to "resultStr":
			for (i += headerNameLen+1; i < (int)sourceLen && (source[i] == ' ' || source[i] == '\t'); ++i) {}
			for (unsigned j = i; j < sourceLen; ++j) {
				if (source[j] == '\r' || source[j] == '\n') {
					// We've found the end of the line.  Copy it to the result (if it will fit):
					if (j-i+1 > resultMaxSize) break;
					char const* resultSource = &source[i];
					char const* resultSourceEnd = &source[j];
					while (resultSource < resultSourceEnd) *resultStr++ = *resultSource++;
					*resultStr = '\0';
					break;
				}
			}
		}
	}
}

Boolean parseHTTPRequestString(char* fRequestBuffer,unsigned fRequestBytesAlreadySeen,
	char* resultCmdName, unsigned resultCmdNameMaxSize,
	char* urlSuffix, unsigned urlSuffixMaxSize,
	char* sessionCookie, unsigned sessionCookieMaxSize,
	char* acceptStr, unsigned acceptStrMaxSize) {
		// Check for the limited HTTP requests that we expect for specifying RTSP-over-HTTP tunneling.
		// This parser is currently rather dumb; it should be made smarter #####
		char const* reqStr = (char const*)fRequestBuffer;
		unsigned const reqStrSize = fRequestBytesAlreadySeen;

		// Read everything up to the first space as the command name:
		Boolean parseSucceeded = False;
		unsigned i;
		for (i = 0; i < resultCmdNameMaxSize-1 && i < reqStrSize; ++i) {
			char c = reqStr[i];
			if (c == ' ' || c == '\t') {
				parseSucceeded = True;
				break;
			}

			resultCmdName[i] = c;
		}
		resultCmdName[i] = '\0';
		if (!parseSucceeded) return False;

		// Look for the string "HTTP/", before the first \r or \n:
		parseSucceeded = False;
		for (; i < reqStrSize-5 && reqStr[i] != '\r' && reqStr[i] != '\n'; ++i) {
			if (reqStr[i] == 'H' && reqStr[i+1] == 'T' && reqStr[i+2]== 'T' && reqStr[i+3]== 'P' && reqStr[i+4]== '/') {
				i += 5; // to advance past the "HTTP/"
				parseSucceeded = True;
				break;
			}
		}
		if (!parseSucceeded) return False;

		// Get the 'URL suffix' that occurred before this:
		unsigned k = i-6;
		while (k > 0 && reqStr[k] == ' ') --k; // back up over white space
		unsigned j = k;
		while (j > 0 && reqStr[j] != ' ' && reqStr[j] != '/') --j;
		// The URL suffix is in position (j,k]:
		if (k - j + 1 > urlSuffixMaxSize) return False; // there's no room> 
		unsigned n = 0;
		while (++j <= k) urlSuffix[n++] = reqStr[j];
		urlSuffix[n] = '\0';

		// Look for various headers that we're interested in:
		lookForHeader("x-sessioncookie", &reqStr[i], reqStrSize-i, sessionCookie, sessionCookieMaxSize);
		lookForHeader("Accept", &reqStr[i], reqStrSize-i, acceptStr, acceptStrMaxSize);

		return True;
}

Boolean parseRangeParam(char const* paramStr, double& rangeStart, double& rangeEnd, char* clock_str) {
  double start, end;
  int numCharsMatched = 0;

  //Locale l("C", Numeric); 没有定义local
  if (sscanf(paramStr, "npt = %lf - %lf", &start, &end) == 2) {
    rangeStart = start;
    rangeEnd = end;
  } else if (sscanf(paramStr, "npt = %lf -", &start) == 1) {
    rangeStart = start;
    rangeEnd = 0.0;
  } else if (strcmp(paramStr, "npt=now-") == 0) {
    rangeStart = 0.0;
    rangeEnd = 0.0;
  } else if (sscanf(paramStr, "clock = %s", clock_str) == 1) {
    // We accept "clock=" parameters, but currently do no interpret them.
  } else if (sscanf(paramStr, "smtpe = %*s%n", &numCharsMatched) == 0 && numCharsMatched > 0) {
    // We accept "smtpe=" parameters, but currently do no interpret them.
  } else {
    return False; // The header is malformed
  }

  return True;
}

Boolean parseRangeHeader(char const* buf, double& rangeStart, double& rangeEnd, char* clock_str) {
  // First, find "Range:"
  while (1) {
    if (*buf == '\0') return False; // not found
    if (_strncasecmp(buf, "Range: ", 7) == 0) break;
    ++buf;
  }

  // Then, run through each of the fields, looking for ones we handle:
  char const* fields = buf + 7;
  while (*fields == ' ') ++fields;
  return parseRangeParam(fields, rangeStart, rangeEnd, clock_str);
}

char const* dateHeader() {
  static char buf[200];
#if !defined(_WIN32_WCE)
  time_t tt = time(NULL);
  strftime(buf, sizeof buf, "Date: %a, %b %d %Y %H:%M:%S GMT\r\n", gmtime(&tt));
#else
  // WinCE apparently doesn't have "time()", "strftime()", or "gmtime()",
  // so generate the "Date:" header a different, WinCE-specific way.
  // (Thanks to Pierre l'Hussiez for this code)
  // RSF: But where is the "Date: " string?  This code doesn't look quite right...
  SYSTEMTIME SystemTime;
  GetSystemTime(&SystemTime);
  WCHAR dateFormat[] = L"ddd, MMM dd yyyy";
  WCHAR timeFormat[] = L"HH:mm:ss GMT\r\n";
  WCHAR inBuf[200];
  DWORD locale = LOCALE_NEUTRAL;

  int ret = GetDateFormat(locale, 0, &SystemTime,
                          (LPTSTR)dateFormat, (LPTSTR)inBuf, sizeof inBuf);
  inBuf[ret - 1] = ' ';
  ret = GetTimeFormat(locale, 0, &SystemTime,
                      (LPTSTR)timeFormat,
                      (LPTSTR)inBuf + ret, (sizeof inBuf) - ret);
  wcstombs(buf, inBuf, wcslen(inBuf));
#endif
  return buf;
}

char* strDup(char const* str) {
	if (str == NULL) return NULL;
	size_t len = strlen(str) + 1;
	char* copy = new char[len];

	if (copy != NULL) {
		memcpy(copy, str, len);
	}
	return copy;
}

char* strDupSize(char const* str) {
	if (str == NULL) return NULL;
	size_t len = strlen(str) + 1;
	char* copy = new char[len];

	return copy;
}

char* getLine(char* startOfLine) {
	// returns the start of the next line, or NULL if none.  Note that this modifies the input string to add '\0' characters.
	for (char* ptr = startOfLine; *ptr != '\0'; ++ptr) {
		// Check for the end of line: \r\n (but also accept \r or \n by itself):
		if (*ptr == '\r' || *ptr == '\n') {
			// We found the end of the line
			if (*ptr == '\r') {
				*ptr++ = '\0';
				if (*ptr == '\n') ++ptr;
			} else {
				*ptr++ = '\0';
			}
			return ptr;
		}
	}
	return NULL;
}

static char base64DecodeTable[256];

static void initBase64DecodeTable() {
	int i;
	for (i = 0; i < 256; ++i) base64DecodeTable[i] = (char)0x80;
	// default value: invalid

	for (i = 'A'; i <= 'Z'; ++i) base64DecodeTable[i] = 0 + (i - 'A');
	for (i = 'a'; i <= 'z'; ++i) base64DecodeTable[i] = 26 + (i - 'a');
	for (i = '0'; i <= '9'; ++i) base64DecodeTable[i] = 52 + (i - '0');
	base64DecodeTable[(unsigned char)'+'] = 62;
	base64DecodeTable[(unsigned char)'/'] = 63;
	base64DecodeTable[(unsigned char)'='] = 0;
}

unsigned char* base64Decode(char* in, unsigned& resultSize,
	Boolean trimTrailingZeros) {
		static Boolean haveInitedBase64DecodeTable = False;
		if (!haveInitedBase64DecodeTable) {
			initBase64DecodeTable();
			haveInitedBase64DecodeTable = True;
		}

		unsigned char* out = (unsigned char*)strDupSize(in); // ensures we have enough space
		int k = 0;
		int const jMax = strlen(in) - 3;
		// in case "in" is not a multiple of 4 bytes (although it should be)
		for (int j = 0; j < jMax; j += 4) {
			char inTmp[4], outTmp[4];
			for (int i = 0; i < 4; ++i) {
				inTmp[i] = in[i+j];
				outTmp[i] = base64DecodeTable[(unsigned char)inTmp[i]];
				if ((outTmp[i]&0x80) != 0) outTmp[i] = 0; // pretend the input was 'A'
			}

			out[k++] = (outTmp[0]<<2) | (outTmp[1]>>4);
			out[k++] = (outTmp[1]<<4) | (outTmp[2]>>2);
			out[k++] = (outTmp[2]<<6) | outTmp[3];
		}

		if (trimTrailingZeros) {
			while (k > 0 && out[k-1] == '\0') --k;
		}
		resultSize = k;
		unsigned char* result = new unsigned char[resultSize];
		memmove(result, out, resultSize);
		delete[] out;

		return result;
}

static const char base64Char[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64Encode(char const* origSigned, unsigned origLength) {
	unsigned char const* orig = (unsigned char const*)origSigned; // in case any input bytes have the MSB set
	if (orig == NULL) return NULL;

	unsigned const numOrig24BitValues = origLength/3;
	Boolean havePadding = origLength > numOrig24BitValues*3;
	Boolean havePadding2 = origLength == numOrig24BitValues*3 + 2;
	unsigned const numResultBytes = 4*(numOrig24BitValues + havePadding);
	char* result = new char[numResultBytes+1]; // allow for trailing '\0'

	// Map each full group of 3 input bytes into 4 output base-64 characters:
	unsigned i;
	for (i = 0; i < numOrig24BitValues; ++i) {
		result[4*i+0] = base64Char[(orig[3*i]>>2)&0x3F];
		result[4*i+1] = base64Char[(((orig[3*i]&0x3)<<4) | (orig[3*i+1]>>4))&0x3F];
		result[4*i+2] = base64Char[((orig[3*i+1]<<2) | (orig[3*i+2]>>6))&0x3F];
		result[4*i+3] = base64Char[orig[3*i+2]&0x3F];
	}

	// Now, take padding into account.  (Note: i == numOrig24BitValues)
	if (havePadding) {
		result[4*i+0] = base64Char[(orig[3*i]>>2)&0x3F];
		if (havePadding2) {
			result[4*i+1] = base64Char[(((orig[3*i]&0x3)<<4) | (orig[3*i+1]>>4))&0x3F];
			result[4*i+2] = base64Char[(orig[3*i+1]<<2)&0x3F];
		} else {
			result[4*i+1] = base64Char[((orig[3*i]&0x3)<<4)&0x3F];
			result[4*i+2] = '=';
		}
		result[4*i+3] = '=';
	}

	result[numResultBytes] = '\0';
	return result;
}
