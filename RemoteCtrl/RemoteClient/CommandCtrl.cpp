#include "pch.h"
#include "CommandCtrl.h"

CCommandCtrl*  CCommandCtrl::m_instance = nullptr;
std::map<UINT, CCommandCtrl::MSGFUNC> CCommandCtrl::m_mapFunc;
CCommandCtrl::Helper CCommandCtrl::m_helper;

CCommandCtrl* CCommandCtrl::getInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new CCommandCtrl();
		struct
		{
			UINT nMsg;
			MSGFUNC func;
		}MsgFunc[] =
		{
			//{WM_SEND_PACK, &CCommandCtrl::OnSendPack},
			//{WM_SEND_PACK, &CCommandCtrl::OnSendData},
			{WM_SHOW_STATUS, &CCommandCtrl::OnShowStatus},
			{WM_SHOW_STATUS, &CCommandCtrl::OnShoWatcher},
			{(UINT)1, nullptr}
		};
		for (int i = 0; MsgFunc[i].func != nullptr; i++)
		{
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(MsgFunc[i].nMsg, MsgFunc[i].func));
		}
	}
	return m_instance;
}

int CCommandCtrl::InitController()
{
	//m_hTread = (HANDLE)_beginthreadex(nullptr, 0, &CCommandCtrl::threadEntry, this, 0, &m_nThreadID);//创建线程
	m_statusDlg.Create(IDD_DIG_STATUS, &m_remoteDlg);//由于控件ID写错: IDD_DLG_STATUS

	return 0;
}

int CCommandCtrl::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_remoteDlg;
	return m_remoteDlg.DoModal();
}

LRESULT CCommandCtrl::SendMessage(MSG msg)
{
	HANDLE hEvent = CreateEvent(nullptr, true, false, nullptr);
	if (hEvent == nullptr)
		return -2;
	MSGINFO info(msg);
	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE, (LPARAM)&info, (WPARAM)&hEvent);
	WaitForSingleObject(hEvent, INFINITE);
	CloseHandle(hEvent);
	LRESULT ret = info.result;
	return ret;
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
		/*m_hThreadDownload = (HANDLE)_beginthread(&CCommandCtrl::threadEntryForDownFile, 0, this);
		if (WaitForSingleObject(m_hThreadDownload, 0) != WAIT_TIMEOUT)
		{
			return -1;
		}*/
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
		//if (m_watchDlg.m_isFull == false)//将截图数据存入到缓存
		//{
			//TRACE("%lld\r\n", nTick);
			if (GetTickCount64() - nTick < 200)
			{
				Sleep(200 - GetTickCount64() + nTick);
			}
			nTick = GetTickCount64();
			int ret = SendCommandPacket(m_watchDlg.GetSafeHwnd(), 6, true, nullptr, 0);
			/*if (ret == 1)
				TRACE("成功发送请求获取服务端截图命令包\r\n");*/
			//TODO: 添加消息相应函数WM_SEND_PACK_ACK
			//TODO: 控制发送频率
			//if (ret == 6)
			//{
			//	//CIQtestmachineTool::Bytes2Image(m_remoteDlg.GetImage(), lstPacks.front().strData);
			//	//if (GetImage(m_watchDlg.m_image/*GetImage()*/) == 0)
			//	if (CIQtestmachineTool::Bytes2Image(m_watchDlg.m_image, lstPacks.front().strData) == S_OK)
			//	{
			//		//TRACE("加载图片成功!\r\n");
			//		m_watchDlg.m_isFull = true;
			//	}
			//	else
			//		TRACE("获取图片失败!\r\n");
			//}
			//else
			//{
			//	Sleep(1);
			//}
		/*}
		Sleep(1);*/
	}
}

//void CCommandCtrl::threadEntryForDownFile(void* arg)
//{
//	CCommandCtrl* thiz = (CCommandCtrl*)arg;
//	thiz->threadDownFile();
//	_endthread();
//}
//
//void CCommandCtrl::threadDownFile()
//{
//	FILE* pFile = fopen(m_strLocal, "wb+");
//	if (pFile == nullptr)
//	{
//		AfxMessageBox("本地没有权限保存该文件, 或者文件无法创建!!!");
//		m_statusDlg.ShowWindow(SW_HIDE);
//		m_remoteDlg.EndWaitCursor();
//		return;
//	}
//	int ret = CCommandCtrl::getInstance()->SendCommandPacket(m_remoteDlg, 4, false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength(), (WPARAM)pFile);
//	CClientSocket* pClient = CClientSocket::getInstance();
//	if (ret < 0)
//	{
//		AfxMessageBox("执行下载命令失败!!!");
//		TRACE("执行下载失败: ret = %d\r\n", ret);
//		fclose(pFile);
//		pClient->CloseSocket();
//		return;
//	}
//	long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
//	//TRACE("文件的长度是%lld\r\n", nLength);
//	if (nLength == 0)
//	{
//		AfxMessageBox("文件长度为零或者无法读取文件!!!");
//		fclose(pFile);
//		pClient->CloseSocket();
//		return;
//	}
//	long long nCount = 0;
//	while (nCount < nLength)
//	{
//		ret = pClient->DealCommand();
//		if (ret < 0)
//		{
//			AfxMessageBox("传输失败!!!");
//			TRACE("传输失败: ret = %d\r\n", ret);
//			fclose(pFile);
//			pClient->CloseSocket();
//			return;
//		}
//		fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
//		nCount += pClient->GetPacket().strData.size();
//	}
//	pClient->DealCommand();//需处理服务端最后发送的空包, 否则短时间内再次点击下载文件会出现问题
//	fclose(pFile);
//	pClient->CloseSocket();
//	m_statusDlg.ShowWindow(SW_HIDE);
//	m_remoteDlg.EndWaitCursor();
//	m_remoteDlg.MessageBox(_T("下载完成"));
//}

void CCommandCtrl::threadFunc()
{
	MSG msg;
	while (::GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE)
		{
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(pmsg->msg.message);
			if (it != m_mapFunc.end())
				pmsg->result = (this->*(it->second))(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam); 
			else
				pmsg->result = -1;
			SetEvent(hEvent);
		}
		else
		{
			std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end())
			{
				(this->*(it->second))(msg.message, msg.wParam, msg.lParam);
			}
		}
	}
}

unsigned __stdcall CCommandCtrl::threadEntry(void* arg)
{
	CCommandCtrl* thiz = (CCommandCtrl*)arg;
	thiz->threadFunc();
	_endthreadex(0);
	return 0;
}

//LRESULT CCommandCtrl::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//	CClientSocket* pClient = CClientSocket::getInstance();
//	CPacket* pPacket = (CPacket*)wParam;
//	return pClient->Send(*pPacket);
//}
//
//LRESULT CCommandCtrl::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//
//	CClientSocket* pClient = CClientSocket::getInstance();
//	char* pBuffer = (char*)wParam;
//	return pClient->Send(pBuffer, (int)lParam);
//}

LRESULT CCommandCtrl::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CCommandCtrl::OnShoWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_watchDlg.DoModal();
}
