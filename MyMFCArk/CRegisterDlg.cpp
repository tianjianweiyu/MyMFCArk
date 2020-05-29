// CRegisterDlg.cpp: 实现文件
//

#include "pch.h"
#include "MyMFCArk.h"
#include "CRegisterDlg.h"
#include "afxdialogex.h"
#include "CService.h"
#include <strsafe.h>
#include "CAddKeyDlg.h"


// CRegisterDlg 对话框

IMPLEMENT_DYNAMIC(CRegisterDlg, CDialogEx)

CRegisterDlg::CRegisterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REGISTER, pParent)
{

}

CRegisterDlg::~CRegisterDlg()
{
}

void CRegisterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_TREE1, m_tree);
}


BEGIN_MESSAGE_MAP(CRegisterDlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_TREE1, &CRegisterDlg::OnNMClickTree1)
	ON_NOTIFY(NM_RCLICK, IDC_TREE1, &CRegisterDlg::OnNMRClickTree1)
	ON_COMMAND(ID_32779, &CRegisterDlg::OnNew)
	ON_COMMAND(ID_32780, &CRegisterDlg::OnDelete)
END_MESSAGE_MAP()


// CRegisterDlg 消息处理程序


BOOL CRegisterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_Menu.LoadMenu(IDR_MENU4);

	CRect rect;
	m_list.GetClientRect(rect);

	//设置列表控件的样式
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//给列表添加列
	m_list.InsertColumn(0, L"名称", 0, rect.right / 3);
	m_list.InsertColumn(1, L"类型", 0, rect.right / 3);
	m_list.InsertColumn(2, L"数据", 0, rect.right / 3);

	//初始化数控件
	HTREEITEM hItem1 = m_tree.InsertItem(L"我的电脑", NULL);
	//设定指定项的数据
	CString Root = L"\\Registry";
	wchar_t* pBuff = _wcsdup(Root);
	m_tree.SetItemData(hItem1, (DWORD_PTR)pBuff);

	CService* myArkService = CService::GetService();
	//获取注册表键下子键的数量
	ULONG nCount = myArkService->GetRegisterChildCount(Root.GetBuffer(), (Root.GetLength() + 1) * 2);
	//根据返回的数量申请空间
	PVOID pRegInfo = malloc(sizeof(REGISTER)*nCount);
	//遍历注册表
	myArkService->EnumReg(Root.GetBuffer(), (Root.GetLength() + 1) * 2,pRegInfo,nCount);

	PREGISTER buffTemp = (PREGISTER)pRegInfo;

	//插入信息到Tree控件
	for (int i = 0; i < nCount; i++)
	{
		//子项
		if (buffTemp[i].Type == 0)
		{
			CString KeyName = buffTemp[i].KeyName;
			CString Path;
			if (KeyName == L"MACHINE")
			{
				Path = L"\\Registry\\Machine";
				KeyName.Format(L"%s", L"HKEY_LOCAL_MACHINE");
			}
			else if (KeyName == L"USER")
			{
				Path = L"\\Registry\\user";
				KeyName.Format(L"%s", L"HKEY_USERS");
			}
			//将所有系统盘符设置到树中
			HTREEITEM hItem2 = m_tree.InsertItem(KeyName, hItem1);
			wchar_t* pBuff = _wcsdup(Path.GetBuffer());
			m_tree.SetItemData(hItem2, (DWORD_PTR)pBuff);
		}
	}
	//释放空间
	free(pRegInfo);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

