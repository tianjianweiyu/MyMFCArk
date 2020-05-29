
// MyMFCArkDlg.h: 头文件
//

#pragma once
#include <vector>

// CMyMFCArkDlg 对话框
class CMyMFCArkDlg : public CDialogEx
{
// 构造
public:
	CMyMFCArkDlg(CWnd* pParent = nullptr);	// 标准构造函数

	~CMyMFCArkDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MYMFCARK_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	/*!
	*  函 数 名： AddTabWnd
	*  日    期： 2020/05/21
	*  返回类型： void
	*  参    数： const CString & title 要添加的选项名
	*  参    数： CDialogEx * pSubWnd	要添加的选项对应的对话框类
	*  参    数： UINT uId				要添加的选项对应对话框的ID
	*  功    能： 给选项卡控件添加选项
	*/
	void AddTabWnd(const CString& title, CDialogEx* pSubWnd, UINT uId);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_tab;
	std::vector<CDialogEx*> m_tabSubWnd;	//对话框数组，一个对话框对应选项卡的一项
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
};
