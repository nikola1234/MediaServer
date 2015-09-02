#ifndef _FILE_OPR_H_
#define _FILE_OPR_H_

#include "Common.h"

#include <fstream>
#include <cassert>
#include "json/json.h"

/* todo 增加报警服务器的ip 和 port*/

typedef struct  _SERVER_PARAM
{
	uint32 ServerID;
	char   Serverip[16];
	uint32 port;
	_SERVER_PARAM(){
		memset(this, 0, sizeof(_SERVER_PARAM));
	}

}T_ServerParam;

class CFileOpr
{

public:
	CFileOpr();
	~CFileOpr();

	int write_server_config(uint32 ServerID,uint32 port, string ServerIp);
	int read_server_config(T_ServerParam &t_SerParam);

protected:

private:
  string SerFile ;
  string CamFile ;
};

#endif
