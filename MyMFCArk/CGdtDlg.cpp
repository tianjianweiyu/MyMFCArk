// CGdtDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyMFCArk.h"
#include "CGdtDlg.h"
#include "afxdialogex.h"
#include "CService.h"


// CGdtDlg 对话框

IMPLEMENT_DYNAMIC(CGdtDlg, CDialogEx)

CGdtDlg::CGdtDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GDT, pParent)
{

}

CGdtDlg::~CGdtDlg()
{
}

void CGdtDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CGdtDlg, CDialogEx)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CGdtDlg::OnNMRClickList1)
	ON_COMMAND(ID_GDT_32782, &CGdtDlg::OnUpGdtInfo)
END_MESSAGE_MAP()


// CGdtDlg 消息处理程序


BOOL CGdtDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_Menu.LoadMenu(IDR_MENU6);

	CRect rect;
	m_list.GetClientRect(rect);

	//设置列表控件的样式
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//给列表添加列
	m_list.InsertColumn(0, L"段基址", 0, rect.right / 5);
	m_list.InsertColumn(1, L"段限长", 0, rect.right / 5);
	m_list.InsertColumn(2, L"段粒度", 0, rect.right / 5);
	m_list.InsertColumn(3, L"特权级", 0, rect.right / 5);
	m_list.InsertColumn(4, L"类型", 0, rect.right / 5);

	CService* myArkService = CService::GetService();
	//获取GDT数量
	ULONG nCount = myArkService->GetGdtCount();
	//根据返回的大小申请空间
	PVOID pGdtInfo = malloc(sizeof(GDTINFO)*nCount);
	//遍历GDT
	myArkService->EnumGdt(pGdtInfo, nCount);

	PGDTINFO buffTemp = (PGDTINFO)pGdtInfo;

	for (int i = 0; i < nCount; i++)
	{
		CString buffer;
		m_list.InsertItem(i, _T(""));
		//段基址
		buffer.Format(_T("0x%08X"), buffTemp[i].BaseAddr);
		m_list.SetItemText(i, 0, buffer);
		//段限长
		buffer.Format(_T("0x%08X"), buffTemp[i].Limit);
		m_list.SetItemText(i, 1, buffer);
		//段粒度
		buffer.Format(_T("%d"), buffTemp[i].Grain);
		m_list.SetItemText(i, 2, buffer);
		//特权级
		buffer.Format(_T("%d"), buffTemp[i].Dpl);
		m_list.SetItemText(i, 3, buffer);
		//类型
		buffer.Format(_T("%d"), buffTemp[i].GateType);
		m_list.SetItemText(i, 4, buffer);

	}

	//释放空间
	free(pGdtInfo);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CGdtDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
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


void CGdtDlg::OnUpGdtInfo()
{
	// TODO: 在此添加命令处理程序代码

	CService* myArkService = CService::GetService();
	//获取GDT数量
	ULONG nCount = myArkService->GetGdtCount();
	//根据返回的大小申请空间
	PVOID pGdtInfo = malloc(sizeof(GDTINFO)*nCount);
	//遍历GDT
	myArkService->EnumGdt(pGdtInfo, nCount);

	PGDTINFO buffTemp = (PGDTINFO)pGdtInfo;

	m_list.DeleteAllItems();

	for (int i = 0; i < nCount; i++)
	{
		CString buffer;
		m_list.InsertItem(i, _T(""));
		//段基址
		buffer.Format(_T("0x%08X"), buffTemp[i].BaseAddr);
		m_list.SetItemText(i, 0, buffer);
		//段限长
		buffer.Format(_T("0x%08X"), buffTemp[i].Limit);
		m_list.SetItemText(i, 1, buffer);
		//段粒度
		buffer.Format(_T("%d"), buffTemp[i].Grain);
		m_list.SetItemText(i, 2, buffer);
		//特权级
		buffer.Format(_T("%d"), buffTemp[i].Dpl);
		m_list.SetItemText(i, 3, buffer);
		//类型
		buffer.Format(_T("%d"), buffTemp[i].GateType);
		m_list.SetItemText(i, 4, buffer);

	}

	//释放空间
	free(pGdtInfo);
}
