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
	//获取全局唯一对象
	static CCommandCtrl* getInstance();
	//初始化操作
	int InitController();
	//启动
	int Invoke(CWnd*& pMainWnd);
	//更新网络服务器地址
	void UpdateAddress(int nIP, int nPort)
	{
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}

	void CloseSocket()
	{
		CClientSocket::getInstance()->CloseSocket();
	}

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
	//返回true代表命令发送成功, false代表失败
	bool SendCommandPacket(
		HWND hWnd,//hWnd: 收到数据包后, 需要应答的窗口
		int nCmd, 
		bool bAutoClose = true, 
		BYTE* pData = nullptr, 
		size_t nLength = 0,
		WPARAM wParam = 0);

	int DownFile(CString strPath);
	void DownFileEnd();
	int StartWatchScreen();
protected:
	static void threadEntryForWatchData(void* arg);//静态函数不能使用this指针, 因此声明如下函数辅助
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
	//static std::map<UINT, MSGFUNC> m_mapFunc;//消息函数指针映射表 不明白为什么控制层这里可以用static, 而网络层却不可以
	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThreadDownload;
	//下载文件的远程路径
	CString m_strRemote;
	//下载文件的本地路径
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

