
#include "FileOpr.h"

CFileOpr::CFileOpr()
{
	SerFile = "Server.json";
	CamFile = "Camera.json";
}

CFileOpr::~CFileOpr()
{

}

int CFileOpr::write_server_config(uint32 ServerID,uint32 port, string ServerIp)
{
	 Json::Value root;
	 Json::Value arrayObj;
	 Json::Value item;
	 Json::FastWriter writer;

	 ofstream ofs;
	 ofs.open(SerFile.c_str());
	 assert(ofs.is_open());

	 item["ID"] = Json::Value(ServerID);
	 item["ip"] = ServerIp;
	 item["port"] = Json::Value(port);
	 arrayObj.append(item);

	 root["AnaSvr"] = arrayObj;

	 item["ID"] = Json::Value(ServerID);
	 item["ip"] = ServerIp;
	 item["port"] = Json::Value(port);
	 arrayObj.append(item);
	 root["AlarmSvr"] = arrayObj;
	 
	 root.toStyledString();
	 std::string out = root.toStyledString();

	 ofs<<out;
	 ofs.close();
	 return 0;
}

int CFileOpr::read_server_config(T_ServerParam &t_SerParam)
{
	ifstream ifs;
	int i = 0;
	int size = 0;
	uint32 ID = 0;
	uint32 port =0;
	string ip;

	ifs.open(SerFile.c_str());
	assert(ifs.is_open());

	Json::Reader reader;
	Json::Value root;

	if (!reader.parse(ifs, root, false))
	{
		 return -1;
	}

	size = root["AnaSvr"].size();  
	for(i = 0; i < size; ++i)
	{
		ID = root["AnaSvr"][i]["ID"].asInt();
		port = root["AnaSvr"][i]["port"].asInt();
		ip = root["AnaSvr"][i]["ip"].asString();
		
		t_SerParam.ServerID   = ID;
		t_SerParam.ServerPort =  port;
		memcpy(t_SerParam.ServerIp ,ip.c_str(),ip.length());
	}

	size = root["AlarmSvr"].size(); 
	for(i = 0; i < size; ++i)
	{
		ID = root["AlarmSvr"][i]["ID"].asInt();
		port = root["AlarmSvr"][i]["port"].asInt();
		ip = root["AlarmSvr"][i]["ip"].asString();
		t_SerParam.AlarmID 	 = ID;
		t_SerParam.AlarmPort = port;
		memcpy(t_SerParam.AlarmIp ,ip.c_str(),ip.length());
	}
	return 0;
}
