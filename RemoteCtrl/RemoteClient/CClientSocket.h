#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#include <list>
#include <map>
#include <mutex>
#pragma warning(disable : 4996)

#define WM_SEND_PACK (WM_USER + 1) //发送命令数据包
#define WM_SEND_PACK_ACK (WM_USER + 100) //命令应答包

#pragma pack(push)
#pragma pack(1)
class CPacket
{
public:
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) { }
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize)//控制端把要发送的信息打包成一个数据包
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
	}
	CPacket(const CPacket& pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize)//从服务端读取来的数据流中打包出一个数据包
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

enum { CM_AUTOCLOSED = 1};//是否自动关闭

typedef struct PackData
{
	std::string strData;
	UINT nMode;
	WPARAM wParam;
	PackData(const char* pData, size_t nLen, UINT mode, WPARAM nwParam = 0)
	{
		strData.resize(nLen);
		memcpy((char*)strData.c_str(), pData, nLen);
		nMode = mode;
		wParam = nwParam;
 	}
	PackData(const PackData& data)
	{
		strData = data.strData;
		nMode = data.nMode;
		wParam = data.wParam;
	}
	PackData& operator=(const PackData& data)
	{
		if (this != &data)
		{
			strData = data.strData;
			nMode = data.nMode;
			wParam = data.wParam;
		}
		return *this;
	}
}PACKET_DATA;

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
			TRACE("连接失败: %d %s\r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
			AfxMessageBox("连接失败");
			
			return false;
		}
		return true;
	}

#define BUFFER_SIZE 8192000

	bool SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed, WPARAM wParam = 0);

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
	HANDLE m_eventInvoke;
	UINT m_nThreadID;
	typedef void(CClientSocket::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	std::map<UINT, MSGFUNC> m_mapFunc;//消息函数指针映射表
	HANDLE m_hThread;
	bool m_bAutoClosed;
	int m_nIP;
	int m_nPort;
	SOCKET m_client;
	CClientSocket() : m_nIP(INADDR_ANY), m_nPort(9527), m_client(INVALID_SOCKET), m_bAutoClosed(true), m_hThread(INVALID_HANDLE_VALUE)
	{
		if (InitSockEnv() == false)
		{
			MessageBox(nullptr, _T("无法初始化套接字环境, 请检查网络设置!"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_eventInvoke = CreateEvent(nullptr, true, false, nullptr);
		m_hThread = (HANDLE)_beginthreadex(nullptr, 0, &CClientSocket::threadEntry, this, 0, &m_nThreadID);
		if (WaitForSingleObject(m_eventInvoke, 100) == WAIT_TIMEOUT)
			TRACE("网络消息处理线程启动失败!\r\n");
		CloseHandle(m_eventInvoke);
		struct
		{
			UINT message;
			MSGFUNC func;
		}MsgFunc[] =
		{
			{WM_SEND_PACK, &CClientSocket::SendPack},
			{0, nullptr}
		};
		for (int i = 0; MsgFunc[i].message != 0; i++)
		{
			if (m_mapFunc.insert(std::pair<UINT, MSGFUNC>(MsgFunc[i].message, MsgFunc[i].func)).second == false)
				TRACE("插入失败, 消息值: %d, 函数指: %08X, 序号: %d\r\n", MsgFunc[i].message, MsgFunc[i].func, i);
		}
	}
	CClientSocket(const CClientSocket& ss)
	{ 
		m_hThread = INVALID_HANDLE_VALUE;
		m_bAutoClosed = ss.m_bAutoClosed;
		m_client = ss.m_client;
		m_nIP = ss.m_nIP;
		m_nPort = ss.m_nPort;
		m_eventInvoke = ss.m_eventInvoke;
	}
	CClientSocket& operator=(const CClientSocket& ss) { }

	~CClientSocket()
	{
		closesocket(m_client);
		m_client = INVALID_SOCKET;
		WSACleanup();
	}


	static unsigned __stdcall threadEntry(void* arg);//该线程在客户端套接字创建时即启动, 专门用于处理客户端UI界面发送的各种消息
	void threadFunc2();

	bool InitSockEnv()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data)) // TODO: 返回值处理
		{
			return false;
		}
		return true;
	}

	void SendPack(UINT msg, WPARAM wParam, LPARAM lParam);

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



