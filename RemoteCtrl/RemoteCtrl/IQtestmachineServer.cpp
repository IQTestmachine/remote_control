#include "pch.h"
#include "IQtestmachineServer.h"
#pragma warning(disable: 4407)

template<IQtestmachineOperator op>
AcceptOverlapped<op>::AcceptOverlapped()
{
    m_worker = CThreadWorker(this, (FUNCTYPE)&AcceptOverlapped<op>::AcceptWorker);
    m_operator = IQAccept;
    memset(&m_overlapped, 0, sizeof(m_overlapped));
    m_buffer.resize(1024);
    m_serv = NULL;
}

template<IQtestmachineOperator op>
int AcceptOverlapped<op>::AcceptWorker()
{
    INT lLength = 0, rLength = 0;
    if (m_clnt->GetBufferSize() > 0)
    {
		sockaddr* plocal = NULL, * premote = NULL;
        GetAcceptExSockaddrs(*m_clnt, 0,
            sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
            (sockaddr**)&plocal, &lLength,//本地地址
            (sockaddr**)&premote, &rLength);//远程地址
		memcpy(m_clnt->GetLocalAddr(), plocal, sizeof(sockaddr_in));
		memcpy(m_clnt->GetRemoteAddr(), premote, sizeof(sockaddr_in));
		m_serv->BindNewSocket(*m_clnt);
		int ret = WSARecv((SOCKET)*m_clnt, m_clnt->RecvWSABuffer(), 1, *m_clnt, &m_clnt->flags(), m_clnt->RecvOverlapped(), NULL);
		if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
        {
            //TODO:报错
			TRACE("ret = %d, error = %d\r\n", ret, WSAGetLastError());
        }
        if (!m_serv->NewAccept())
        {
            return -2;
        }
           
    }
    return -1;
}

template<IQtestmachineOperator op>
RecvOverlapped<op>::RecvOverlapped()
{
	m_worker = CThreadWorker(this, (FUNCTYPE)&RecvOverlapped<op>::RecvWorker);
	m_operator = IQRecv;
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024);
	m_serv = NULL;
}

template<IQtestmachineOperator op>
SendOverlapped<op>::SendOverlapped()
{
	m_worker = CThreadWorker(this, (FUNCTYPE)&SendOverlapped<op>::SendWorker);
	m_operator = IQSend;
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024 * 256);
	m_serv = NULL;
}

IQtestmachineClient::IQtestmachineClient()
	: m_isbusy(false), m_flags(0),
	m_overlapped(new ACCEPTOVERLAPPED()),
	m_recv(new RECVOVERLAPPED()),
	m_send(new SENDOVERLAPPED()),
	m_vecSend(this, (SENDCALLBACK)&IQtestmachineClient::SendData)
{
    m_client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    m_buffer.resize(1024);
    memset(&m_laddr, 0, sizeof(m_laddr));
    memset(&m_raddr, 0, sizeof(m_raddr));
}

void IQtestmachineClient::SetOverlapped(PCLIENT& ptr)
{
    m_overlapped->m_clnt = ptr.get();
	m_recv->m_clnt = ptr.get();
	m_send->m_clnt = ptr.get();
}

IQtestmachineClient::operator LPOVERLAPPED()
{
    return &m_overlapped->m_overlapped;
}

LPWSABUF IQtestmachineClient::RecvWSABuffer()
{
	return &m_recv->m_wsabuffer;
}

LPWSAOVERLAPPED IQtestmachineClient::RecvOverlapped()
{
	return &m_recv->m_overlapped;
}

LPWSABUF IQtestmachineClient::SendWSABuffer()
{
	return &m_send->m_wsabuffer;
}

LPWSAOVERLAPPED IQtestmachineClient::SendOverlapped()
{
	return &m_send->m_overlapped;
}

int IQtestmachineClient::Recv()
{
	int ret = recv(m_client, m_buffer.data() + m_used, m_buffer.size() - m_used, 0);
	if (ret <= 0)
		return -1;
	m_used += (size_t)ret;
	CIQtestmachineTool::Dump((BYTE*)m_buffer.data(), ret);
	//TODO: 解析数据
	return 0;
}

int IQtestmachineClient::Send(void* buffer, size_t nSize)
{
	std::vector<char> data(nSize);
	memcpy(data.data(), buffer, nSize);
	if (m_vecSend.PushBack(data))
		return 0;
	return -1;
}

