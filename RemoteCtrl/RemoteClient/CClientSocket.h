#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#pragma warning(disable : 4996)

#pragma pack(push)
#pragma pack(1)

class CPacket
{
public:
	CPacket() : sHead(0), nLength(0), sCmd(0), sSum(0) { }
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize)//����˰�Ҫ���͵���Ϣ�����һ�����ݰ�
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
	CPacket(const BYTE* pData, size_t& nSize)//�ӿͻ��˶�ȡ�����������д����һ�����ݰ�
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
	WORD sHead;//��ͷ, �̶�λFE FF
	DWORD nLength;//������(�ӿ������ʼ,��У��ͽ���)
	WORD sCmd;//��������
	std::string strData;//������
	WORD sSum;//��У��
	std::string strOut;// �������ݰ�
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

	bool InitSocket(const std::string& strIPAddress)
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
		serv_adr.sin_addr.s_addr = inet_addr(strIPAddress.c_str());
		serv_adr.sin_port = htons(9527);
		if (serv_adr.sin_addr.s_addr == INADDR_NONE)
		{
			AfxMessageBox("ָ����IP��ַ������!");
			return false;
		}
		int ret = connect(m_client, (sockaddr*)&serv_adr, sizeof(serv_adr));
		if (ret == -1)
		{
			AfxMessageBox("����ʧ��");
			TRACE("����ʧ��: %d %s\r\n", WSAGetLastError(), GetErrorInfo(WSAGetLastError()).c_str());
			return false;
		}
		return true;
	}

#define BUFFER_SIZE 4096
	int DealCommand()
	{
		if (m_client == -1)
			return -1;
		//char buffer[1024] = "";
		char* buffer = m_buffer.data();
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

	CPacket GetPacket()
	{
		return m_packet;
	}
	void CloseSocket()
	{
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}

private:
	std::vector<char> m_buffer;
	SOCKET m_client;
	CPacket m_packet;
	CClientSocket()
	{
		if (InitSockEnv() == false)
		{
			MessageBox(nullptr, _T("�޷���ʼ���׽��ֻ���, ������������!"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_buffer.resize(BUFFER_SIZE);
	}
	CClientSocket(const CClientSocket&) { }
	CClientSocket& operator=(const CClientSocket& ss)
	{
		m_client = ss.m_client;
	}

	~CClientSocket()
	{
		closesocket(m_client);
		WSACleanup();
	}

	bool InitSockEnv()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data)) // TODO: ����ֵ����
		{
			return false;
		}
		return true;
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



