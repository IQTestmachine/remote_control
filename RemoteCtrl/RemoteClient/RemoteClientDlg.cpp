﻿
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


////非常重要的函数! 该函数将多种功能封装在了一起
////建立客户端套接字与服务端套接字的连接, 发送客户端的命令, 接收服务端处理命令后发送的一部分数据并从中解析出一个数据包
////每次调用该函数会重新建立客户端套接字与服务端套接字的连接, 并且注意88行注释
////注: 个人理解如果把这么多功能封装在一起, 至少应该改变一下函数名称的命名方式
////而且DealCommand()只能接收一部分数据并解析出一个数据包, 在这个函数里调用它应该不合理
//int CRemoteClientDlg::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t nLength)
//{
//	
//	int cmd = CCommandCtrl::getInstance()->SendCommandPacket(nCmd, bAutoClose, pData, nLength);
//	return cmd;
//}

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
	ON_WM_TIMER()
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData();
	m_server_address = 0xC0A8D382;//192.168.211.130 //0x7F000001 127.0.0.1;
	m_nPort = _T("9527");
	CCommandCtrl* pController = CCommandCtrl::getInstance();
	pController->UpdateAddress(m_server_address, atoi((LPCTSTR)m_nPort));
	UpdateData(FALSE);
	m_dlgStatus.Create(IDD_DIG_STATUS, this);
	m_dlgStatus.ShowWindow(SW_HIDE);
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

//void CRemoteClientDlg::threadEntryForWatchData(void* arg)
//{
//	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
//	thiz->threadWatchData();
//	_endthread();
//}
//
//void CRemoteClientDlg::threadWatchData()//可能存在线程异步问题
//{
//	Sleep(50);
//	CCommandCtrl* pController = CCommandCtrl::getInstance();
//	while (!m_isClosed)
//	{
//		if (m_isFull == false)//将截图数据存入到缓存
//		{
//			int ret = pController->SendCommandPacket(6, true, nullptr, 0);
//			if (ret == 6)
//			{
//				if (pController->GetImage(m_image) == 0)
//					m_isFull = true;
//				else
//					TRACE("获取图片失败!\r\n");
//			}
//			else
//			{
//				Sleep(1);
//			}
//		}
//		else
//		{
//			Sleep(1);
//		}
//	}
//}

//void CRemoteClientDlg::threadEntryForDownFile(void* arg)
//{
//	CRemoteClientDlg* thiz = (CRemoteClientDlg*)arg;
//	thiz->threadDownFile();
//	_endthread();
//}
//
//void CRemoteClientDlg::threadDownFile()
//{
//	int nListSelected = m_List.GetSelectionMark();//获得选择的标记(m_List文件列表下选中的那个框)
//	CString strFile = m_List.GetItemText(nListSelected, 0);//拿到文件名
//	//在本地(客户端)为要下载的文件创建环境, dlg对象包含要下载的文件名, 下载路径等一些信息
//	CFileDialog dlg(false, "*",
//		strFile, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, nullptr, this);
//	if (dlg.DoModal() == IDOK)
//	{
//		FILE* pFile = fopen(dlg.GetPathName(), "wb+");
//		if (pFile == nullptr)
//		{
//			AfxMessageBox("本地没有权限保存该文件, 或者文件无法创建!!!");
//			m_dlgStatus.ShowWindow(SW_HIDE);
//			EndWaitCursor();
//			return;
//		}
//		HTREEITEM hSelected = m_Tree.GetSelectedItem();
//		strFile = GetPath(hSelected) + strFile;//获取文件完整路径
//		TRACE("%s\r\n", LPCSTR(strFile));
//		int ret = CCommandCtrl::getInstance()->SendCommandPacket(4, false, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
//		CClientSocket* pClient = CClientSocket::getInstance();
//		if (ret < 0)
//		{
//			AfxMessageBox("执行下载命令失败!!!");
//			TRACE("执行下载失败: ret = %d\r\n", ret);
//			fclose(pFile);
//			pClient->CloseSocket();
//			return;
//		}
//		long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
//		//TRACE("文件的长度是%lld\r\n", nLength);
//		if (nLength == 0)
//		{
//			AfxMessageBox("文件长度为零或者无法读取文件!!!");
//			fclose(pFile);
//			pClient->CloseSocket();
//			return;
//		}
//		long long nCount = 0;
//		while (nCount < nLength)
//		{
//			ret = pClient->DealCommand();
//			if (ret < 0)
//			{
//				AfxMessageBox("传输失败!!!");
//				TRACE("传输失败: ret = %d\r\n", ret);
//				fclose(pFile);
//				pClient->CloseSocket();
//				return;
//			}
//			fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
//			nCount += pClient->GetPacket().strData.size();
//		}
//		pClient->DealCommand();//需处理服务端最后发送的空包, 否则短时间内再次点击下载文件会出现问题
//		fclose(pFile);
//		pClient->CloseSocket();
//	}
//	m_dlgStatus.ShowWindow(SW_HIDE);
//	EndWaitCursor();
//	MessageBox(_T("下载完成"));
//}

