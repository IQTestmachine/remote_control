#pragma once
#include "CClientSocket.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include <map>
#include "Resource.h"
#include "CClientSocket.h"
#include "IQtestmachineTool.h"

//#define WM_SEND_PACK (WM_USER + 1) //���Ͱ�����
//#define WM_SEND_PACK (WM_USER + 2) //��������
#define WM_SHOW_STATUS (WM_USER + 3) //չʾ״̬
#define WM_SHOW_STATUS (WM_USER + 3) //Զ�̼��
#define WM_SEND_MESSAGE (WM_USER + 0x10000)//�Զ�����Ϣ����

class CCommandCtrl
{
public:
	//��ȡȫ��Ψһ����
	static CCommandCtrl* getInstance();
	//��ʼ������
	int InitController();
	//����
	int Invoke(CWnd*& pMainWnd);
	//������Ϣ
	LRESULT SendMessage(MSG msg);
	//���������������ַ
	void UpdateAddress(int nIP, int nPort)
	{
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}

	/*int DealCommand()
	{
		return CClientSocket::getInstance()->DealCommand();
	}*/

	void CloseSocket()
	{
		CClientSocket::getInstance()->CloseSocket();
	}

	/*bool SendPacket(const CPacket& pack)
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		if (pClient->InitSocket() == false)
			return false;
		pClient->SendPacket(pack, );
		return true;
	}*/

	//1.�鿴���̷���
	//2.�鿴ָ��Ŀ¼�µ��ļ�
	//3.���ļ�
	//4.�����ļ�
	//5.������
	//6.�����÷������Ļ��ͼ
	//7.����
	//8.����
	//9.ɾ���ļ�
	//1981. �������� 
	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = nullptr, size_t nLength = 0, std::list<CPacket>* plstPacks = nullptr)
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		/*if (pClient->InitSocket() == false)
			return false;*/


		/////////////
		std::list<CPacket> lstPacks;
		if (plstPacks == nullptr)
			plstPacks = &lstPacks;
		HANDLE hEvent = CreateEvent(nullptr, true, false, nullptr);
		pClient->SendPacket(CPacket(nCmd, pData, nLength, hEvent), *plstPacks);
		CloseHandle(hEvent);//�����¼����, ��ֹ��Դ�ľ�
		if (plstPacks->size() > 0)
			return plstPacks->front().sCmd;
		return -1;
	}

	int GetImage(CImage& image)
	{
		CClientSocket* pClient = CClientSocket::getInstance();
		return CIQtestmachineTool::Bytes2Image(image, pClient->GetPacket().strData);
	}

	int DownFile(CString strPath);
	int StartWatchScreen();
protected:
	static void threadEntryForWatchData(void* arg);//��̬��������ʹ��thisָ��, ����������º�������
	void threadWatchData();
	static void threadEntryForDownFile(void* arg);
	void threadDownFile();
	CCommandCtrl() : m_statusDlg(&m_remoteDlg), m_watchDlg(&m_remoteDlg)
	{ 
		m_hTread = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
		m_hThreadDownload = INVALID_HANDLE_VALUE;
		m_hThreadWatch = INVALID_HANDLE_VALUE;

	}

	~CCommandCtrl()
	{
		WaitForSingleObject(m_hTread, 100);
	}

	void threadFunc();
	static unsigned __stdcall threadEntry(void* arg);

	static void relaseInstance()
	{
		if (m_instance != nullptr)
		{
			/*CCommandCtrl* tmp = m_instance;
			m_instance = nullptr;
			delete tmp;*/
			delete m_instance;
			m_instance = nullptr;
		}
	}

	/*LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);*/
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShoWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);
private:
	typedef struct MsgInfo
	{
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m)
		{
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m)
		{
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}

		MsgInfo& operator=(const MsgInfo& m)
		{
			if (this != &m)
			{
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			return *this;
		}

	}MSGINFO;
	typedef LRESULT (CCommandCtrl::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC> m_mapFunc;//��Ϣ����ָ��ӳ���
	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hTread;
	HANDLE m_hThreadDownload;
	//�����ļ���Զ��·��
	CString m_strRemote;
	//�����ļ��ı���·��
	CString m_strLocal;
	HANDLE m_hThreadWatch;
	//bool m_isClosed;
	unsigned m_nThreadID;
	static CCommandCtrl* m_instance;
public:
	class Helper
	{
	public:
		Helper()
		{
			//CCommandCtrl::getInstance();
		}
		~Helper()
		{
			CCommandCtrl::relaseInstance();
		}
	};

	static Helper m_helper;
};

