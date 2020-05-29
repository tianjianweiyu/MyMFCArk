// CModuleDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyMFCArk.h"
#include "CModuleDlg.h"
#include "afxdialogex.h"
#include "CService.h"


// CModuleDlg 对话框

IMPLEMENT_DYNAMIC(CModuleDlg, CDialogEx)

CModuleDlg::CModuleDlg(ULONG pEprocess, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MODULE, pParent), m_pEprocess(pEprocess)
{

}

CModuleDlg::~CModuleDlg()
{
}

void CModuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CModuleDlg, CDialogEx)
END_MESSAGE_MAP()


// CModuleDlg 消息处理程序


BOOL CModuleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CRect rect;
	m_list.GetClientRect(rect);

	//设置列表控件的样式
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//给列表添加列
	m_list.InsertColumn(0, L"模块路径", 0, rect.right / 3);
	m_list.InsertColumn(1, L"基地址", 0, rect.right / 3);
	m_list.InsertColumn(2, L"大小", 0, rect.right / 3);

	CService* myArkService = CService::GetService();
	//获取进程模块数量
	ULONG nCount = myArkService->GetModuleCount(&m_pEprocess);
	//根据返回的数量申请空间
	PVOID ModuleInfo = malloc(sizeof(MODULEINFO)*nCount);
	//遍历进程模块
	myArkService->EnumModule(&m_pEprocess, ModuleInfo, nCount);

	PMODULEINFO buffTemp = (PMODULEINFO)ModuleInfo;

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
		//模块路径
		m_list.SetItemText(j, 0, buffTemp[i].wcModuleFullPath);
		//基地址
		buffer.Format(_T("0x%08X"), buffTemp[i].DllBase);
		m_list.SetItemText(j, 1, buffer);
		//大小
		buffer.Format(_T("0x%08X"), buffTemp[i].SizeOfImage);
		m_list.SetItemText(j, 2, buffer);
	}

	//释放空间
	free(ModuleInfo);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
