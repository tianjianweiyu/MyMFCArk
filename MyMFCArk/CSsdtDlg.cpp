// CSsdtDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyMFCArk.h"
#include "CSsdtDlg.h"
#include "afxdialogex.h"
#include "CService.h"


// CSsdtDlg 对话框

IMPLEMENT_DYNAMIC(CSsdtDlg, CDialogEx)

CSsdtDlg::CSsdtDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SSDT, pParent)
{

}

CSsdtDlg::~CSsdtDlg()
{
}

void CSsdtDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CSsdtDlg, CDialogEx)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CSsdtDlg::OnNMRClickList1)
	ON_COMMAND(ID_SSDT_32783, &CSsdtDlg::OnUpSsdtInfo)
END_MESSAGE_MAP()


// CSsdtDlg 消息处理程序


BOOL CSsdtDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_Menu.LoadMenu(IDR_MENU7);

	CRect rect;
	m_list.GetClientRect(rect);

	//设置列表控件的样式
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//给列表添加列
	m_list.InsertColumn(0, L"回调号", 0, rect.right / 2);
	m_list.InsertColumn(1, L"函数地址", 0, rect.right / 2);

	CService* myArkService = CService::GetService();
	//获取SSDT数量
	INT nCount = myArkService->GetSsdtCount();
	//根据返回的大小申请空间
	PVOID pSsdtInfo = malloc(sizeof(SSDTINFO)*nCount);
	//遍历SSDT
	myArkService->EnumSsdt(pSsdtInfo, nCount);

	PSSDTINFO pbuffTemp = (PSSDTINFO)pSsdtInfo;

	for (int i = 0; i < nCount; i++)
	{
		CString buffer;
		m_list.InsertItem(i, _T(""));
		//回调号
		buffer.Format(_T("%d"), pbuffTemp[i].uIndex);
		m_list.SetItemText(i, 0, buffer);
		//函数地址
		buffer.Format(_T("0x%08X"), pbuffTemp[i].uFuntionAddr);
		m_list.SetItemText(i, 1, buffer);
	}

	//释放空间
	free(pSsdtInfo);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CSsdtDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	if (m_list.GetSelectionMark() != -1)
	{
		CMenu *nMenu = m_Menu.GetSubMenu(0);
		POINT pos;
		GetCursorPos(&pos);
		nMenu->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this);
	}
}


void CSsdtDlg::OnUpSsdtInfo()
{
	// TODO: 在此添加命令处理程序代码

	CService* myArkService = CService::GetService();
	//获取SSDT数量
	INT nCount = myArkService->GetSsdtCount();
	//根据返回的大小申请空间
	PVOID pSsdtInfo = malloc(sizeof(SSDTINFO)*nCount);
	//遍历SSDT
	myArkService->EnumSsdt(pSsdtInfo, nCount);

	PSSDTINFO pbuffTemp = (PSSDTINFO)pSsdtInfo;

	//清空列表
	m_list.DeleteAllItems();

	for (int i = 0; i < nCount; i++)
	{
		CString buffer;
		m_list.InsertItem(i, _T(""));
		//回调号
		buffer.Format(_T("%d"), pbuffTemp[i].uIndex);
		m_list.SetItemText(i, 0, buffer);
		//函数地址
		buffer.Format(_T("0x%08X"), pbuffTemp[i].uFuntionAddr);
		m_list.SetItemText(i, 1, buffer);
	}

	//释放空间
	free(pSsdtInfo);
}
