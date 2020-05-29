#pragma once


// CGdtDlg 对话框

class CGdtDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGdtDlg)

public:
	CGdtDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CGdtDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GDT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list;
	CMenu m_Menu;

	virtual BOOL OnInitDialog();

	afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnUpGdtInfo();
};
