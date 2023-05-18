#include "pch.h"
#include "CClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;
CClientSocket::Helper CClientSocket::m_helper;

//CServerSocket* pserver = CServerSocket::getInstance();