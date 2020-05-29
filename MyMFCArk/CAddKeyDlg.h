#pragma once


// CAddKeyDlg 对话框

class CAddKeyDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAddKeyDlg)

public:
	CAddKeyDlg(CString KeyName,CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CAddKeyDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ADDKEY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_edit_str;
	CString m_KeyName;
	afx_msg void OnBnClickedOk();
};
