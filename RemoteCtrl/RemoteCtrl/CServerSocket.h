#pragma once
#include "pch.h"
#include "framework.h"
void Dump(BYTE* pData, size_t nSize);

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
}MOUSEEV, *PMOUSEEV;

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
		// TODO: У��, �׽����Ƿ񴴽��ɹ�
		if (m_server == -1)
			return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);
		// ��IP��ַ��˿ں�
		if (bind(m_server, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) // TODO: ����ֵ����
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
		TRACE("m_client = %d\r\n", m_client);
		if (m_client == -1)
			return false;
		return true;
	}

#define BUFFER_SIZE 4096
	int DealCommand()//�������ݰ�
	{
		if (m_client == -1)
			return -1;
		//char buffer[1024] = "";
		char* buffer = new char[4096];
		if (buffer == nullptr)
		{
			TRACE("�ڴ治��!");
			return -2;
		}
		memset(buffer, 0, 4096);
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
				return m_packet.sCmd;
			}		
		}
		delete[] buffer;//? û��Ҫ�˰�, �Ѿ�ȷ���ͷ��ڴ��˰�
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
		Dump((BYTE*)pack.Data(), pack.Size());
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
		closesocket(m_client);
		m_client = INVALID_SOCKET;
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
			MessageBox(nullptr, _T("�޷���ʼ���׽��ֻ���, ������������!"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
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
		if (WSAStartup(MAKEWORD(1, 1), &data)) // TODO: ����ֵ����
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

