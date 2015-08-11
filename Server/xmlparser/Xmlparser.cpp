
#include "Xmlparser.h"

CXmlparser::CXmlparser(char* infile)
{
	ConfigDocument= new TiXmlDocument(infile);
	ConfigDocument->LoadFile();
	RootElement  	  = ConfigDocument->RootElement();

	ServerElement =  RootElement->FirstChildElement();

	ServerIpElement 	 		 = ServerElement->FirstChildElement();
	ServerPositionElement  = ServerIpElement->NextSiblingElement();
}

CXmlparser::~CXmlparser()
{
}

void CXmlparser::GetConfigparam(T_ServerParam &t_SerParam)
{
	sprintf(t_SerParam.Serverip,"%s", ServerIpElement->FirstChild()->Value());
	t_SerParam.ServerID= (uint32)atoi(ServerPositionElement->FirstChild()->Value());
}
