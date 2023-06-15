
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "WatchDialog.h"
#include "afxdialogex.h"
#include "CommandCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "WatchDialog.h"

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_nPort(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteClientDlg::OnClickedBtnTest)
	ON_BN_CLICKED(IDC_BUT_FILEINFO, &CRemoteClientDlg::OnBnClickedButFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteClientDlg::OnDeleteFile)
	ON_COMMAND(ID_OPEN_FILE, &CRemoteClientDlg::OnOpenFile)
	//ON_MESSAGE(WM_SEND_PACKET, &CRemoteClientDlg::OnSendPacket)//第三步: 注册消息
	ON_BN_CLICKED(IDC_BTN_START_WATCH, &CRemoteClientDlg::OnBnClickedBtnStartWatch)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS_SERV, &CRemoteClientDlg::OnIpnFieldchangedIpaddressServ)
	ON_EN_CHANGE(IDC_EDIT_PORT, &CRemoteClientDlg::OnEnChangeEditPort)
	ON_MESSAGE(WM_SEND_PACK_ACK, &CRemoteClientDlg::OnSendPacketAck)
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// TODO: 在此添加额外的初始化代码
	InitUIData();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRemoteClientDlg::OnClickedBtnTest()
{
	// TODO: 在此添加控件通知处理程序代码
	CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 1981);
}


void CRemoteClientDlg::OnBnClickedButFileinfo()
{
	// TODO: 在此添加控件通知处理程序代码
	int ret = CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 1, true, nullptr, 0);
	if (ret == -1)
	{
		AfxMessageBox(_T("命令处理失败"));
		return;
	}
}

void CRemoteClientDlg::InitUIData()
{
	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	UpdateData();
	m_server_address = 0x7F000001;// 0xC0A8D382;//192.168.211.130 //0x7F000001 127.0.0.1;
	m_nPort = _T("9527");
	CCommandCtrl* pController = CCommandCtrl::getInstance();
	pController->UpdateAddress(m_server_address, atoi((LPCTSTR)m_nPort));
	UpdateData(FALSE);
	m_dlgStatus.Create(IDD_DIG_STATUS, this);
	m_dlgStatus.ShowWindow(SW_HIDE);
}

void CRemoteClientDlg::Str2Tree(const std::string& drivers, CTreeCtrl& tree)
{
	TRACE("drivers = %s\r\n", drivers.c_str());
	std::string dr;
	m_Tree.DeleteAllItems();//因为被控制端的文件目录是一个树结构, 所以这里添加的变量m_Tree应该是一个树. 这里的函数目标是清空树, 防止因为多次点击导致树越来越大
	for (size_t i = 0; i < drivers.size(); i++)
	{
		if (drivers[i] != ',')
			dr += drivers[i];
		if (drivers[i] == ',' || i == drivers.size() - 1)
		{
			dr += ":";
			//m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST); 添加到根目录下, 以追加的方式添加
			HTREEITEM hTmp = m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);//? 上一行代码更改为这一行及下一行代码, 不理解
			m_Tree.InsertItem("", hTmp, TVI_LAST);
			dr.clear();
		}
	}
}

void CRemoteClientDlg::UpdateFileInfo(const FILEINFO& finfo, HTREEITEM hParent)
{
	if (finfo.HasNext == false)
		return;
	if (finfo.HasNext)
	{
		if (finfo.IsDirectory)
		{
			if (CString(finfo.szFileName) == "." || CString(finfo.szFileName) == "..")
				return;
			HTREEITEM hTmp = m_Tree.InsertItem(finfo.szFileName, hParent, TVI_LAST);
			m_Tree.InsertItem("", hTmp, TVI_LAST);//似乎没有必要追加空字符串
			m_Tree.Expand(hParent, TVE_EXPAND);
		}
		else
		{
			m_List.InsertItem(0, finfo.szFileName);
		}
	}
}

void CRemoteClientDlg::UpdateDownloadFile(const std::string& strData, FILE* pFile)
{
	static long long length = 0, index = 0;
	if (length == 0)
	{
		length = *(long long*)strData.c_str();
		TRACE("length = %d\r\n", (int)length);
		if (length == 0)
		{
			AfxMessageBox("文件长度为零或者无法读取文件!!!");
			CCommandCtrl::getInstance()->DownFileEnd();
		}
	}
	else
	{
		if (strData.size() == 0)//重构后以接收到空包作为下载文件的结束
		{
			fclose(pFile);
			CCommandCtrl::getInstance()->DownFileEnd();
			if (index != length)
			{
				MessageBox("下载文件", "文件下载过程中存在数据包丢失!", MB_ICONINFORMATION);
			}
			index = 0;
			length = 0;
			return;
		}
		fwrite(strData.c_str(), 1, strData.size(), pFile);
		index += strData.size();
	}
}

void CRemoteClientDlg::LoadFileCurrent(HTREEITEM lParam)
{
	m_List.DeleteAllItems();//重新点击时先清空原来的内容
	CString strPath = GetPath(lParam);
	int cmd = CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength(), (WPARAM)lParam);
}


