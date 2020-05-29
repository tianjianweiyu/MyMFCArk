
// MyMFCArkDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MyMFCArk.h"
#include "MyMFCArkDlg.h"
#include "afxdialogex.h"
#include "CDriverDlg.h"
#include "CProcessDlg.h"
#include "CService.h"
#include "CFileDlg.h"
#include "CRegisterDlg.h"
#include "CIdtDlg.h"
#include "CGdtDlg.h"
#include "CSsdtDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMyMFCArkDlg 对话框



CMyMFCArkDlg::CMyMFCArkDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MYMFCARK_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CMyMFCArkDlg::~CMyMFCArkDlg()
{
	CService* myArkService = CService::GetService();
	myArkService->CloseMyService();
}

void CMyMFCArkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tab);
}

BEGIN_MESSAGE_MAP(CMyMFCArkDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CMyMFCArkDlg::OnTcnSelchangeTab1)
END_MESSAGE_MAP()


// CMyMFCArkDlg 消息处理程序

BOOL CMyMFCArkDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	// 获取程序自身的PID
	ULONG PID = _getpid();

	CService* myArkService = CService::GetService();
	//发送自身PID到0环
	myArkService->SendSelfPid(&PID);


	//给Tab控件添加选项卡
	AddTabWnd(L"驱动信息", new CDriverDlg, IDD_DRIVER);
	AddTabWnd(L"进程信息", new CProcessDlg, IDD_PROCESS);
	AddTabWnd(L"文件信息", new CFileDlg, IDD_FILE);
	AddTabWnd(L"注册表信息", new CRegisterDlg, IDD_REGISTER);
	AddTabWnd(L"IDT", new CIdtDlg, IDD_IDT);
	AddTabWnd(L"GDT", new CGdtDlg, IDD_GDT);
	AddTabWnd(L"SSDT", new CSsdtDlg, IDD_SSDT);

	//将选项卡对应对话框显示出来
	m_tab.SetCurSel(0);
	OnTcnSelchangeTab1(0, 0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMyMFCArkDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMyMFCArkDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMyMFCArkDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMyMFCArkDlg::AddTabWnd(const CString& title, CDialogEx* pSubWnd, UINT uId)
{
	//GetItemCount()获取当前选项卡控件中的选项卡的数量
	//选项卡是从零开始索引的
	//在选项卡尾端插入新的选项卡
	m_tab.InsertItem(m_tab.GetItemCount(), title);
	//创建子窗口，设置父窗口
	pSubWnd->Create(uId, &m_tab);

	CRect rect;
	//获取选项卡控件客户区的大小
	m_tab.GetClientRect(rect);
	//根据控件客户区大小设置对应对话框的位置
	rect.DeflateRect(1, 23, 1, 1);
	//更改选项卡对应的对话框大小
	//并将其移动到当前选项卡控件客户区
	pSubWnd->MoveWindow(rect);


	//将要添加的对话框从尾部放入对话框数组
	m_tabSubWnd.push_back(pSubWnd);
	//将新插入的选项卡变为选中状态
	m_tab.SetCurSel(m_tabSubWnd.size() - 1);
}

void CMyMFCArkDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码

	//循环将选项卡每项对应的对话框都隐藏
	for (auto&i : m_tabSubWnd)
	{
		i->ShowWindow(SW_HIDE);
	}

	//将当前选项卡选项对应的对话框显示出来
	m_tabSubWnd[m_tab.GetCurSel()]->ShowWindow(SW_SHOW);
	//将当前选项卡选项对应的对话框内容更新
	m_tabSubWnd[m_tab.GetCurSel()]->UpdateWindow();
}
