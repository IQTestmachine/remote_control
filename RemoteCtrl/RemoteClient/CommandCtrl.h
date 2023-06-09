#pragma once
#include "CClientSocket.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include <map>
#include "Resource.h"
#include "CClientSocket.h"
#include "IQtestmachineTool.h"

class CCommandCtrl
{
public:
	//��ȡȫ��Ψһ����
	static CCommandCtrl* getInstance();
	//��ʼ������
	int InitController();
	//����
	int Invoke(CWnd*& pMainWnd);
	//���������������ַ
	void UpdateAddress(int nIP, int nPort)
	{
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}

	void CloseSocket()
	{
		CClientSocket::getInstance()->CloseSocket();
	}

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
	//����true��������ͳɹ�, false����ʧ��
	bool SendCommandPacket(
		HWND hWnd,//hWnd: �յ����ݰ���, ��ҪӦ��Ĵ���
		int nCmd, 
		bool bAutoClose = true, 
		BYTE* pData = nullptr, 
		size_t nLength = 0,
		WPARAM wParam = 0);

	int DownFile(CString strPath);
	void DownFileEnd();
	int StartWatchScreen();
protected:
	static void threadEntryForWatchData(void* arg);//��̬��������ʹ��thisָ��, ����������º�������
	void threadWatchData();
	CCommandCtrl() : m_statusDlg(&m_remoteDlg), m_watchDlg(&m_remoteDlg)
	{ 
		m_hThreadDownload = INVALID_HANDLE_VALUE;
		m_hThreadWatch = INVALID_HANDLE_VALUE;
	}

	~CCommandCtrl() { }

	static void relaseInstance()
	{
		if (m_instance != nullptr)
		{
			delete m_instance;
			m_instance = nullptr;
		}
	}

private:
	//typedef struct MsgInfo
	//{
	//	MSG msg;
	//	LRESULT result;
	//	MsgInfo(MSG m)
	//	{
	//		result = 0;
	//		memcpy(&msg, &m, sizeof(MSG));
	//	}
	//	MsgInfo(const MsgInfo& m)
	//	{
	//		result = m.result;
	//		memcpy(&msg, &m.msg, sizeof(MSG));
	//	}

	//	MsgInfo& operator=(const MsgInfo& m)
	//	{
	//		if (this != &m)
	//		{
	//			result = m.result;
	//			memcpy(&msg, &m.msg, sizeof(MSG));
	//		}
	//		return *this;
	//	}

	//}MSGINFO;
	//typedef LRESULT (CCommandCtrl::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	//static std::map<UINT, MSGFUNC> m_mapFunc;//��Ϣ����ָ��ӳ��� ������Ϊʲô���Ʋ����������static, �������ȴ������
	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThreadDownload;
	//�����ļ���Զ��·��
	CString m_strRemote;
	//�����ļ��ı���·��
	CString m_strLocal;
	HANDLE m_hThreadWatch;
	static CCommandCtrl* m_instance;
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

