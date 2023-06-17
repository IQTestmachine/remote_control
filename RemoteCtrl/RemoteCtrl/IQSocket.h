#pragma once
#include <winSock2.h>
#include <memory>
#include <string>
#pragma warning(disable : 4996)

class IQSockaddrIn
{
public:
	IQSockaddrIn()
	{
		memset(&m_addr, 0, sizeof(m_addr));
		m_port = -1;
	}
	IQSockaddrIn(UINT nIP, short nPort)
	{
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(nPort);
		m_addr.sin_addr.s_addr = htonl(nIP);
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_port = nPort;
	}
	IQSockaddrIn(sockaddr_in addr)
	{
		memcpy(&m_addr, &addr, sizeof(addr));
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_port = ntohs(m_addr.sin_port);
	}
	IQSockaddrIn(const std::string& strIP, short nPort)
	{
		m_ip = strIP;
		m_port = nPort;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(nPort);
		m_addr.sin_addr.s_addr = inet_addr(strIP.c_str());
	}
	IQSockaddrIn(const IQSockaddrIn& addr)
	{
		memcpy(&m_addr, &addr.m_addr, sizeof(m_addr));
		m_ip = addr.m_ip;
		m_port = addr.m_port;
	}

	IQSockaddrIn operator=(const IQSockaddrIn& addr)
	{
		if (this != &addr)
		{
			memcpy(&m_addr, &addr.m_addr, sizeof(m_addr));
			m_ip = addr.m_ip;
			m_port = addr.m_port;
		}
		return *this;
	}

	operator sockaddr* ()
	{
		return (sockaddr*)&m_addr;
	}
	operator void* ()
	{
		return (void*)&m_addr;
	}
	std::string GetIP() const
	{
		return m_ip;
	}
	short GetPort() const
	{
		return m_port;
	}
	void updata()
	{
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_port = ntohs(m_addr.sin_port);
	}
	int size() const
	{
		return sizeof(sockaddr_in);
	}
private:
	sockaddr_in m_addr;
	std::string m_ip;
	short m_port;
};

class IQBuffer : public std::string 
{
public:
	IQBuffer(const char* str)
	{
		resize(strlen(str));
		memcpy((void*)c_str(), str, size());
	}
	IQBuffer(size_t size = 0) : std::string()
	{
		if (size > 0)
		{
			resize(size);
			memset(*this, 0, this->size());
		}
	}
	IQBuffer(void* buffer, size_t size) : std::string()
	{
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}

	~IQBuffer()
	{
		std::string::~basic_string();
	}

	operator char* () const
	{
		return (char*)c_str();
	}
	operator const char* () const
	{
		return (const char*)c_str();
	}
	operator BYTE* () const
	{
		return (BYTE*)c_str();
	}
	operator void* () const
	{
		return (void*)c_str();
	}
	void Update(void* buffer, size_t size)
	{
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
};

class IQSocket
{
public:
	enum IQTYPE {
		IQTypeTCP = 1,
		IQTypeUDP
	};
	IQSocket(IQTYPE nType = IQTypeTCP, int nProtocol = 0)
	{
		m_socket = socket(PF_INET, nType, nProtocol);
		m_type = nType;
		m_protocol = nProtocol;
	}
	IQSocket(const IQSocket& sock)
	{
		m_socket = socket(PF_INET, (int)sock.m_type, m_protocol);
		m_type = sock.m_type;
		m_protocol = sock.m_protocol;
		m_addr = sock.m_addr;
	}

	IQSocket& operator=(const IQSocket& sock)
	{
		if (this != &sock)
		{
			m_socket = socket(PF_INET, (int)sock.m_type, m_protocol);
			m_type = sock.m_type;
			m_protocol = sock.m_protocol;
			m_addr = sock.m_addr;
		}
		return *this;
	}


	~IQSocket() 
	{ 
		close();
	}

	operator SOCKET() const
	{
		return m_socket;
	}
	operator SOCKET()
	{
		return m_socket;
	}
	
	bool operator==(SOCKET sock) const
	{
		return m_socket == sock;
	}

	int IQlisten(int backlog = 5)
	{
		if (m_type != IQTYPE::IQTypeTCP)
			return -1;
		return ::listen(m_socket, backlog);
	}
	int IQbind(const std::string& ip, short port)
	{
		m_addr = IQSockaddrIn(ip, port);
		return ::bind(m_socket, m_addr, m_addr.size());
	}
	int IQaccept() { }
	int IQconnect(const std::string& ip, short port) { }
	int IQsend(const IQBuffer& buffer)
	{
		return ::send(m_socket, buffer, buffer.size(), 0);
	}
	int IQrecv(IQBuffer& buffer)
	{
		return ::recv(m_socket, buffer, buffer.size(), 0);
	}
	int IQsendto(IQSockaddrIn& to, const IQBuffer& buffer)
	{
		return ::sendto(m_socket, buffer, buffer.size(), 0, to, to.size());
	}
	int IQrecvfrom(IQSockaddrIn& from, IQBuffer& buffer)
	{ 
		int len = from.size();
		int ret =  ::recvfrom(m_socket, buffer, buffer.size(), 0, from, &len);
		if (ret > 0)
		{
			from.updata();//注意更新IQSockaddrIn对象client的成员m_ip与m_port, 因为之前的recvfrom只得到了m_addr
		}
		return ret;
	}
	void close()
	{
		if (m_socket != INVALID_SOCKET)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
	}
private:
	SOCKET m_socket;
	IQTYPE m_type;
	int m_protocol;
	IQSockaddrIn m_addr;
};
typedef std::shared_ptr<IQSocket> IQSOCKET;

