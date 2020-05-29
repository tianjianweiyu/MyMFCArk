// CThreadDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyMFCArk.h"
#include "CThreadDlg.h"
#include "afxdialogex.h"
#include "CService.h"


// CThreadDlg 对话框

IMPLEMENT_DYNAMIC(CThreadDlg, CDialogEx)

CThreadDlg::CThreadDlg(ULONG pEprocess, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_THREAD, pParent), m_pEprocess(pEprocess)
{

}

CThreadDlg::~CThreadDlg()
{
}

void CThreadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CThreadDlg, CDialogEx)
END_MESSAGE_MAP()


// CThreadDlg 消息处理程序


BOOL CThreadDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CRect rect;
	m_list.GetClientRect(rect);

	//设置列表控件的样式
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//给列表添加列
	m_list.InsertColumn(0, L"线程ID", 0, rect.right / 5);
	m_list.InsertColumn(1, L"ETHREAD", 0, rect.right / 5);
	m_list.InsertColumn(2, L"Teb", 0, rect.right / 5);
	m_list.InsertColumn(3, L"优先级", 0, rect.right / 5);
	m_list.InsertColumn(4, L"切换次数", 0, rect.right / 5);

	CService* myArkService = CService::GetService();
	//获取进程线程数量
	ULONG nCount = myArkService->GetThreadCount(&m_pEprocess);
	//根据返回的数量申请空间
	PVOID ThreadInfo = malloc(sizeof(THREADINFO)*nCount);
	//遍历进程线程
	myArkService->EnumThread(&m_pEprocess, ThreadInfo, nCount);

	PTHREADINFO buffTemp = (PTHREADINFO)ThreadInfo;

	int j = 0;
	//插入信息到List
	for (int i = 0; i < nCount; i++, j++)
	{
		ULONG temp = (ULONG)(buffTemp[i].Tid) & 0x80000000;
		if (temp == 0x80000000)
		{
			j--;
			continue;
		}
		CString buffer;
		m_list.InsertItem(j, _T(""));
		//线程ID
		buffer.Format(_T("%d"), buffTemp[i].Tid);
		m_list.SetItemText(j, 0, buffer);
		//线程执行块地址
		buffer.Format(_T("0x%08X"), buffTemp[i].pEthread);
		m_list.SetItemText(j, 1, buffer);
		//Teb结构地址
		buffer.Format(_T("0x%08X"), buffTemp[i].pTeb);
		m_list.SetItemText(j, 2, buffer);
		//静态优先级
		buffer.Format(_T("%d"), buffTemp[i].BasePriority);
		m_list.SetItemText(j, 3, buffer);
		//切换次数
		buffer.Format(_T("%d"), buffTemp[i].ContextSwitches);
		m_list.SetItemText(j, 4, buffer);
	}

	//释放空间
	free(ThreadInfo);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
