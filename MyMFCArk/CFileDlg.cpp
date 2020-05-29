// CFileDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyMFCArk.h"
#include "CFileDlg.h"
#include "afxdialogex.h"
#include "CService.h"
#include <strsafe.h>


// CFileDlg 对话框

IMPLEMENT_DYNAMIC(CFileDlg, CDialogEx)

CFileDlg::CFileDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILE, pParent)
{

}

CFileDlg::~CFileDlg()
{
}

void CFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_tree);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CFileDlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_TREE1, &CFileDlg::OnNMClickTree1)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CFileDlg::OnNMRClickList1)
	ON_COMMAND(ID_32778, &CFileDlg::OnDeleteFile)
END_MESSAGE_MAP()


// CFileDlg 消息处理程序


BOOL CFileDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_Menu.LoadMenu(IDR_MENU3);

	CRect rect;
	m_list.GetClientRect(rect);

	//设置列表控件的样式
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//给列表添加列
	m_list.InsertColumn(0, L"文件名", 0, rect.right / 4);
	m_list.InsertColumn(1, L"大小", 0, rect.right / 4);
	m_list.InsertColumn(2, L"创建时间", 0, rect.right / 4);
	m_list.InsertColumn(3, L"修改时间", 0, rect.right / 4);

	//初始化数控件
	HTREEITEM hItem = m_tree.InsertItem(L"我的电脑", NULL);
	//设定指定项的数据
	wchar_t* pBuff = _wcsdup(L"我的电脑");
	m_tree.SetItemData(hItem, (DWORD_PTR)pBuff);

	WCHAR szDriver[6] = L"A:";
	CString temp;

	CService* myArkService = CService::GetService();

	for (szDriver[0] = 'A'; szDriver[0] <= 'Z'; szDriver[0]++)
	{
		if (DRIVE_FIXED == GetDriveType(szDriver) ||
			DRIVE_CDROM == GetDriveType(szDriver) ||
			DRIVE_REMOVABLE == GetDriveType(szDriver) ||
			DRIVE_REMOTE == GetDriveType(szDriver))
		{
			switch (GetDriveType(szDriver))
			{
			case DRIVE_FIXED:
			{
				temp.Format(L"%s(%s)", L"本地磁盘", szDriver);
				break;
			}
			case DRIVE_CDROM:
			{
				temp.Format(L"%s(%s)", L"光驱", szDriver);
				break;
			}
			case DRIVE_REMOVABLE:
			{
				temp.Format(L"%s(%s)", L"可移动磁盘", szDriver);
				break;
			}
			case DRIVE_REMOTE:
			{
				temp.Format(L"%s(%s)", L"网络驱动器", szDriver);
				break;
			}
			}

			HTREEITEM hDriver = m_tree.InsertItem(temp, hItem);
			//设定指定项的数据为路径
			wchar_t* pBuff = _wcsdup(szDriver);
			m_tree.SetItemData(hDriver, (DWORD_PTR)pBuff);

			if (DRIVE_CDROM != GetDriveType(szDriver))
			{
				//获取目录下文件的数量
				ULONG nCount = myArkService->GetFileCount(szDriver, 6);
				//根据返回的数量申请空间
				PVOID pFileInfo = malloc(sizeof(FILEINFO)*nCount);
				//遍历文件
				myArkService->EnumFile(szDriver, 6, pFileInfo, nCount);

				PFILEINFO buffTemp = (PFILEINFO)pFileInfo;

				//插入信息到Tree
				for (int i = 0; i < nCount; i++)
				{
					if (!buffTemp[i].FileOrDirectory)
					{
						HTREEITEM hFileDir1 = m_tree.InsertItem(buffTemp[i].wcFileName, hDriver);
						//拼接路径
						WCHAR szFullPath[MAX_PATH] = { 0 };
						StringCbPrintf(szFullPath, MAX_PATH, L"%s\\%s", szDriver, buffTemp[i].wcFileName);
						//设定指定项的数据
						wchar_t* pBuff = _wcsdup(szFullPath);
						m_tree.SetItemData(hFileDir1, (DWORD_PTR)pBuff);
					}
				}

				//释放空间
				free(pFileInfo);
			}

		}

	}


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

