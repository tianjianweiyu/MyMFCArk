// CProcessDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyMFCArk.h"
#include "CProcessDlg.h"
#include "afxdialogex.h"
#include "CService.h"
#include "CModuleDlg.h"
#include "CThreadDlg.h"


// CProcessDlg 对话框

IMPLEMENT_DYNAMIC(CProcessDlg, CDialogEx)

CProcessDlg::CProcessDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PROCESS, pParent)
{

}

CProcessDlg::~CProcessDlg()
{
}

void CProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CProcessDlg, CDialogEx)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CProcessDlg::OnNMRClickList1)
	ON_COMMAND(ID_32773, &CProcessDlg::OnHideProcess)
	ON_COMMAND(ID_32774, &CProcessDlg::OnKillProcess)
	ON_COMMAND(ID_32775, &CProcessDlg::OnCheckModule)
	ON_COMMAND(ID_32776, &CProcessDlg::OnCheckThread)
	ON_COMMAND(ID_32777, &CProcessDlg::OnUpProcess)
END_MESSAGE_MAP()


// CProcessDlg 消息处理程序


BOOL CProcessDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_Menu.LoadMenu(IDR_MENU2);

	CRect rect;
	m_list.GetClientRect(rect);

	//设置列表控件的样式
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//给列表添加列
	m_list.InsertColumn(0, L"进程名", 0, rect.right / 5);
	m_list.InsertColumn(1, L"进程ID", 0, rect.right / 5);
	m_list.InsertColumn(2, L"父进程ID", 0, rect.right / 5);
	m_list.InsertColumn(3, L"EPROCESS", 0, rect.right / 5);
	m_list.InsertColumn(4, L"映像路径", 0, rect.right / 5);

	CService* myArkService = CService::GetService();
	//获取进程数量
	ULONG nCount = myArkService->GetProcessCount();
	//根据返回的数量申请空间
	PVOID ProcessInfo = malloc(sizeof(PROCESSINFO)*nCount);
	//遍历进程
	myArkService->EnumProcess(ProcessInfo, nCount);

	PPROCESSINFO buffTemp = (PPROCESSINFO)ProcessInfo;

	int j = 0;
	//插入信息到List
	for (int i = 0; i < nCount; i++, j++)
	{
		if (buffTemp[i].Pid == 0)
		{
			j--;
			continue;
		}

		CString buffer;
		m_list.InsertItem(j, _T(""));

		//保存进程路径
		WCHAR tempBuffer[MAX_PATH] = { 0 };
		wcscpy_s(tempBuffer, MAX_PATH, buffTemp[i].wcProcessFullPath);
		//获取进程名
		PathStripPath(buffTemp[i].wcProcessFullPath);
		m_list.SetItemText(j, 0, buffTemp[i].wcProcessFullPath);
		//格式化进程ID
		buffer.Format(_T("%d"), buffTemp[i].Pid);
		m_list.SetItemText(j, 1, buffer);
		//格式化父进程ID
		buffer.Format(_T("%d"), buffTemp[i].PPid);
		m_list.SetItemText(j, 2, buffer);
		//格式化EPROCESS
		buffer.Format(_T("0x%08X"), buffTemp[i].pEproce);
		m_list.SetItemText(j, 3, buffer);
		//进程路径
		m_list.SetItemText(j, 4, tempBuffer);

	}
	//释放空间
	free(ProcessInfo);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

//右键List列表控件 ，弹出菜单
void CProcessDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
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


void CProcessDlg::OnHideProcess()
{
	// TODO: 在此添加命令处理程序代码

	//获取要隐藏的进程ID
	DWORD nNow = m_list.GetSelectionMark();
	CString BaseName = m_list.GetItemText(nNow, 1);
	ULONG ProcessId = _tcstoul(BaseName, 0, 10);

	//向0环发送隐藏进程请求
	CService* myArkService = CService::GetService();
	myArkService->HideProcessInfo(&ProcessId);

	//刷新
	OnUpProcess();
}


void CProcessDlg::OnKillProcess()
{
	// TODO: 在此添加命令处理程序代码

	//获取要隐藏的进程ID
	DWORD nNow = m_list.GetSelectionMark();
	CString BaseName = m_list.GetItemText(nNow, 1);
	ULONG ProcessId = _tcstoul(BaseName, 0, 10);

	//向0环发送结束进程请求
	CService* myArkService = CService::GetService();
	myArkService->KillProcess(&ProcessId);

	//等待一下下
	Sleep(100);

	//刷新
	OnUpProcess();
}


void CProcessDlg::OnCheckModule()
{
	// TODO: 在此添加命令处理程序代码

	//获取要查看模块的进程EPROCESS的地址
	DWORD nNow = m_list.GetSelectionMark();
	CString BaseName = m_list.GetItemText(nNow, 3);
	ULONG pEprocess = _tcstoul(BaseName, 0, 16);

	CModuleDlg moduleDlg(pEprocess);
	moduleDlg.DoModal();
}


void CProcessDlg::OnCheckThread()
{
	// TODO: 在此添加命令处理程序代码

	//获取要查看线程的进程EPROCESS的地址
	DWORD nNow = m_list.GetSelectionMark();
	CString BaseName = m_list.GetItemText(nNow, 3);
	ULONG pEprocess = _tcstoul(BaseName, 0, 16);

	CThreadDlg threadDlg(pEprocess);
	threadDlg.DoModal();
}


void CProcessDlg::OnUpProcess()
{
	// TODO: 在此添加命令处理程序代码

	CService* myArkService = CService::GetService();
	//获取进程数量
	ULONG nCount = myArkService->GetProcessCount();
	//根据返回的数量申请空间
	PVOID ProcessInfo = malloc(sizeof(PROCESSINFO)*nCount);
	//遍历进程
	myArkService->EnumProcess(ProcessInfo, nCount);

	PPROCESSINFO buffTemp = (PPROCESSINFO)ProcessInfo;

	//清空列表
	m_list.DeleteAllItems();

	int j = 0;
	//插入信息到List
	for (int i = 0; i < nCount; i++, j++)
	{
		if (buffTemp[i].Pid == 0)
		{
			j--;
			continue;
		}

		CString buffer;
		m_list.InsertItem(j, _T(""));

		//保存进程路径
		WCHAR tempBuffer[MAX_PATH] = { 0 };
		wcscpy_s(tempBuffer, MAX_PATH, buffTemp[i].wcProcessFullPath);
		//获取进程名
		PathStripPath(buffTemp[i].wcProcessFullPath);
		m_list.SetItemText(j, 0, buffTemp[i].wcProcessFullPath);
		//格式化进程ID
		buffer.Format(_T("%d"), buffTemp[i].Pid);
		m_list.SetItemText(j, 1, buffer);
		//格式化父进程ID
		buffer.Format(_T("%d"), buffTemp[i].PPid);
		m_list.SetItemText(j, 2, buffer);
		//格式化EPROCESS
		buffer.Format(_T("0x%08X"), buffTemp[i].pEproce);
		m_list.SetItemText(j, 3, buffer);
		//进程路径
		m_list.SetItemText(j, 4, tempBuffer);

	}
	//释放空间
	free(ProcessInfo);
}
