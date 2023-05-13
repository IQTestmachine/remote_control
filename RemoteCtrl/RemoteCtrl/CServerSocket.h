#pragma once
#include "pch.h"
#include "framework.h"

#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) { }
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize)
	{
		sHead = 0xFEFE;
		nLength = nSize + 4;
		sCmd = nCmd;
		strData.resize(nSize);
		memcpy((void*)strData.c_str(), pData, nSize);
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}
	CPacket(const CPacket& pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize)
	{
		size_t i = 0;
		for (; i < nSize; i++)
		{
			if (*(WORD*)(pData + i) == 0xFEFE)
			{
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize)// 数据包可能不全, 或包头未能全部接收到 
		{
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);
		i += 4;
		if (nLength + i > nSize)// 未能完全接收到数据包, 则返回, 解析失败
		{
			nSize = 0;
		}
		sCmd = *(WORD*)(pData + i);
		i += 2;
		if (nLength > 4)
		{
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i);
		i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum)
		{
			nSize = i;
			return;
		}
		else
		{
			nSize = 0;
		}
	}

	CPacket& operator=(const CPacket& pack)
	{
		if (this == &pack)
			return *this;
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
		return *this;
	}

	int Size()//数据包的大小
	{
		return nLength + 6;
	}
	const char* Data()
	{
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;
		pData += 2;
		*(DWORD*)pData = nLength;
		pData += 4;
		*(WORD*)pData = sCmd;
		pData += 2;
		memcpy(pData, strData.c_str(), strData.size());
		pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

	~CPacket() { }
public:
	WORD sHead;//包头, 固定位FE FF
	DWORD nLength;//包长度(从控制命令开始,到校验和结束)
	WORD sCmd;//控制命令
	std::string strData;//包数据
	WORD sSum;//和校验
	std::string strOut;// 整个数据包
};

#pragma pack(pop)

class CServerSocket
{
public:
	static CServerSocket* getInstance()
	{
		if (m_instance == nullptr)
		{
			m_instance = new CServerSocket();
		}
		return m_instance;
	}

	bool InitSocket()
	{
		// TODO: 校验, 套接字是否创建成功
		if (m_server == -1)
			return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);
		// 绑定IP地址与端口号
		if (bind(m_server, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) // TODO: 返回值处理
			return false;
		if (listen(m_server, 1) == -1)
			return false;
		return true;
		
	}

	bool AcceptClient()
	{
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_server, (sockaddr*)&client_adr, &cli_sz);
		if (m_client == -1)
			return false;
		return true;
	}

#define BUFFER_SIZE 4096
	int DealComment()
	{
		if (m_client == -1)
			return -1;
		//char buffer[1024] = "";
		char* buffer = new char[4096];
		memset(buffer, 0, 4096);
		size_t index = 0;
		while (true)
		{
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0)
				return -1;
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;
			}		
		}
		return -1;
	}

	bool Send(const char* pData, int nSize)
	{
		if (m_client == -1)
			return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack)
	{
		if (m_client == -1)
			return false;
		return send(m_client, pack.Data(), pack.Size(), 0) > 0;
	}

private:
	SOCKET m_server;
	SOCKET m_client;
	CPacket m_packet;
	CServerSocket()
	{
		m_client = INVALID_SOCKET;
		if (InitSockEnv() == false)
		{
			MessageBox(nullptr, _T("无法初始化套接字环境, 请检查网络设置!"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_server = socket(PF_INET, SOCK_STREAM, 0);

	}
	CServerSocket(const CServerSocket&) { }
	CServerSocket& operator=(const CServerSocket& ss) 
	{
		m_server = ss.m_server;
		m_client = ss.m_client;
	}

	~CServerSocket() 
	{
		closesocket(m_server);
		WSACleanup();
	}

	bool InitSockEnv()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data)) // TODO: 返回值处理
		{
			return false;
		}
		return true;
	}

	static CServerSocket* m_instance;

	static void relaseInstance()
	{
		if (m_instance != nullptr)
		{
			CServerSocket* tmp = m_instance;
			m_instance = nullptr;
			delete tmp;
		}
	}

	class Helper
	{
	public:
		Helper()
		{
			CServerSocket::getInstance();
		}
		~Helper()
		{
			CServerSocket::relaseInstance();
		}
	};

	static Helper m_helper;
};

//extern CServerSocket server;

