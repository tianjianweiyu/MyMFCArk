// CDriverDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyMFCArk.h"
#include "CDriverDlg.h"
#include "afxdialogex.h"
#include "CService.h"


// CDriverDlg 对话框

IMPLEMENT_DYNAMIC(CDriverDlg, CDialogEx)

CDriverDlg::CDriverDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DRIVER, pParent)
{

}

CDriverDlg::~CDriverDlg()
{
}

void CDriverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CDriverDlg, CDialogEx)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CDriverDlg::OnNMRClickList1)
	ON_COMMAND(ID_32771, &CDriverDlg::OnUpDriverInfo)
	ON_COMMAND(ID_32772, &CDriverDlg::OnHideDriverInfo)
END_MESSAGE_MAP()


// CDriverDlg 消息处理程序


BOOL CDriverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_Menu.LoadMenu(IDR_MENU1);

	CRect rect;
	m_list.GetClientRect(rect);

	//设置列表控件的样式
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//给列表添加列
	m_list.InsertColumn(0, L"驱动名", 0, rect.right / 4);
	m_list.InsertColumn(1, L"基址", 0, rect.right / 4);
	m_list.InsertColumn(2, L"大小", 0, rect.right / 4);
	m_list.InsertColumn(3, L"驱动路径", 0, rect.right / 4);

	CService* myArkService = CService::GetService();
	//获取驱动数量
	ULONG nCount = myArkService->GetDriverCount();
	//根据返回的数量申请空间
	PVOID DriverInfo = malloc(sizeof(DRIVERINFO)*nCount);
	//遍历驱动
	myArkService->EnumDriver(DriverInfo, nCount);

	PDRIVERINFO buffTemp = (PDRIVERINFO)DriverInfo;

	int j = 0;
	//插入信息到List
	for (int i = 0; i < nCount; i++, j++)
	{
		if (buffTemp[i].SizeOfImage == 0)
		{
			j--;
			continue;
		}

		CString buffer;
		m_list.InsertItem(j, _T(""));
		//驱动名
		m_list.SetItemText(j, 0, buffTemp[i].wcDriverBasePath);
		//格式化基址
		buffer.Format(_T("0x%08X"), buffTemp[i].DllBase);
		m_list.SetItemText(j, 1, buffer);
		//格式化大小
		buffer.Format(_T("0x%08X"), buffTemp[i].SizeOfImage);
		m_list.SetItemText(j, 2, buffer);
		//驱动路径
		m_list.SetItemText(j, 3, buffTemp[i].wcDriverFullPath);
	}
	//释放空间
	free(DriverInfo);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

//右键弹出菜单
void CDriverDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
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


void CDriverDlg::OnUpDriverInfo()
{
	// TODO: 在此添加命令处理程序代码

	CService* myArkService = CService::GetService();
	//获取驱动数量
	ULONG nCount = myArkService->GetDriverCount();
	//根据返回的数量申请空间
	PVOID DriverInfo = malloc(sizeof(DRIVERINFO)*nCount);
	//遍历驱动
	myArkService->EnumDriver(DriverInfo, nCount);

	PDRIVERINFO buffTemp = (PDRIVERINFO)DriverInfo;

	//清空列表
	m_list.DeleteAllItems();

	int j = 0;
	//插入信息到List
	for (int i = 0; i < nCount; i++, j++)
	{
		if (buffTemp[i].SizeOfImage == 0)
		{
			j--;
			continue;
		}

		CString buffer;
		m_list.InsertItem(j, _T(""));
		//驱动名
		m_list.SetItemText(j, 0, buffTemp[i].wcDriverBasePath);
		//格式化基址
		buffer.Format(_T("0x%08X"), buffTemp[i].DllBase);
		m_list.SetItemText(j, 1, buffer);
		//格式化大小
		buffer.Format(_T("0x%08X"), buffTemp[i].SizeOfImage);
		m_list.SetItemText(j, 2, buffer);
		//驱动路径
		m_list.SetItemText(j, 3, buffTemp[i].wcDriverFullPath);
	}
	//释放空间
	free(DriverInfo);
}


void CDriverDlg::OnHideDriverInfo()
{
	// TODO: 在此添加命令处理程序代码

	//获取要隐藏的驱动名
	DWORD nNow = m_list.GetSelectionMark();
	CString BaseName = m_list.GetItemText(nNow, 0);

	//向0环发送隐藏驱动请求
	CService* myArkService = CService::GetService();
	myArkService->HideDriverInfo(BaseName.GetBuffer(0));

	//刷新
	OnUpDriverInfo();
}
