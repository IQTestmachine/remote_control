
// RemoteClientDlg.h: 头文件
//

#pragma once
#include "CClientSocket.h"
#include "StatusDlg.h"

//#define WM_SEND_PACKET (WM_USER + 10)//第一步:发送数据包的消息
#ifndef WM_SEND_PACK_ACK
#define WM_SEND_PACK_ACK (WM_USER + 100)
#endif

// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
	void InitUIData();//初始化RemoteDlg的UI, 以及IP地址与端口号
	void Str2Tree(const std::string& driver, CTreeCtrl& tree);//将获取的磁盘分区数据包转换为树的形式
	void UpdateFileInfo(const FILEINFO& finfo, HTREEITEM hParent);//将获取的文件夹或文件插入到对应的控件
	void UpdateDownloadFile(const std::string& strData, FILE* pFile);//将下载文件所接收到的数据写入客户端对应的文件中
	void LoadFileCurrent(HTREEITEM lParam);//删除文件后实现列表刷新
	void LoadFileInfo();
	CString GetPath(HTREEITEM hTree);
	void DeleteTreeChildrenItem(HTREEITEM hTree);


// 实现
protected:
	HICON m_hIcon;
	CStatusDlg m_dlgStatus;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_nPort;
	DWORD m_server_address;
	afx_msg void OnBnClickedButFileinfo();
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// 文件列表
	CListCtrl m_List;
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnOpenFile();
	//afx_msg LRESULT OnSendPacket(WPARAM wParam, LPARAM lParam);//第二步:声明自定义消息函数
	afx_msg LRESULT OnSendPacketAck(WPARAM wParam, LPARAM lParam);//自定义消息响应函数, 用于显示数据, 将套接字接收到的数据包转化为相应的可视化数据

	afx_msg void OnBnClickedBtnStartWatch();
	afx_msg void OnClickedBtnTest();
	afx_msg void OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditPort();
};
