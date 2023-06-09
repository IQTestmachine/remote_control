
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
//public:
//	bool isClosed()
//	{
//		return m_isClosed;
//	}
//	void SetWatchStatus(bool isClosed = false)
//	{
//		m_isClosed = isClosed;
//	}
private:
	//static void threadEntryForWatchData(void* arg);//静态函数不能使用this指针, 因此声明如下函数辅助
	//void threadWatchData();
	//static void threadEntryForDownFile(void* arg);
	//void threadDownFile();
	void InitUIData();
	void Str2Tree(const std::string& driver, CTreeCtrl& tree);
	void UpdateFileInfo(const FILEINFO& finfo, HTREEITEM hParent);
	void UpdateDownloadFile(const std::string& strData, FILE* pFile);
	void LoadFileCurrent(HTREEITEM lParam);
	//void LoadFileCurrent();
	void LoadFileInfo();
	CString GetPath(HTREEITEM hTree);
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	//返回值是命令号, 若小于0则表示错误
	/*int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = nullptr, size_t nLength = 0);*/


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
	afx_msg void OnBnClickedAbort();
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
	afx_msg LRESULT OnSendPacketAck(WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedBtnStartWatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClickedBtnTest();
	afx_msg void OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeEditPort();
};
