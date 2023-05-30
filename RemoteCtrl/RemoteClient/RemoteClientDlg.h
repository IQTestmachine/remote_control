
// RemoteClientDlg.h: 头文件
//

#pragma once
#include "CClientSocket.h"
#include "StatusDlg.h"

#define WM_SEND_PACKET (WM_USER + 10)//第一步:发送数据包的消息


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
public:
	bool isFull() const
	{
		return m_isFull;
	}
	CImage GetImage()
	{
		return m_image;
	}
	void SetImageStatus(bool isFull = false)
	{
		m_isFull = isFull;
	}
private:
	CImage m_image;//缓存(采用一张图片来用作缓存区域)
	bool m_isFull;//缓存是否有数据
	bool m_isClosed;//监视是否关闭
private:
	static void threadEntryForWatchData(void* arg);//静态函数不能使用this指针, 因此声明如下函数辅助
	void threadWatchData();
	static void threadEntryForDownFile(void* arg);
	void threadDownFile();
	void LoadFileCurrent();
	void LoadFileInfo();
	CString GetPath(HTREEITEM hTree);
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	//1.查看磁盘分区
	//2.查看指定目录下的文件
	//3.打开文件
	//4.下载文件
	//5.鼠标操作
	//6.请求获得服务端屏幕截图
	//7.锁机
	//8.解锁
	//9.删除文件
	//1981. 测试连接
	//返回值是命令号, 若小于0则表示错误
	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = nullptr, size_t nLength = 0);


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
	afx_msg LRESULT OnSendPacket(WPARAM wParam, LPARAM lParam);//第二步:声明自定义消息函数

	afx_msg void OnBnClickedBtnStartWatch();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
