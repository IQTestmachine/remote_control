#pragma once
#include "pch.h"
#include "framework.h"
#include "Packet.h"
#include "list"
void Dump(BYTE* pData, size_t nSize);

typedef void (*SOCKET_CALLBACK)(void*, int, std::list<CPacket>&, CPacket& inPacket);

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

	int Run(SOCKET_CALLBACK callback, void* arg, short port = 9527)
	{
		// TODO: socket, blind, listen, accept, read, close
		// 套接字初始化
		std::list<CPacket> lstPackets;
		bool ret = InitSocket(port);
		if (ret == false)
			return -1;
		int count = 0;
		while (true)
		{
			if (AcceptClient() == false)
			{
				if (count >= 3)
					return -2;
				count++;
				//continue;
			}
			int ret = DealCommand();
			if (ret > 0)
			{
				m_callback(m_arg, ret, lstPackets, m_packet);//参数m_arg可能应该换成arg;
				while (lstPackets.size() > 0)
				{
					Send(lstPackets.front());
					lstPackets.pop_front();
				}
			}
			CloseClient();
		}
		m_callback = callback;
		m_arg = arg;
		return 0;
	}

protected:
	bool InitSocket(short port)
	{
		// TODO: 校验, 套接字是否创建成功
		if (m_server == -1)
			return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(port);
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
		//TRACE("m_client = %d\r\n", m_client);
		if (m_client == -1)
			return false;
		return true;
	}

#define BUFFER_SIZE 4096
	int DealCommand()//接收数据包
	{
		if (m_client == -1)
			return -1;
		//char buffer[1024] = "";
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == nullptr)
		{
			TRACE("内存不足!");
			return -2;
		}
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true)
		{
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0)
			{
				delete[] buffer;
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0)
			{
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				delete[] buffer;
				//TRACE("服务端收到的命令号是%d\r\n", m_packet.sCmd);
				return m_packet.sCmd;
			}		
		}
		delete[] buffer;//? 没必要了啊, 已经确保释放内存了啊
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
		//Dump((BYTE*)pack.Data(), pack.Size());
		return send(m_client, pack.Data(), pack.Size(), 0) > 0;
	}

	bool GetFilePath(std::string& strPath)
	{
		if ((m_packet.sCmd >= 2 && m_packet.sCmd <= 4) || m_packet.sCmd == 9)
		{
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}

	bool GetMouseEvent(MOUSEEV& mouse)
	{
		if (m_packet.sCmd == 5)
		{
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}

	CPacket& GetPacket()
	{
		return m_packet;
	}

	void CloseClient()
	{
		if (m_client != INVALID_SOCKET)
		{
			closesocket(m_client);
			m_client = INVALID_SOCKET;
		}
	}
private:
	SOCKET_CALLBACK m_callback;
	void* m_arg;
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

