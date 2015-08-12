
#include "FileOpr.h"

CFileOpr::CFileOpr()
{
	SerFile = "Server.json";
	CamFile = "Camera.json";
}

CFileOpr::~CFileOpr()
{

}

int CFileOpr::write_server_config(uint32 ServerID, string ServerIp)
{
	 Json::Value root;
	 Json::Value arrayObj;
	 Json::Value item;
	 Json::FastWriter writer;

	 ofstream ofs;
	 ofs.open(SerFile.c_str());
	 assert(ofs.is_open());

	 item["ServerID"] = Json::Value(ServerID);
	 item["ip"] = ServerIp;
	 arrayObj.append(item);

	 root["name"] = "server";
	 root["info"] = arrayObj;

	 root.toStyledString();
	 std::string out = root.toStyledString();
	 //std::cout << out << std::endl;
	 ofs<<out;
	 ofs.close();
	 return 0;
}

int CFileOpr::read_server_config(T_ServerParam &t_SerParam)
{
	ifstream ifs;
	uint32 ID = 0;
	string ip;

	ifs.open(SerFile.c_str());
	assert(ifs.is_open());

	Json::Reader reader;
	Json::Value root;

	if (!reader.parse(ifs, root, false))
	 {
			 return -1;
	 }

 	std::string name = root["name"].asString();
	//std::cout << name << std::endl;
	int size = root["info"].size();
	//std::cout << size << std::endl;

	for(int i = 0; i < size; ++i)
  {
     ID = root["info"][i]["ServerID"].asInt();
     ip = root["info"][i]["ip"].asString();
		 //cout<< ID << ip <<endl;
  }
	t_SerParam.ServerID = ID;
	memcpy(t_SerParam.Serverip ,ip.c_str(),ip.length());
	
	return 0;
}
