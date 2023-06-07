#pragma once
#include "afxdialogex.h"
#ifndef WM_SEND_PACK_ACK
#define WM_SEND_PACK_ACK (WM_USER + 100)
#endif


// CWatchDialog 对话框

class CWatchDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CWatchDialog)

public:
	CWatchDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWatchDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_WATCH };
#endif
public:
	int m_nObjWidth;
	int m_nObjHeight;
	CImage m_image;//缓存(采用一张图片来用作缓存区域)
	bool m_isFull;//缓存是否有数据
	bool m_isClosed;//监视是否关闭
//public:
//	CImage& GetImage()
//	{
//		return m_image;
//	}
//	bool isFull()
//	{
//		return m_isFull;
//	}
//	void SetImageStatus(bool isFull = false)
//	{
//		m_isFull = isFull;
//	}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CPoint UserPointtoRemotePoint(CPoint& point, bool isScreen = false);//客户端鼠标转换为服务端鼠标
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_picture;
	afx_msg LRESULT OnSendPacketAck(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnStnClickedWatch();
	virtual void OnOK();
	afx_msg void OnBnClickedBtnLock();
	afx_msg void OnBnClickedBtnUnlock();
};
