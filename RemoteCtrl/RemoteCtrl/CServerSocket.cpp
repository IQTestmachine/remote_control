#include "pch.h"
#include "CServerSocket.h"

//CServerSocket server;

CServerSocket* CServerSocket::m_instance = NULL;
CServerSocket::Helper CServerSocket::m_helper;

//CServerSocket* pserver = CServerSocket::getInstance();