// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDialog.h"
#include "CommandCtrl.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialogEx)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_WATCH, pParent)
{
	m_nObjWidth = -1;
	m_nObjHeight = -1;
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
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDialog::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDialog::OnBnClickedBtnUnlock)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPointtoRemotePoint(CPoint& point, bool isScreen)
{
	CRect clientRect;
	//if (isScreen == true)
	//	ScreenToClient(&point);//全局坐标到相对坐标
	if (!isScreen)
		ClientToScreen(&point);//转换为相对客户端屏幕坐上角的坐标(全局坐标), 初始坐标实际上是相对于监视对话框左上角的坐标
	m_picture.ScreenToClient(&point);//全局坐标转换为相对于m_pictual控件的左上角坐标
	//TRACE("客户端鼠标位置: point.x = %d, point.y = %d\r\n", point.x, point.y);
	m_picture.GetWindowRect(clientRect);
	//TRACE("客户端监视窗口的宽高: width = %d, height = %d\r\n", clientRect.Width(), clientRect.Height());
	//转换到服务端屏幕坐标
	return CPoint(point.x * m_nObjWidth / clientRect.Width(), (point.y) * m_nObjHeight / clientRect.Height());
	//return CPoint(point.x * m_nObjWidth / clientRect.Width(), (point.y - 76) * m_nObjHeight / clientRect.Height());//!注意减去76
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
		if (m_isFull)
		{
			//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			CRect rect;
			m_picture.GetWindowRect(rect);
			m_image.StretchBlt(
				m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(nullptr);
			//!注意获取服务端发送来的截图宽和高应在显示之后获取, 否则会崩溃, 崩溃的原因可能是
			//调用pParent->GetImage().GetWidth()之后导致m_image发生了一些未知改变
			m_nObjWidth = m_image.GetWidth();
			m_nObjHeight = m_image.GetHeight();
			//TRACE("服务端截图宽高: m_nObjWidth = %d, m_nObjHeight = %d\r\n", m_nObjWidth, m_nObjHeight);
			m_image.Destroy(); //在m_image.Load()之前使用了m_Destroy(), 这里似乎不用再调用
			m_isFull = false;
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		TRACE("进入监控鼠标左键抬起函数\r\n");
		//坐标转换
		CPoint remote = UserPointtoRemotePoint(point);
		//TRACE("转变为服务端的鼠标位置: x = %d, y = %d\r\n", remote.x, remote.y);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 3;//抬起
		CCommandCtrl::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
		/*CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);*/
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		//TRACE("进入监控鼠标左键双击函数\r\n");
		//坐标转换
		CPoint remote = UserPointtoRemotePoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 1;//双击
		CCommandCtrl::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
		/*CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);	*/
	}
	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		TRACE("进入监控鼠标左键按下函数\r\n");
		//TRACE("最初获取的鼠标位置: x = %d, y = %d\r\n", point.x, point.y);
		//坐标转换
		CPoint remote = UserPointtoRemotePoint(point);
		//TRACE("转变为服务端的鼠标位置: x = %d, y = %d\r\n", remote.x, remote.y);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 2;//按下
		CCommandCtrl::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
		/*CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);*/
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		//TRACE("进入监控鼠标右键双击函数\r\n");
		//坐标转换
		CPoint remote = UserPointtoRemotePoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 1;//双击
		CCommandCtrl::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
		/*CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);*/
	}
	CDialogEx::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		//TRACE("进入监控鼠标右键按下函数\r\n");
		//坐标转换
		CPoint remote = UserPointtoRemotePoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 2;//按下
		CCommandCtrl::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
		/*CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);*/
	}
	CDialogEx::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		//TRACE("进入监控鼠标右键弹开函数\r\n");
		//坐标转换
		CPoint remote = UserPointtoRemotePoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 3;//抬起
		CCommandCtrl::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
		/*CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);*/
	}
	CDialogEx::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		//TRACE("进入监控鼠标移动函数\r\n");
		//坐标转换
		CPoint remote = UserPointtoRemotePoint(point);
		//TRACE("转变为服务端的鼠标位置: point.x = %d, point.y = %d\r\n", point.x, point.y);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 4;//没有按键
		event.nAction = 5;//移动
		CCommandCtrl::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
		/*CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);*/
	}
	CDialogEx::OnMouseMove(nFlags, point);
}

void CWatchDialog::OnStnClickedWatch()//无法进入左键单击函数, 该函数似乎已经失效
{
	TRACE("进入监控鼠标单击函数\r\n");
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint point;
		GetCursorPos(&point);
		//坐标转换
		CPoint remote = UserPointtoRemotePoint(point, true);
		TRACE("转变为服务端的鼠标位置: x = %d, y = %d\r\n", remote.x, remote.y);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 0;//单击
		CCommandCtrl::getInstance()->SendCommandPacket(5, true, (BYTE*)&event, sizeof(event));
		/*CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);*/
	}
}


void CWatchDialog::OnOK()//使得按Enter键不会关闭监视窗口
{
	// TODO: 在此添加专用代码和/或调用基类
	//CDialogEx::OnOK();
}


void CWatchDialog::OnBnClickedBtnLock()
{
	// TODO: 在此添加控件通知处理程序代码
	CCommandCtrl::getInstance()->SendCommandPacket(7);
	/*CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 7 << 1 | 1);*/
}


void CWatchDialog::OnBnClickedBtnUnlock()
{
	// TODO: 在此添加控件通知处理程序代码
	CCommandCtrl::getInstance()->SendCommandPacket(8);
	/*CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 8 << 1 | 1);*/
}
