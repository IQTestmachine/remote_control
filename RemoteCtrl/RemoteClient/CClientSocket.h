#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#include <list>
#include <map>
#include <mutex>
#pragma warning(disable : 4996)

#define WM_SEND_PACK (WM_USER + 1) //�����������ݰ�
#define WM_SEND_PACK_ACK (WM_USER + 100) //����Ӧ���

#pragma pack(push)
#pragma pack(1)
class CPacket
{
public:
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) { }
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize)//���ƶ˰�Ҫ���͵���Ϣ�����һ�����ݰ�
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
	CPacket(const BYTE* pData, size_t& nSize)//�ӷ���˶�ȡ�����������д����һ�����ݰ�
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
		if (i + 4 + 2 + 2 > nSize)// ���ݰ����ܲ�ȫ, ���ͷδ��ȫ�����յ� 
		{
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);
		i += 4;
		if (nLength + i > nSize)// δ����ȫ���յ����ݰ�, �򷵻�, ����ʧ��
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

	int Size()//���ݰ��Ĵ�С
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
	WORD sHead;//��ͷ, �̶�λFE FE
	DWORD nLength;//������(�ӿ������ʼ,��У��ͽ���)
	WORD sCmd;//��������
	std::string strData;//������
	WORD sSum;//��У��
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
	WORD nAction;//���, �ƶ�, ˫��
	WORD nButton;//���, �Ҽ�, �м�
	POINT ptXY;//����
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
	bool IsInvalid;//·���Ƿ���Ч
	bool IsDirectory;//�Ƿ�ΪĿ¼ 0:�� 1:��
	bool HasNext;//�Ƿ��к��� 0:�� 1:��
	char szFileName[256];//�ļ���
}FILEINFO, * PFILEINFO;

enum { CM_AUTOCLOSED = 1};//�Ƿ��Զ��ر�

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
		// TODO: У��, �׽����Ƿ񴴽��ɹ�
		if (m_client == -1)
			return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = htonl(m_nIP);//ע�⽫�����ֽ���ת��Ϊ�����ֽ���
		serv_adr.sin_port = htons(m_nPort);
		if (serv_adr.sin_addr.s_addr == INADDR_NONE)
		{
			AfxMessageBox("ָ����IP��ַ������!");
			return false;
		}
		int ret = connect(m_client, (sockaddr*)&serv_adr, sizeof(serv_adr));
		if (ret == -1)
		{
			TRACE("����ʧ��: %d %s\r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
			AfxMessageBox("����ʧ��");
			
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
	std::map<UINT, MSGFUNC> m_mapFunc;//��Ϣ����ָ��ӳ���
	HANDLE m_hThread;
	bool m_bAutoClosed;
	int m_nIP;
	int m_nPort;
	SOCKET m_client;
	CClientSocket() : m_nIP(INADDR_ANY), m_nPort(9527), m_client(INVALID_SOCKET), m_bAutoClosed(true), m_hThread(INVALID_HANDLE_VALUE)
	{
		if (InitSockEnv() == false)
		{
			MessageBox(nullptr, _T("�޷���ʼ���׽��ֻ���, ������������!"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_eventInvoke = CreateEvent(nullptr, true, false, nullptr);
		m_hThread = (HANDLE)_beginthreadex(nullptr, 0, &CClientSocket::threadEntry, this, 0, &m_nThreadID);
		if (WaitForSingleObject(m_eventInvoke, 100) == WAIT_TIMEOUT)
			TRACE("������Ϣ�����߳�����ʧ��!\r\n");
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
				TRACE("����ʧ��, ��Ϣֵ: %d, ����ָ: %08X, ���: %d\r\n", MsgFunc[i].message, MsgFunc[i].func, i);
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


	static unsigned __stdcall threadEntry(void* arg);//���߳��ڿͻ����׽��ִ���ʱ������, ר�����ڴ���ͻ���UI���淢�͵ĸ�����Ϣ
	void threadFunc2();

	bool InitSockEnv()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data)) // TODO: ����ֵ����
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



