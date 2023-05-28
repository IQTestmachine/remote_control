// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialogEx)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_WATCH, pParent)
{

}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialogEx)
	ON_WM_TIMER()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPointtoRemotePoint(CPoint& point, bool isScreen)
{
	//800×450
	CRect clientRect;
	TRACE("point.x = %d, point.y = %d\r\n", point.x, point.y);
	if (isScreen == true)
		ScreenToClient(&point);//全局坐标到相对坐标
	m_picture.GetWindowRect(clientRect);
	TRACE("x = %d, y = %d\r\n", clientRect.Width(), clientRect.Height());
	//转换到客户端屏幕坐标
	return CPoint(point.x * 1920 / clientRect.Width(), point.y * 1080 / clientRect.Height());
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetTimer(0, 50, nullptr);
	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 0)
	{
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		if (pParent->isFull())
		{
			//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			CRect rect;
			m_picture.GetWindowRect(rect);
			pParent->GetImage().StretchBlt(
				m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(nullptr);
			//pParent->GetImage().Destroy(); //在m_image。Load()使用了m_Destroy(), 这里似乎不用再调用
			pParent->SetImageStatus();
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPointtoRemotePoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 3;//抬起
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPointtoRemotePoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 1;//双击
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM)&event);
	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPointtoRemotePoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 2;//按下
	TRACE("point.x = %d, point.y = %d\r\n", point.x, point.y);
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPointtoRemotePoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 1;//右键
	event.nAction = 1;//双击
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	CDialogEx::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPointtoRemotePoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 1;//右键
	event.nAction = 2;//按下
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	CDialogEx::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPointtoRemotePoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 1;//右键
	event.nAction = 3;//抬起
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	CDialogEx::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	return;
	//坐标转换
	CPoint remote = UserPointtoRemotePoint(point);
	TRACE("point.x = %d, point.y = %d\r\n", point.x, point.y);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 4;//没有按键
	event.nAction = 5;//移动
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	CDialogEx::OnMouseMove(nFlags, point);
}

void CWatchDialog::OnStnClickedWatch()
{
	CPoint point;
	GetCursorPos(&point);
	TRACE("point.x = %d, point.y = %d\r\n", point.x, point.y);
	//坐标转换
	CPoint remote = UserPointtoRemotePoint(point, true);
	TRACE("point.x = %d, point.y = %d\r\n", point.x, point.y);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 0;//单击
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
}
