#pragma once

#include "Common.h"

#include "tinyxml.h"
#include "tinystr.h"

typedef struct  _SERVER_PARAM
{
  uint32 ServerID;
	char   Serverip[16];

	_SERVER_PARAM(){
		memset(this, 0, sizeof(_SERVER_PARAM));
	}

}T_ServerParam;

class CXmlparser
{

public:
	CXmlparser(char* infile);
	~CXmlparser();

	TiXmlDocument *ConfigDocument;
	TiXmlElement *RootElement;

	TiXmlElement * ServerElement;

	TiXmlElement *ServerIpElement;
	TiXmlElement *ServerPositionElement ;

	void GetConfigparam(T_ServerParam &t_SerParam);
protected:
private:
};
