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
	ON_MESSAGE(WM_SEND_PACK_ACK, &CWatchDialog::OnSendPacketAck)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序

CPoint CWatchDialog::UserPointtoRemotePoint(CPoint& point, bool isScreen)
{
	CRect clientRect;
	if (!isScreen)
		ClientToScreen(&point);//转换为相对客户端屏幕坐上角的坐标(全局坐标), 初始坐标实际上是相对于监视对话框左上角的坐标
	m_picture.ScreenToClient(&point);//全局坐标转换为相对于m_pictual控件的左上角坐标
	m_picture.GetWindowRect(clientRect);
	//转换到服务端屏幕坐标
	return CPoint(point.x * m_nObjWidth / clientRect.Width(), (point.y) * m_nObjHeight / clientRect.Height());
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

//该函数用于显示数据, 将套接字接收到的数据包转化为相应的可视化数据
LRESULT CWatchDialog::OnSendPacketAck(WPARAM wParam, LPARAM lParam)
{
	if (lParam == -1 || lParam == -2)
	{
		TRACE("未能连接到服务端或者未能成功发送命令");
	}
	else if (lParam == 1)
	{
		TRACE("已全部接收服务端处理某个命令发送的数据包(此时服务端套接字已关闭)");
	}
	else
	{
		if (wParam != NULL)
		{
			CPacket packAck = *(CPacket*)wParam;
			delete (CPacket*)wParam;
			switch (packAck.sCmd)
			{
			case 6:
			{
				CIQtestmachineTool::Bytes2Image(m_image, packAck.strData);
				CRect rect;
				m_picture.GetWindowRect(rect);
				m_image.StretchBlt(
					m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
				m_picture.InvalidateRect(nullptr);
				m_nObjWidth = m_image.GetWidth();
				m_nObjHeight = m_image.GetHeight();
				//TRACE("服务端截图宽高: m_nObjWidth = %d, m_nObjHeight = %d\r\n", m_nObjWidth, m_nObjHeight);
				m_image.Destroy();
				//TRACE("已显示截图\r\n");
				break;
			}
			case 5:
				TRACE("服务端已执行鼠标命令: %d\r\n", packAck.sCmd);
				break;
			case 7:
				break;
			case 8:
				TRACE("服务端已执行解锁命令: %d\r\n", packAck.sCmd);
				break;
			default:
				break;
			}
		}
	}
	return 0;
}

void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint remote = UserPointtoRemotePoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 3;//抬起
		CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint remote = UserPointtoRemotePoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 1;//双击
		CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint remote = UserPointtoRemotePoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 2;//按下
		CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint remote = UserPointtoRemotePoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 1;//双击
		CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialogEx::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint remote = UserPointtoRemotePoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 2;//按下
		CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialogEx::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint remote = UserPointtoRemotePoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 3;//抬起
		CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialogEx::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1)
	{
		CPoint remote = UserPointtoRemotePoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 4;//没有按键
		event.nAction = 5;//移动
		CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
		Sleep(50);
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
		CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
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
	CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 7);
}


void CWatchDialog::OnBnClickedBtnUnlock()
{
	// TODO: 在此添加控件通知处理程序代码
	CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 8);
}