int IQtestmachineClient::SendData(std::vector<char>& data)
{
	if (m_vecSend.Size() > 0)
	{
		int ret = WSASend(m_client, SendWSABuffer(), 1, &m_received, m_flags, &m_send->m_overlapped, NULL);
		if (ret != 0 && WSAGetLastError() != WSA_IO_PENDING)
			CIQtestmachineTool::ShowError();
		return -1;
	}
	return 0;
}

int CIQtestmachineServer::threadIocp()
{
	DWORD transferred = 0;
	ULONG_PTR CompletionKey;
	OVERLAPPED* lpOverlapped = NULL;
	if (GetQueuedCompletionStatus(m_hIOCP, &transferred, &CompletionKey, &lpOverlapped, INFINITE))
	{
		if (CompletionKey != 0)
		{
			IQtestmachineOverlapped* pOverlapped = CONTAINING_RECORD(lpOverlapped, IQtestmachineOverlapped, m_overlapped);
			TRACE("pOverlapped->m_operator %d\r\n", pOverlapped->m_operator);
			pOverlapped->m_serv = this;
			switch (pOverlapped->m_operator)
			{
			case IQAccept:
			{
				ACCEPTOVERLAPPED* pOver = (ACCEPTOVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(pOver->m_worker);
				break;
			}
			case IQRecv:
			{
				RECVOVERLAPPED* pOver = (RECVOVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(pOver->m_worker);
				break;
			}
			case IQSend:
			{
				SENDOVERLAPPED* pOver = (SENDOVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(pOver->m_worker);
				break;
			}
			case IQError:
			{
				ERROROVERLAPPED* pOver = (ERROROVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(pOver->m_worker);
				break;
			}
			}
		}
		else
			return -1;
	}
	return 0;
}

CIQtestmachineServer::~CIQtestmachineServer()
{
	closesocket(m_server);
	std::map<SOCKET, PCLIENT>::iterator it = m_client.begin();
	for (; it != m_client.end(); it++)
	{
		it->second.reset();
	}
	m_client.clear();
	CloseHandle(m_hIOCP);
	m_pool.Stop();
	WSACleanup();
}

bool CIQtestmachineServer::StartServer()
{
	CreateSocket();
	if (bind(m_server, (sockaddr*)&m_addr, sizeof(m_addr)) == -1)
	{
		closesocket(m_server);
		m_server = INVALID_SOCKET;
		return false;
	}
	if (listen(m_server, 3) == -1)
		return false;
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
	if (m_hIOCP = NULL)
	{
		closesocket(m_server);
		m_server = INVALID_SOCKET;
		m_hIOCP = INVALID_HANDLE_VALUE;
		return false;
	}
	CreateIoCompletionPort((HANDLE)m_server, m_hIOCP, (ULONG_PTR)this, 0);
	m_pool.Invoke();//启动线程池
	m_pool.DispatchWorker(CThreadWorker(this, (FUNCTYPE)&CIQtestmachineServer::threadIocp));//分配出线程用于处理函数
	if (!NewAccept())
		return false;
	//m_pool.DispatchWorker(CThreadWorker(this, (FUNCTYPE)&CIQtestmachineServer::threadIocp));
	//m_pool.DispatchWorker(CThreadWorker(this, (FUNCTYPE)&CIQtestmachineServer::threadIocp));
	//m_pool.DispatchWorker(CThreadWorker(this, (FUNCTYPE)&CIQtestmachineServer::threadIocp));

	return true;
}

bool CIQtestmachineServer::NewAccept()
{ 
	PCLIENT pClient(new IQtestmachineClient());
	pClient->SetOverlapped(pClient);
	m_client.insert(std::pair<SOCKET, PCLIENT>(*pClient, pClient));
	if (!AcceptEx(m_server, *pClient, *pClient, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, *pClient, *pClient))
	{
		TRACE("%d\r\n", WSAGetLastError());
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			closesocket(m_server);
			m_server = INVALID_SOCKET;
			m_hIOCP = INVALID_HANDLE_VALUE;
			return false;
		}
	}
	return true;
}

bool CIQtestmachineServer::BindNewSocket(SOCKET s)
{
	CreateIoCompletionPort((HANDLE)s, m_hIOCP, (ULONG_PTR)this, 0);
	return true;
}

void CIQtestmachineServer::CreateSocket()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	m_server = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	int opt = 1;
	setsockopt(m_server, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
}
