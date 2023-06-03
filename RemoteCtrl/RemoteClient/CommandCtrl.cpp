#include "pch.h"
#include "CommandCtrl.h"

CCommandCtrl*  CCommandCtrl::m_instance = nullptr;
std::map<UINT, CCommandCtrl::MSGFUNC> CCommandCtrl::m_mapFunc;

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
			{WM_SEND_PACK, &CCommandCtrl::OnSendPack},
			{WM_SEND_PACK, &CCommandCtrl::OnSendData},
			{WM_SHOW_STATUS, &CCommandCtrl::OnShowStatus},
			{WM_SHOW_STATUS, &CCommandCtrl::OnShoWatcher},
			{(UINT) - 1, nullptr}
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
	m_hTread = (HANDLE)_beginthreadex(nullptr, 0, &CCommandCtrl::threadEntry, this, 0, &m_nThreadID);//创建线程
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

	WaitForSingleObject(hEvent, -1);
	LRESULT ret = info.result;
	return ret;
}

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

LRESULT CCommandCtrl::OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CCommandCtrl::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CCommandCtrl::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CCommandCtrl::OnShoWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return m_watchDlg.DoModal();
}
