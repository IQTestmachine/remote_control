#pragma once
#include "CClientSocket.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include <map>
#include "Resource.h"
#include "CClientSocket.h"
#include "IQtestmachineTool.h"

//#define WM_SEND_PACK (WM_USER + 1) //发送包数据
//#define WM_SEND_PACK (WM_USER + 2) //发送数据
#define WM_SHOW_STATUS (WM_USER + 3) //展示状态
#define WM_SHOW_STATUS (WM_USER + 3) //远程监控
#define WM_SEND_MESSAGE (WM_USER + 0x10000)//自定义消息处理

class CCommandCtrl
{
public:
	//获取全局唯一对象
	static CCommandCtrl* getInstance();
	//初始化操作
	int InitController();
	//启动
	int Invoke(CWnd*& pMainWnd);
	//发送消息
	LRESULT SendMessage(MSG msg);
	//更新网络服务器地址
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

	//1.查看磁盘分区
	//2.查看指定目录下的文件
	//3.打开文件
	//4.下载文件
	//5.鼠标操作
	//6.请求获得服务端屏幕截图
	//7.锁机
	//8.解锁
	//9.删除文件
	//1981. 测试连接 
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
		CloseHandle(hEvent);//回收事件句柄, 防止资源耗尽
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
	static void threadEntryForWatchData(void* arg);//静态函数不能使用this指针, 因此声明如下函数辅助
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
	static std::map<UINT, MSGFUNC> m_mapFunc;//消息函数指针映射表
	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hTread;
	HANDLE m_hThreadDownload;
	//下载文件的远程路径
	CString m_strRemote;
	//下载文件的本地路径
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