//左键单击树控件，+/-按钮点击响应
void CRegisterDlg::OnNMClickTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	//获取选中的树控件的项
	CPoint pos = { 0 };
	GetCursorPos(&pos);
	ScreenToClient(&pos);
	HTREEITEM hItem = m_tree.HitTest(pos);
	if (!hItem)
	{
		return;
	}

	//获取选中的数控件的项的数据（之前设置为对应文件的路径）
	CString Path = (WCHAR*)m_tree.GetItemData(hItem);
	if (Path.IsEmpty()||
		Path == L"\\Registry")
	{
		return;
	}

	HTREEITEM hChild = m_tree.GetChildItem(hItem);
	HTREEITEM hTempItem;
	while (hChild)//如果有子项
	{
		hTempItem = hChild;
		hChild = m_tree.GetNextSiblingItem(hChild);//得到下一个子项
		m_tree.DeleteItem(hTempItem);//删除它
	}

	//清空list列表
	m_list.DeleteAllItems();

	CService* myArkService = CService::GetService();
	//获取注册表键下子键的数量
	ULONG nCount = myArkService->GetRegisterChildCount(Path.GetBuffer(), (Path.GetLength() + 1) * 2);
	//根据返回的数量申请空间
	PVOID pRegInfo = malloc(sizeof(REGISTER)*nCount);
	//遍历注册表
	myArkService->EnumReg(Path.GetBuffer(), (Path.GetLength() + 1) * 2, pRegInfo, nCount);

	PREGISTER buffTemp = (PREGISTER)pRegInfo;

	int nIndex = 0;
	//插入信息到Tree控件
	for (int i = 0; i < nCount; i++)
	{
		//子项并且没有子节点
		if (buffTemp[i].Type == 0)
		{
			//将所有系统盘符设置到树中
			HTREEITEM hItem2 = m_tree.InsertItem(buffTemp[i].KeyName, hItem);
			//拼接路径
			WCHAR szFullPath[MAX_PATH] = { 0 };
			StringCbPrintf(szFullPath, MAX_PATH, L"%s\\%s", Path, buffTemp[i].KeyName);
			wchar_t* pBuff = _wcsdup(szFullPath);
			m_tree.SetItemData(hItem2, (DWORD_PTR)pBuff);
		}
		else if (buffTemp[i].Type == 1)
		{
			CString buffer;
			CString buffer2;

			m_list.InsertItem(nIndex, _T(""));

			//值名
			buffer = buffTemp[i].ValueName;
			if (buffer == L"")
			{
				buffer = L"默认";
			}
			m_list.SetItemText(nIndex, 0, buffer);

			switch (buffTemp[i].ValueType)
			{
				//表示以NULL结尾的字符串
			case REG_SZ:
			{
				buffer = L"REG_SZ";
				buffer2.Format(L"%s", buffTemp[i].Value);
				break;
			}
				//表示二进制数据
			case REG_BINARY:
			{
				buffer = L"REG_BINARY";
				buffer2 = L"";
				CString temp;
				for (int j = 0; j < buffTemp[i].ValueLength; ++j)
				{
					temp.Format(L"%02X", (unsigned char)buffTemp[i].Value[j]);
					buffer2 += temp;
					buffer2 += L" ";
				}
				break;
			}
			case REG_DWORD:
			{
				buffer = L"REG_DWORD";
				buffer2.Format(L"0x%08x(%d)", *(DWORD*)buffTemp[i].Value, *(DWORD*)buffTemp[i].Value);
				break;
			}
			case REG_MULTI_SZ:
			{
				buffer = L"REG_MULTI_SZ";
				buffer2.Format(L"%s", buffTemp[i].Value);
				break;
			}
			case REG_EXPAND_SZ:
			{
				buffer = L"REG_EXPAND_SZ";
				buffer2.Format(L"%s", buffTemp[i].Value);
				break;
			}
			default:
				buffer.Format(L"%d", buffTemp[i].ValueType);
				buffer2 = L"";
				CString temp;
				for (int j = 0; j < buffTemp[i].ValueLength; ++j)
				{
					temp.Format(L"%02X", (unsigned char)buffTemp[i].Value[j]);
					buffer2 += temp;
					buffer2 += L" ";
				}
				break;
			}
			m_list.SetItemText(nIndex, 1, buffer);	// 文
			m_list.SetItemText(nIndex, 2, buffer2);	// 数据
			nIndex++;
		}
	}

	//释放空间
	free(pRegInfo);
}


void CRegisterDlg::OnNMRClickTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码

	//获取选中的树控件的项
	CPoint pos = { 0 };
	GetCursorPos(&pos);
	ScreenToClient(&pos);
	HTREEITEM hItem = m_tree.HitTest(pos);
	if (!hItem)
	{
		return;
	}

	//获取选中的数控件的项的数据（之前设置为对应键名）
	CString Path = (WCHAR*)m_tree.GetItemData(hItem);
	if (Path.IsEmpty()||
		Path == L"\\Registry")
	{
		return;
	}
	m_KeyName = Path;

	GetCursorPos(&pos);
	//获取菜单的子菜单
	CMenu *nMenu = m_Menu.GetSubMenu(0);
	//弹出窗口
	nMenu->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this);

	*pResult = 0;

}

//创建子项
void CRegisterDlg::OnNew()
{
	// TODO: 在此添加命令处理程序代码
	CAddKeyDlg addKeyDlg(m_KeyName);
	addKeyDlg.DoModal();

}

//删除子项
void CRegisterDlg::OnDelete()
{
	// TODO: 在此添加命令处理程序代码

	CString LastPath;
	LastPath = m_KeyName + L"\\";

	CService* myArkService = CService::GetService();
	//遍历注册表
	myArkService->DeleteKey(LastPath.GetBuffer(), (LastPath.GetLength() + 1) * 2);

}
