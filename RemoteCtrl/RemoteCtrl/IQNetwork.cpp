#include "pch.h"
#include "IQNetwork.h"

IQServer::IQServer(const IQServerParameter& param) : m_stop(false), m_args(NULL)
{
	m_params = param;
	m_thread.Start();
	m_thread.UpdataWorker(CThreadWorker(this, (FUNCTYPE)&IQServer::threadFunc));
}

IQServer::~IQServer()
{
	Stop();
}

int IQServer::Invoke(void* arg)
{
    m_sock.reset(new IQSocket(m_params.m_type));
    if (*m_sock == INVALID_SOCKET)
    {
        printf("%s(%d): %s ERROR(%d)!\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError());
        return -1;
    }
    if (m_params.m_type == IQSocket::IQTypeTCP)
    {
        if (m_sock->IQlisten() == -1)
            return -2;
    }
    IQSockaddrIn client;
    if (m_sock->IQbind(m_params.m_ip, m_params.m_port) == -1)
    {
        printf("%s(%d): %s ERROR!\r\n", __FILE__, __LINE__, __FUNCTION__);
        return -3;
    }
    if (m_thread.Start() == false)
        return -4;
	m_args = arg;
    return 0;
}

int IQServer::threadFunc()
{
	if (m_params.m_type == IQSocket::IQTypeTCP)
		return threadTCPFunc();
	else
		return threadUDPFunc();
}

int IQServer::threadUDPFunc()
{
	IQBuffer buf(1024 * 256);
	IQSockaddrIn client;
	int ret = 0;
	while (!m_stop)
	{
		int ret = m_sock->IQrecvfrom(client, buf);
		printf("%s(%d): %s ret = %d\r\n", __FILE__, __LINE__, __FUNCTION__, ret);
		if (ret > 0)
		{
			client.updata();
			if (m_params.m_recvfrom != NULL)
			{
				m_params.m_recvfrom(m_args, buf, client);
			}
		}
		else
		{
			printf("%s(%d): %s ERROR(%d)! ret = %d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
			break;
		}
	}
	if (m_stop == false)
		m_stop = true;
	m_sock->close();
	printf("%s(%d): %s\r\n", __FILE__, __LINE__, __FUNCTION__);
	return 0;
}

int IQServer::threadTCPFunc()
{
	return 0;
}

int IQServer::Stop()
{
	if (m_stop == false)
	{
		m_sock->close();
		m_stop = true;
		m_thread.Stop();
	}
	return 0;
}

int IQServer::Send(IQSOCKET& client, const IQBuffer& buffer)
{
	int ret = m_sock->IQsend(buffer);//TODO: 待优化, 发送虽然成功, 但buffer可能还存在数据
	if (m_params.m_send)
		m_params.m_send(m_args, client, ret);//? 此处的功能尚不清楚
	return ret;
}

int IQServer::Sendto(IQSockaddrIn& addr, const IQBuffer& buffer)
{
	int ret = m_sock->IQsendto(addr, buffer);
	if (m_params.m_send)
		m_params.m_sendto(m_args, addr, ret);
	return ret;
}

IQServerParameter::IQServerParameter(const std::string& ip, short port, IQSocket::IQTYPE type, AcceptFunc acceptf, RecvFunc recvf, SendFunc sendf, RecvFromFunc recvfromf, SendToFunc sendtof)
{
	m_ip = ip;
	m_port = port;
	m_type = type;
	m_accept = acceptf;
	m_recv = recvf;
	m_send = sendf;
	m_recvfrom = recvfromf;
	m_sendto = sendtof;
}

IQServerParameter::IQServerParameter(const IQServerParameter& param)
{
	m_ip = param.m_ip;
	m_port = param.m_port;
	m_type = param.m_type;
	m_accept = param.m_accept;
	m_recv = param.m_recv;
	m_send = param.m_send;
	m_recvfrom = param.m_recvfrom;
	m_sendto = param.m_sendto;
}

IQServerParameter& IQServerParameter::operator=(const IQServerParameter& param)
{
	if (this != &param)
	{
		m_ip = param.m_ip;
		m_port = param.m_port;
		m_type = param.m_type;
		m_accept = param.m_accept;
		m_recv = param.m_recv;
		m_send = param.m_send;
		m_recvfrom = param.m_recvfrom;
		m_sendto = param.m_sendto;
	}
	return *this;
}

IQServerParameter& IQServerParameter::operator<<(AcceptFunc func)
{
	m_accept = func;
	return *this;
}

IQServerParameter& IQServerParameter::operator<<(RecvFunc func)
{
	m_recv = func;
	return *this;
}

IQServerParameter& IQServerParameter::operator<<(SendFunc func)
{
	m_send = func;
	return *this;
}

IQServerParameter& IQServerParameter::operator<<(RecvFromFunc func)
{
	m_recvfrom = func;
	return *this;
}

IQServerParameter& IQServerParameter::operator<<(SendToFunc func)
{
	m_sendto = func;
	return *this;
}

IQServerParameter& IQServerParameter::operator<<(std::string& ip)
{
	m_ip = ip;
	return *this;
}

IQServerParameter& IQServerParameter::operator<<(short port)
{
	m_port = port;
	return *this;
}

IQServerParameter& IQServerParameter::operator<<(IQSocket::IQTYPE type)
{
	m_type = type;
	return *this;
}

IQServerParameter& IQServerParameter::operator>>(AcceptFunc& func)
{
	func = m_accept;
	return *this;
}

IQServerParameter& IQServerParameter::operator>>(RecvFunc& func)
{
	func = m_recv;
	return *this;
}

IQServerParameter& IQServerParameter::operator>>(SendFunc& func)
{
	func = m_send;
	return *this;
}

IQServerParameter& IQServerParameter::operator>>(RecvFromFunc& func)
{
	func = m_recvfrom;
	return *this;
}

IQServerParameter& IQServerParameter::operator>>(SendToFunc& func)
{
	func = m_sendto;
	return *this;
}

IQServerParameter& IQServerParameter::operator>>(std::string& ip)
{
	ip = m_ip;
	return *this;
}

IQServerParameter& IQServerParameter::operator>>(short& port)
{
	port = m_port;
	return *this;
}

IQServerParameter& IQServerParameter::operator>>(IQSocket::IQTYPE& type)
{
	type = m_type;
	return *this;
}