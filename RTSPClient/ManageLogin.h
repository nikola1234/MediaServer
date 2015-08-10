#pragma once

#include <list>
#include "boost/shared_ptr.hpp"
#include <boost/thread.hpp>
#include "boost/atomic.hpp"

class CConnectMC;
class CManageLogin
{
public:
	typedef boost::shared_ptr<CConnectMC> ConnectMCPtr;
	typedef boost::shared_lock<boost::shared_mutex> readLock;
	typedef boost::unique_lock<boost::shared_mutex> writeLock;
public:
	CManageLogin(void);
	~CManageLogin(void);

	ConnectMCPtr CreateConnectMC();
	ConnectMCPtr FindConnectMCByLoginHandle(int loginHandle);
	void RemoveConnectMCByLoginHandle(int loginHandle);

	ConnectMCPtr IsExistConnectMC(char *ip, int port, char *username);
private:
	std::list<ConnectMCPtr> m_connectMCList;
	boost::shared_mutex m_connectMCMutex;
};

