#include "pch.h"
#include "CommandCtrl.h"

CCommandCtrl*  CCommandCtrl::m_instance = nullptr;
//std::map<UINT, CCommandCtrl::MSGFUNC> CCommandCtrl::m_mapFunc;
CCommandCtrl::Helper CCommandCtrl::m_helper;

CCommandCtrl* CCommandCtrl::getInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new CCommandCtrl();
		/*struct
		{
			UINT nMsg;
			MSGFUNC func;
		}MsgFunc[] =
		{
			{WM_SHOW_STATUS, &CCommandCtrl::OnShowStatus},
			{WM_SHOW_STATUS, &CCommandCtrl::OnShoWatcher},
			{(UINT)1, nullptr}
		};
		for (int i = 0; MsgFunc[i].func != nullptr; i++)
		{
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(MsgFunc[i].nMsg, MsgFunc[i].func));
		}*/
	}
	return m_instance;
}

int CCommandCtrl::InitController()
{
	m_statusDlg.Create(IDD_DIG_STATUS, &m_remoteDlg);//由于控件ID写错: IDD_DLG_STATUS

	return 0;
}

int CCommandCtrl::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_remoteDlg;
	return m_remoteDlg.DoModal();
}

bool CCommandCtrl::SendCommandPacket(HWND hWnd, int nCmd, bool bAutoClose, BYTE* pData, size_t nLength, WPARAM wParam)
{
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(nCmd, pData, nLength);
	bool ret = pClient->SendPacket(hWnd, pack, bAutoClose, wParam);
	return ret;
}

int CCommandCtrl::DownFile(CString strPath)
{
	//在本地(客户端)为要下载的文件创建环境, dlg对象包含要下载的文件名, 下载路径等一些信息
	CFileDialog dlg(false, "*",
		strPath, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, nullptr, &m_remoteDlg);
	if (dlg.DoModal() == IDOK)
	{
		m_strRemote = strPath;
		m_strLocal = dlg.GetPathName();
		FILE* pFile = fopen(m_strLocal, "wb+");
		if (pFile == nullptr)
		{
			AfxMessageBox("本地没有权限保存该文件, 或者文件无法创建!!!");
			return 1;
		}
		SendCommandPacket(m_remoteDlg, 4, false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength(), (WPARAM)pFile);
		m_remoteDlg.BeginWaitCursor();//将光标设置为一个沙漏, 表示等待状态
		m_statusDlg.m_info.SetWindowText(_T("命令正在执行中"));
		m_statusDlg.ShowWindow(SW_SHOW);
		m_statusDlg.CenterWindow(&m_remoteDlg);
		m_statusDlg.SetActiveWindow();
	}
	return 0;
}

void CCommandCtrl::DownFileEnd()
{
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("下载完成"));
}

int CCommandCtrl::StartWatchScreen()
{
	m_watchDlg.m_isClosed = false;
	m_hThreadWatch = (HANDLE)_beginthread(&CCommandCtrl::threadEntryForWatchData, 0, this);
	m_watchDlg.DoModal();
	m_watchDlg.m_isClosed = true;
	WaitForSingleObject(m_hThreadWatch, 500);
	return 0;
}

void CCommandCtrl::threadEntryForWatchData(void* arg)
{
	CCommandCtrl* thiz = (CCommandCtrl*)arg;
	thiz->threadWatchData();
	TRACE("监控服务端屏幕线程已结束\r\n");
	_endthread();
}

void CCommandCtrl::threadWatchData()
{
	ULONGLONG nTick = GetTickCount64();
	while (!m_watchDlg.m_isClosed)
	{
		if (GetTickCount64() - nTick < 200)
		{
			Sleep(200 - GetTickCount64() + nTick);
		}
		nTick = GetTickCount64();
		int ret = SendCommandPacket(m_watchDlg.GetSafeHwnd(), 6, true, nullptr, 0);
	}
}