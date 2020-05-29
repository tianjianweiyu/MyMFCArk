// CIdtDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyMFCArk.h"
#include "CIdtDlg.h"
#include "afxdialogex.h"
#include "CService.h"


// CIdtDlg 对话框

IMPLEMENT_DYNAMIC(CIdtDlg, CDialogEx)

CIdtDlg::CIdtDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_IDT, pParent)
{

}

CIdtDlg::~CIdtDlg()
{
}

void CIdtDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CIdtDlg, CDialogEx)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CIdtDlg::OnNMRClickList1)
	ON_COMMAND(ID_IDT_32781, &CIdtDlg::OnUpIdtInfo)
END_MESSAGE_MAP()


// CIdtDlg 消息处理程序


BOOL CIdtDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_Menu.LoadMenu(IDR_MENU5);

	CRect rect;
	m_list.GetClientRect(rect);

	//设置列表控件的样式
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//给列表添加列
	m_list.InsertColumn(0, L"中断号", 0, rect.right / 6);
	m_list.InsertColumn(1, L"段选择子", 0, rect.right / 6);
	m_list.InsertColumn(2, L"处理函数的地址", 0, rect.right / 6);
	m_list.InsertColumn(3, L"参数个数", 0, rect.right / 6);
	m_list.InsertColumn(4, L"特权级", 0, rect.right / 6);
	m_list.InsertColumn(5, L"类型", 0, rect.right / 6);

	CService* myArkService = CService::GetService();
	//申请空间
	PVOID pIdtInfo = malloc(sizeof(IDTINFO) * 0x100);
	//遍历IDT
	myArkService->EnumIdt(pIdtInfo);

	PIDTINFO pbuffTemp = (PIDTINFO)pIdtInfo;

	for (int i = 0; i < 0x100; i++)
	{
		CString buffer;
		m_list.InsertItem(i, _T(""));
		//中断号
		buffer.Format(_T("%d"), i);
		m_list.SetItemText(i, 0, buffer);
		//段选择子
		buffer.Format(_T("%d"), pbuffTemp[i].Selector);
		m_list.SetItemText(i, 1, buffer);
		//处理函数的地址
		buffer.Format(_T("0x%08X"), pbuffTemp[i].pFunction);
		m_list.SetItemText(i, 2, buffer);
		//参数个数
		buffer.Format(_T("%d"), pbuffTemp[i].ParamCount);
		m_list.SetItemText(i, 3, buffer);
		//特权级
		buffer.Format(_T("%d"), pbuffTemp[i].Dpl);
		m_list.SetItemText(i, 4, buffer);
		//类型
		buffer.Format(_T("%d"), pbuffTemp[i].GateType);
		m_list.SetItemText(i, 5, buffer);
	}

	//释放空间
	free(pIdtInfo);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CIdtDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
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


void CIdtDlg::OnUpIdtInfo()
{
	// TODO: 在此添加命令处理程序代码

	CService* myArkService = CService::GetService();
	//申请空间
	PVOID pIdtInfo = malloc(sizeof(IDTINFO) * 0x100);
	//遍历IDT
	myArkService->EnumIdt(pIdtInfo);

	PIDTINFO pbuffTemp = (PIDTINFO)pIdtInfo;

	//清空列表
	m_list.DeleteAllItems();

	for (int i = 0; i < 0x100; i++)
	{
		CString buffer;
		m_list.InsertItem(i, _T(""));
		//中断号
		buffer.Format(_T("%d"), i);
		m_list.SetItemText(i, 0, buffer);
		//段选择子
		buffer.Format(_T("%d"), pbuffTemp[i].Selector);
		m_list.SetItemText(i, 1, buffer);
		//处理函数的地址
		buffer.Format(_T("0x%08X"), pbuffTemp[i].pFunction);
		m_list.SetItemText(i, 2, buffer);
		//参数个数
		buffer.Format(_T("%d"), pbuffTemp[i].ParamCount);
		m_list.SetItemText(i, 3, buffer);
		//特权级
		buffer.Format(_T("%d"), pbuffTemp[i].Dpl);
		m_list.SetItemText(i, 4, buffer);
		//类型
		buffer.Format(_T("%d"), pbuffTemp[i].GateType);
		m_list.SetItemText(i, 5, buffer);
	}

	//释放空间
	free(pIdtInfo);
}
