#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#include <list>
#include <map>
#pragma warning(disable : 4996)

#pragma pack(push)
#pragma pack(1)
class CPacket
{
public:
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) { }
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize, HANDLE nEvent)//控制端把要发送的信息打包成一个数据包
	{
		sHead = 0xFEFE;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0)
		{
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else
		{
			strData.clear();
		}
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
		hEvent = nEvent;
	}
	CPacket(const CPacket& pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
		hEvent = pack.hEvent;
	}
	CPacket(const BYTE* pData, size_t& nSize) : hEvent(INVALID_HANDLE_VALUE)//从服务端读取来的数据流中打包出一个数据包
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
			return;
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
		hEvent = pack.hEvent;
		return *this;
	}

	int Size()//数据包的大小
	{
		return nLength + 6;
	}
	const char* Data(std::string& strOut) const
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

	~CPacket() { };
public:
	WORD sHead;//包头, 固定位FE FE
	DWORD nLength;//包长度(从控制命令开始,到校验和结束)
	WORD sCmd;//控制命令
	std::string strData;//包数据
	WORD sSum;//和校验
	HANDLE hEvent;
};
#pragma pack(pop)

typedef struct MouseEvent
{
	MouseEvent()
	{
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//点击, 移动, 双击
	WORD nButton;//左键, 右键, 中键
	POINT ptXY;//坐标
}MOUSEEV, * PMOUSEEV;

typedef struct file_info
{
	file_info()
	{
		IsInvalid = false;
		IsDirectory = -1;
		bool HasNext = true;
		memset(szFileName, 0, sizeof(szFileName));
	}
	bool IsInvalid;//路径是否有效
	bool IsDirectory;//是否为目录 0:否 1:是
	bool HasNext;//是否还有后续 0:否 1:是
	char szFileName[256];//文件名
}FILEINFO, * PFILEINFO;


std::string GetErrorInfo(int wsaErrCode);

class CClientSocket
{
public:
	static CClientSocket* getInstance()
	{
		if (m_instance == nullptr)
		{
			m_instance = new CClientSocket();
		}
		return m_instance;
	}

	bool InitSocket()
	{
		if (m_client != INVALID_SOCKET)
			CloseSocket();
		m_client = socket(PF_INET, SOCK_STREAM, 0);
		TRACE("%d\r\n", m_client);
		// TODO: 校验, 套接字是否创建成功
		if (m_client == -1)
			return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = htonl(m_nIP);//注意将本地字节序转换为网络字节序
		serv_adr.sin_port = htons(m_nPort);
		if (serv_adr.sin_addr.s_addr == INADDR_NONE)
		{
			AfxMessageBox("指定的IP地址不存在!");
			return false;
		}
		int ret = connect(m_client, (sockaddr*)&serv_adr, sizeof(serv_adr));
		if (ret == -1)
		{
			AfxMessageBox("连接失败");
			TRACE("连接失败: %d %s\r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
			return false;
		}
		return true;
	}

#define BUFFER_SIZE 2048000
	//客户端的DealCommand()与服务端的DealCommand()不同, 
	//通常服务端的DealCommand只是获取客户端的操作命令, 服务端在关闭m_client套接字之前不会再调用该函数, 即服务端每次连接仅执行一条命令
	//执行客户端的命令服务端可能发送大量数据包, 因此客户端需要多次调用DealCommand, 
	//客户端会每次调用DealCommand只获取一个数据包, 然而TCP连接导致DealCommand里调用recv()函数可能接收到了大量数据, 
	//这些数据不存在数据边界, 可能包括几个数据包和不完整的数据包, 
	//因此增添了新的数据成员(接收缓冲区)m_buffer来使得调用DealCommand接收一个数据包之后剩余数据依旧存在
	int DealCommand()//接收一个数据包
	{
		if (m_client == -1)
		{
			TRACE("客户端连接关闭");
			return -1;
		}	
		//char buffer[1024] = "";
		char* buffer = m_buffer.data();
		static size_t index = 0;//index表示m_buffer中有多少个字节, 因此每次调用recv()和CPacket(const BYTE* pData, size_t& nSize)都需调整index
		while (true)
		{
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0 && index <= 0)
			{
				//TRACE("接收数据有问题, len = %d, index = %d\r\n", len, index);
				return -1;
			}
			index += len;
			//TRACE("buffer + index = %x, len = %d\r\n", buffer + index, len);
			size_t tmp = index;
			m_packet = CPacket((BYTE*)buffer, tmp);//tmp表示从m_buffer中取出的数据包有多少个字节, 如果tmp等于0, 则代表并未取出任何数据
			//TRACE("解包的长度是%lld %d\r\n", *(long long*)m_packet.strData.c_str(), m_packet.nLength);
			if (tmp > 0)//采用TCP连接, 不一定能从m_buffer中出一个数据包, 此时就要继续执行循环, 去recv()数据
			{
				memmove(buffer, buffer + tmp, index - tmp);//由于取出了一个数据包, 因此需要调整m_buffer
				index -= tmp;
				return m_packet.sCmd;
			}
		}

		return -1;
	}

	bool SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks)
	{
		if (m_client == INVALID_SOCKET)
		{
			if (InitSocket() == false)
				return false;
			_beginthread(&CClientSocket::threadEntry, 0, this);
		}
		m_lstSend.push_back(pack);
		WaitForSingleObject(pack.hEvent, INFINITE);
		std::map<HANDLE, std::list<CPacket>>::iterator it;
		it = m_mapAck.find(pack.hEvent);
		if (it != m_mapAck.end())
		{
			std::list<CPacket>::iterator i;
			for (i = it->second.begin(); i != it->second.end(); i++)
			{
				lstPacks.push_back(*i);
			}
			m_mapAck.erase(it);
			return true;
		}
		return false;
	}

	bool GetFilePath(std::string& strPath)
	{
		if (m_packet.sCmd >= 2 && m_packet.sCmd <= 4)
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
	void CloseSocket()
	{
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}

	void UpdateAddress(int nIP, int nPort)
	{
		if (m_nIP != nIP || m_nPort != nPort)
		{
			m_nIP = nIP;
			m_nPort = nPort;
		} 
	}

private:
	std::list<CPacket> m_lstSend;
	std::map<HANDLE, std::list<CPacket>> m_mapAck;
	int m_nIP;
	int m_nPort;
	std::vector<char> m_buffer;//与服务端相比新增的成员变量(接收缓冲区), 详见208~213行注释
	SOCKET m_client;
	CPacket m_packet;
	CClientSocket() : m_nIP(INADDR_ANY), m_nPort(0), m_client(INVALID_SOCKET)
	{
		if (InitSockEnv() == false)
		{
			MessageBox(nullptr, _T("无法初始化套接字环境, 请检查网络设置!"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_buffer.resize(BUFFER_SIZE);
		memset(m_buffer.data(), 0, BUFFER_SIZE);
	}
	CClientSocket(const CClientSocket& ss)
	{ 
		m_client = ss.m_client;
		m_nIP = ss.m_nIP;
		m_nPort = ss.m_nPort;
	}
	CClientSocket& operator=(const CClientSocket& ss) { }

	~CClientSocket()
	{
		closesocket(m_client);
		m_client = INVALID_SOCKET;
		WSACleanup();
	}

	static void threadEntry(void* arg);
	void threadFunc();

	bool InitSockEnv()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data)) // TODO: 返回值处理
		{
			return false;
		}
		return true;
	}

	bool Send(const char* pData, int nSize)
	{
		if (m_client == -1)
			return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
	bool Send(const CPacket& pack)
	{
		if (m_client == -1)
			return false;
		std::string strOut;
		pack.Data(strOut);
		return send(m_client, strOut.c_str(), strOut.size(), 0) > 0;
	}

	static CClientSocket* m_instance;

	static void relaseInstance()
	{
		if (m_instance != nullptr)
		{
			CClientSocket* tmp = m_instance;
			m_instance = nullptr;
			delete tmp;
		}
	}

	class Helper
	{
	public:
		Helper()
		{
			CClientSocket::getInstance();
		}
		~Helper()
		{
			CClientSocket::relaseInstance();
		}
	};

	static Helper m_helper;
};

//extern CServerSocket server;



