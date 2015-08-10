#include "stdafx.h"
#include "ManageLogin.h"
#include "ConnectMC.h"


CManageLogin::CManageLogin(void)
{
}


CManageLogin::~CManageLogin(void)
{
}

CManageLogin::ConnectMCPtr CManageLogin::CreateConnectMC()
{
	ConnectMCPtr connectMCPtr_(new CConnectMC());
	writeLock wrlock(m_connectMCMutex);
	m_connectMCList.push_front(connectMCPtr_);
	return connectMCPtr_;
}

CManageLogin::ConnectMCPtr CManageLogin::FindConnectMCByLoginHandle(int loginHandle)
{
	ConnectMCPtr connectMCPtr_;

	readLock rdlock(m_connectMCMutex);
	std::list<ConnectMCPtr>::iterator it = m_connectMCList.begin();
	for (; it != m_connectMCList.end(); it++){
		if((*it)->GetLoginHandle() == loginHandle)
			return (*it);
	}

	return connectMCPtr_;
}

void CManageLogin::RemoveConnectMCByLoginHandle(int loginHandle)
{
	writeLock wrlock(m_connectMCMutex);
	std::list<ConnectMCPtr>::iterator it = m_connectMCList.begin();
	for (; it != m_connectMCList.end();it++){
		if((*it)->GetLoginHandle() == loginHandle){
			it = m_connectMCList.erase(it);
			return;
		}
	}
}

CManageLogin::ConnectMCPtr CManageLogin::IsExistConnectMC(char *ip, int port, char *username)
{
	ConnectMCPtr connectMCPtr_;

	readLock rdlock(m_connectMCMutex);
	std::list<ConnectMCPtr>::iterator it = m_connectMCList.begin();
	for (; it != m_connectMCList.end(); it++){
		if((*it)->IsExistConnectMC(ip, port, username))
			return (*it);
	}

	return connectMCPtr_;
}