void CRemoteClientDlg::Str2Tree(const std::string& driver, CTreeCtrl& tree)
{
}

void CRemoteClientDlg::LoadFileCurrent()//删除文件后调用该函数实现列表刷新
{
	HTREEITEM hTree = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hTree);
	m_List.DeleteAllItems();
	TRACE("%s", strPath);
	int cmd = CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	TRACE("%d", cmd);
	CClientSocket* pClient = CClientSocket::getInstance();
	PFILEINFO pInfo = (PFILEINFO)pClient->GetPacket().strData.c_str();
	while (pInfo->HasNext)
	{
		TRACE("szFilename = %s, IsDirectory = %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
		if (!pInfo->IsDirectory)
			m_List.InsertItem(0, pInfo->szFileName);
		int cmd = pClient->DealCommand();
		TRACE("ack: %d\r\n", cmd);
		if (cmd < 0)
			break;
		pInfo = PFILEINFO(CClientSocket::getInstance()->GetPacket().strData.c_str());
	}
	pClient->CloseSocket();
}

void CRemoteClientDlg::LoadFileInfo()//展开指定目录下的文件夹(m_Tree)与文件(m_List)
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_Tree.ScreenToClient(&ptMouse);
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);
	if (hTreeSelected == nullptr)//检测是否双击或单击
		return;

	/*if (m_Tree.GetChildItem(hTreeSelected) == nullptr)
		return;*/

	DeleteTreeChildrenItem(hTreeSelected);//重新点击时先清空原来的内容
	m_List.DeleteAllItems();//重新点击时先清空原来的内容
	int counts = 0;//文件夹与文件夹的总数量
	CString strPath = GetPath(hTreeSelected);
	//TRACE("%s\r\n", strPath);
	std::list<CPacket> lstPackets;
	int cmd = CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength(), (WPARAM)hTreeSelected);
	TRACE("%d\r\n", cmd);
	////PFILEINFO pInfo = (PFILEINFO)lstPackets.front().strData.c_str();
	//lstPackets.pop_back();
	//if (lstPackets.size() > 0)
	//{
	//	for (auto it = lstPackets.begin(); it != lstPackets.end(); it++)
	//	{
	//		PFILEINFO pInfo = (PFILEINFO)(*it).strData.c_str();
	//		if (pInfo->IsDirectory)
	//		{
	//			if (CString(pInfo->szFileName) == "." || CString(pInfo->szFileName) == "..")
	//			{
	//				continue;
	//			}
	//			HTREEITEM hTmp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
	//			m_Tree.InsertItem("", hTmp, TVI_LAST);//似乎没有必要追加空字符串
	//		}
	//		else
	//		{
	//			m_List.InsertItem(0, pInfo->szFileName);
	//		}
	//	}
		/*lstPackets.pop_front();
		pInfo = PFILEINFO(lstPackets.front().strData.c_str());*/

	//CClientSocket* pClient = CClientSocket::getInstance();
	//PFILEINFO pInfo = (PFILEINFO)pClient->GetPacket().strData.c_str();
	//while (pInfo->HasNext)
	//{
	//	//TRACE("szFilename = %s, IsDirectory = %d, HasNext = %d\r\n", pInfo->szFileName, pInfo->IsDirectory, pInfo->HasNext);
	//	if (pInfo->IsDirectory)
	//	{
	//		if (CString(pInfo->szFileName) == "." || CString(pInfo->szFileName) == "..")
	//		{
	//			int cmd = pClient->DealCommand();
	//			//TRACE("ack: %d\r\n", cmd);
	//			if (cmd < 0)
	//				break;
	//			pInfo = PFILEINFO(CClientSocket::getInstance()->GetPacket().strData.c_str());
	//			continue;				
	//		}
	//		HTREEITEM hTmp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
	//		m_Tree.InsertItem("", hTmp, TVI_LAST);//似乎没有必要追加空字符串
	//	}
	//	else
	//	{
	//		m_List.InsertItem(0, pInfo->szFileName);
	//	}
	//	counts++;
	//	int cmd = pClient->DealCommand();
	//	//TRACE("ack: %d\r\n", cmd);
	//	if (cmd < 0)
	//		break;
	//	pInfo = PFILEINFO(CClientSocket::getInstance()->GetPacket().strData.c_str());
	//}

	//CCommandCtrl::getInstance()->CloseSocket();
	//TRACE("counts = %d\r\n", counts);
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
	//////添加线程处理函数
	//_beginthread(CRemoteClientDlg::threadEntryForDownFile, 0, this);
	//BeginWaitCursor();//将光标设置为一个沙漏, 表示等待状态
	//m_dlgStatus.m_info.SetWindowText(_T("命令正在执行中"));
	//m_dlgStatus.ShowWindow(SW_SHOW);
	//m_dlgStatus.CenterWindow(this);
	//m_dlgStatus.SetActiveWindow();
}


