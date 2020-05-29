// CAddKeyDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyMFCArk.h"
#include "CAddKeyDlg.h"
#include "afxdialogex.h"
#include "CService.h"


// CAddKeyDlg 对话框

IMPLEMENT_DYNAMIC(CAddKeyDlg, CDialogEx)

CAddKeyDlg::CAddKeyDlg(CString KeyName,CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ADDKEY, pParent)
	, m_edit_str(_T(""))
	,m_KeyName(KeyName)
{

}

CAddKeyDlg::~CAddKeyDlg()
{
}

void CAddKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_edit_str);
}


BEGIN_MESSAGE_MAP(CAddKeyDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAddKeyDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAddKeyDlg 消息处理程序


void CAddKeyDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码

	UpdateData(TRUE);

	CString LastPath;
	LastPath = m_KeyName + L"\\" + m_edit_str + L"\\";

	CService* myArkService = CService::GetService();

	//遍历注册表
	myArkService->NewKey(LastPath.GetBuffer(), (LastPath.GetLength() + 1) * 2);

	CDialogEx::OnOK();
}