//左键单击树控件，+/-按钮点击响应
void CFileDlg::OnNMClickTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码

	DWORD dwChild = 0;		//子节点标志，为1表示有子节点

	//获取选中的树控件的项
	CPoint pos = { 0 };
	GetCursorPos(&pos);
	ScreenToClient(&pos);
	HTREEITEM hItem = m_tree.HitTest(pos);
	if (!hItem)
	{
		return;
	}

	//判断是否有子节点
	HTREEITEM hChild = m_tree.GetNextItem(hItem, TVGN_CHILD);
	if (hChild)
	{
		dwChild = 1;
	}

	//获取选中的数控件的项的数据（之前设置为对应文件的路径）
	CString Path = (WCHAR*)m_tree.GetItemData(hItem);
	if (Path.IsEmpty() ||
		!Path.Compare(L"我的电脑") ||
		!Path.Compare(L"D:"))
	{
		return;
	}

	m_Dir = Path;

	//清空list列表
	m_list.DeleteAllItems();

	CService* myArkService = CService::GetService();
	//获取目录下文件的数量,GetLength()获取的是字符数，不是字节数,
	ULONG nCount = myArkService->GetFileCount(Path.GetBuffer(), (Path.GetLength() + 1) * sizeof(TCHAR));
	//根据返回的数量申请空间
	PVOID pFileInfo = malloc(sizeof(FILEINFO)*nCount);
	//遍历文件,GetLength()获取的是字符数，不是字节数
	myArkService->EnumFile(Path.GetBuffer(), (Path.GetLength() + 1) * sizeof(TCHAR), pFileInfo, nCount);

	PFILEINFO buffTemp = (PFILEINFO)pFileInfo;

	int nIndex = 0;
	//插入信息到Tree
	for (int i = 0; i < nCount; i++)
	{
		//目录,并且没有子节点
		if (buffTemp[i].FileOrDirectory == 0 &&
			!dwChild &&
			wcscmp(buffTemp[i].wcFileName, L".") &&
			wcscmp(buffTemp[i].wcFileName, L".."))
		{

			HTREEITEM hFileDir2 = m_tree.InsertItem(buffTemp[i].wcFileName, hItem);
			//拼接路径
			WCHAR szFullPath[MAX_PATH] = { 0 };
			StringCbPrintf(szFullPath, MAX_PATH, L"%s\\%s", Path, buffTemp[i].wcFileName);
			//设定指定项的数据
			wchar_t* pBuff = _wcsdup(szFullPath);
			m_tree.SetItemData(hFileDir2, (DWORD_PTR)pBuff);
		}
		//文件
		else if (buffTemp[i].FileOrDirectory == 1)
		{
			CString buffer;
			m_list.InsertItem(nIndex, _T(""));

			//文件名
			m_list.SetItemText(nIndex, 0, buffTemp[i].wcFileName);
			//大小
			buffer.Format(_T("%d"), buffTemp[i].Size);
			m_list.SetItemText(nIndex, 1, buffer);
			//创建时间
			FILETIME ft = { 0 };
			SYSTEMTIME st = { 0 };
			FileTimeToLocalFileTime((PFILETIME)&buffTemp[i].CreateTime, &ft);
			FileTimeToSystemTime(&ft, &st);
			buffer.Format(L"%4d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			m_list.SetItemText(nIndex, 2, buffer);
			//修改时间
			FileTimeToLocalFileTime((PFILETIME)&buffTemp[i].ChangeTime, &ft);
			FileTimeToSystemTime(&ft, &st);
			buffer.Format(L"%4d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			m_list.SetItemText(nIndex, 3, buffer);

			nIndex++;
		}
	}

	//释放空间
	free(pFileInfo);

	*pResult = 0;
}

//右键单击列表控件，弹出菜单
void CFileDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
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


void CFileDlg::OnDeleteFile()
{
	// TODO: 在此添加命令处理程序代码

	//获取到列表名字
	DWORD nNow = m_list.GetSelectionMark();
	CString BaseName = m_list.GetItemText(nNow, 0);
	//拼接
	WCHAR szFullPath[MAX_PATH] = { 0 };
	StringCbPrintf(szFullPath, MAX_PATH, L"%s\\%s", m_Dir, BaseName);

	CService* myArkService = CService::GetService();
	//向0环发送删除文件请求
	myArkService->MyDeleteFile(szFullPath, wcslen(szFullPath)*2+2);
}