void CRemoteClientDlg::OnDeleteFile()//点击删除文件的事件处理程序(函数)
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);
	strFile = strPath + strFile;
	int ret = CCommandCtrl::getInstance()->SendCommandPacket(GetSafeHwnd(), 9, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0)
	{
		AfxMessageBox("删除文件命令执行失败!!!");
	}
	LoadFileCurrent();
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
		//未能连接到服务端或者未能成功发送命令
	}
	else if (lParam == 1)
	{
		//已全部接收服务端处理某个命令发送的数据包(此时服务端套接字已关闭)
	}
	if (lParam == 0)
	{
		if (wParam != NULL)
		{
			CPacket packAck = *(CPacket*)wParam;
			delete (CPacket*)wParam;
			switch (packAck.sCmd)
			{
			case 1:
			{
				std::string drivers = packAck.strData;
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
			case 2:
			{
				PFILEINFO pInfo = (PFILEINFO)packAck.strData.c_str();
				if (pInfo->HasNext == false)
					break;
				if (pInfo->HasNext)
				{
					if (pInfo->IsDirectory)
					{
						if (CString(pInfo->szFileName) == "." || CString(pInfo->szFileName) == "..")
							break;
						HTREEITEM hTmp = m_Tree.InsertItem(pInfo->szFileName, (HTREEITEM)lParam, TVI_LAST);
						m_Tree.InsertItem("", hTmp, TVI_LAST);//似乎没有必要追加空字符串
					}
					else
					{
						m_List.InsertItem(0, pInfo->szFileName);
					}
				}
			}
			case 3:
				break;
			case 4:
			{
				static long long length = 0, index = 0;
				if (length == 0)
				{
					length = *(long long*)packAck.strData.c_str();
					if (length == 0)
					{
						AfxMessageBox("文件长度为零或者无法读取文件!!!");
						CCommandCtrl::getInstance()->DownFileEnd();
					}
				}
				else
				{
					FILE* pFile = (FILE*)lParam;
					if (packAck.strData.size() == 0)//重构后以接收到空包作为下载文件的结束
					{
						CCommandCtrl::getInstance()->DownFileEnd();
						if (index != length)
						{
							AfxMessageBox("文件下载过程中存在数据包丢失!");
						}
						index = 0;
						length = 0;
						break;
					}
					fwrite(packAck.strData.c_str(), 1, packAck.strData.size(), pFile);
					index += packAck.strData.size();
				}
				break;
			}
			case 9:
				break;
			case 1981:
				TRACE("已收到连接测试的应答数据包\r\n");
				break;
			default:
				break;
			}
		}
	}
	return 0;
}

//LRESULT CRemoteClientDlg::OnSendPacket(WPARAM wParam, LPARAM lParam)//第四步: 定义消息响应函数
//{
//	int ret = 0;
//	int cmd = wParam >> 1;
//	TRACE("命令号cmd = %d\r\n", cmd);
//	switch (cmd)
//	{
//	case 4:
//	{
//		CString strFile = (LPCSTR)lParam;
//		TRACE("%d\r\n", wParam & 1);
//		ret = CCommandCtrl::getInstance()->SendCommandPacket(cmd, wParam & 1, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
//		break;
//	}
//	case 5:
//	{
//		ret = CCommandCtrl::getInstance()->SendCommandPacket(cmd, wParam & 1, (BYTE*)lParam, sizeof(MOUSEEV));
//		break;
//	}
//	case 6:
//	case 7:
//	case 8:
//	{
//		ret = CCommandCtrl::getInstance()->SendCommandPacket(cmd, wParam & 1);
//		break;
//	}
//	default:
//		ret = -1;
//	}
//	
//	return ret;
//}

void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{
	int ret = CCommandCtrl::getInstance()->StartWatchScreen();
	/*m_isClosed = false;
	CWatchDialog dlg(this);
	HANDLE hThread = (HANDLE)_beginthread(CRemoteClientDlg::threadEntryForWatchData, 0, this);
	dlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(hThread, 500);*/
}


void CRemoteClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CDialogEx::OnTimer(nIDEvent);
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