void CRemoteClientDlg::LoadFileInfo()//展开指定目录下的文件夹(m_Tree)与文件(m_List)
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_Tree.ScreenToClient(&ptMouse);
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);
	if (hTreeSelected == nullptr)//检测是否双击或单击
		return;

	DeleteTreeChildrenItem(hTreeSelected);//重新点击时先清空原来的内容
	m_List.DeleteAllItems();//重新点击时先清空原来的内容
	CString strPath = GetPath(hTreeSelected);
	TRACE("%s\r\n", strPath);
	int cmd = CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength(), (WPARAM)hTreeSelected);
}

CString CRemoteClientDlg::GetPath(HTREEITEM hTree)//获取当前点击位置的详细路径
{
	CString strRet, strTmp;
	do {
		strTmp = m_Tree.GetItemText(hTree);//当前所点击目录(文件夹)名
		strRet = strTmp + "\\" + strRet;
		hTree = m_Tree.GetParentItem(hTree);//获取父目录
	} while (hTree != nullptr);
	return strRet;
}

void CRemoteClientDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{
	HTREEITEM hSub = nullptr;
	do {
		hSub = m_Tree.GetChildItem(hTree);
		if (hSub != nullptr)
			m_Tree.DeleteItem(hSub);
	} while (hSub != nullptr);
}

void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse, ptList;//有两个变量是为了保证点击后还能继续
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);
	int ListSelected = m_List.HitTest(ptList);
	if (ListSelected < 0)
		return;
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);//加载整个菜单
	CMenu* pPupup = menu.GetSubMenu(0);//取整个菜单的子菜单的第一个
	if (pPupup != nullptr)
	{
		pPupup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);
	}
}

void CRemoteClientDlg::OnDownloadFile()//点击下载文件的事件处理程序(函数)
{
	int nListSelected = m_List.GetSelectionMark();//获得选择的标记(m_List文件列表下选中的那个框)
	CString strFile = m_List.GetItemText(nListSelected, 0);//拿到文件名
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	strFile = GetPath(hSelected) + strFile;//获取文件完整路径
	TRACE("%s\r\n", LPCSTR(strFile));
	int ret = CCommandCtrl::getInstance()->DownFile(strFile);
	if (ret != 0)
	{
		AfxMessageBox("下载失败!!!");
		TRACE("下载失败: ret = %d\r\n", ret);
	}
}


void CRemoteClientDlg::OnDeleteFile()//点击删除文件的事件处理程序(函数)
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);
	strFile = strPath + strFile;
	int ret = CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 9, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength(), (WPARAM)hSelected);
	if (ret < 0)
	{
		AfxMessageBox("删除文件命令执行失败!!!");
	}
}

void CRemoteClientDlg::OnOpenFile()//点击打开文件的事件处理程序(函数)
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);
	strFile = strPath + strFile;
	int ret = CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 3, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0)
	{
		AfxMessageBox("打开文件命令执行失败!!!");
	}
}

LRESULT CRemoteClientDlg::OnSendPacketAck(WPARAM wParam, LPARAM lParam)
{
	if (lParam == -1 || lParam == -2)
	{
		TRACE("未能连接到服务端或者未能成功发送命令\r\n");
	}
	else if (lParam == 1)
	{
		TRACE("已全部接收服务端处理某个命令发送的数据包(此时服务端套接字已关闭)\r\n");
	}
	else 
	{
		if (wParam != NULL)
		{
			CPacket packAck = *(CPacket*)wParam;
			delete (CPacket*)wParam;
			switch (packAck.sCmd)
			{
			case 1:
			{
				TRACE("已收到获取磁盘分区的应答数据包\r\n");
				Str2Tree(packAck.strData, m_Tree);
				break;
			}
			case 2:
			{
				TRACE("已收到获得指定路径下文件夹和文件的一个应答数据包\r\n");
				PFILEINFO pInfo = (PFILEINFO)packAck.strData.c_str();
				//TRACE("%08X", packAck.strData.c_str());
				UpdateFileInfo(*pInfo, (HTREEITEM)lParam);
				break;
			}
			case 3:
				TRACE("已收到打开文件的应答数据包\r\n");
				break;
			case 4:
			{
				TRACE("已收到下载文件的应答数据包\r\n");
				UpdateDownloadFile(packAck.strData, (FILE*)lParam);
				break;
			}
			case 9:
				TRACE("已收到删除文件的应答数据包\r\n");
				LoadFileCurrent((HTREEITEM)lParam);
				break;
			case 1981:
				TRACE("已收到连接测试的应答数据包\r\n");
				MessageBox("连接成功", "连接测试", MB_ICONINFORMATION);
				break;
			default:
				break;
			}
		}
	}
	return 0;
}

void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{
	int ret = CCommandCtrl::getInstance()->StartWatchScreen();
}


void CRemoteClientDlg::OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	UpdateData();
	CCommandCtrl* pController = CCommandCtrl::getInstance();
	pController->UpdateAddress(m_server_address, atoi((LPCTSTR)m_nPort));
}


void CRemoteClientDlg::OnEnChangeEditPort()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData();//
	CCommandCtrl* pController = CCommandCtrl::getInstance();
	pController->UpdateAddress(m_server_address, atoi((LPCTSTR)m_nPort));
}
