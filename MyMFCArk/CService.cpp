#include "pch.h"
#include "CService.h"

CService* CService::m_pService = nullptr;

CService* CService::GetService()
{
	if (!m_pService)
	{
		MessageBoxA(NULL, "1", "1", NULL);
		m_pService = new CService;
		//加载驱动
		m_pService->LoadDriver();
		//打开设备，与0环建立链接
		m_pService->StartMyService();
	}
	return m_pService;
}

BOOL CService::LoadDriver()
{
	//1.打开服务管理器
	m_hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	//获取当前程序所在路径
	WCHAR pszFileName[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, pszFileName, MAX_PATH);
	//获取当前程序所在目录
	(wcsrchr(pszFileName, '\\'))[0] = 0;
	//拼接驱动文件路径
	WCHAR pszDriverName[MAX_PATH] = { 0 };
	swprintf_s(pszDriverName, L"%s\\%s", pszFileName, L"MyDriver.sys");

	//2.创建服务
	m_hService = CreateService(
		m_hServiceMgr,		//SMC句柄
		L"MyService",		//驱动服务名称
		L"MyDriver",		//驱动服务显示名称
		SERVICE_ALL_ACCESS,		//权限（所有访问权限）
		SERVICE_KERNEL_DRIVER,	//服务类型（驱动程序）
		SERVICE_DEMAND_START,	//启动方式
		SERVICE_ERROR_IGNORE,	//错误控制
		pszDriverName,		//驱动文件路径
		NULL,	//加载组命令
		NULL,	//TagId(指向一个加载顺序的标签值)
		NULL,	//依存关系
		NULL,	//服务启动名
		NULL	//密码
	);
	//2.1判断服务是否存在
	if (GetLastError() == ERROR_SERVICE_EXISTS)
	{
		//如果服务存在，只要打开
		m_hService = OpenService(m_hServiceMgr, L"MyService", SERVICE_ALL_ACCESS);
	}

	//3.启动服务
	StartService(m_hService, NULL, NULL);
	return TRUE;
}

BOOL CService::StartMyService()
{
	//打开设备，获取句柄
	m_hDev = CreateFile(
		L"\\\\.\\myark",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (m_hDev == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, L"[3环程序]打开驱动设备失败", L"提示", NULL);
		return FALSE;
	}

	//方便调试
	MessageBox(NULL, L"[3环程序]打开驱动设备成功", L"提示", NULL);
	return TRUE;
}

BOOL CService::CloseMyService()
{
	//关闭设备
	CloseHandle(m_hDev);
	//删除服务
	DeleteService(m_hService);
	//关闭服务句柄
	CloseServiceHandle(m_hService);
	return TRUE;
}

ULONG CService::GetDriverCount()
{
	DWORD size = 0;
	//用于接收从0环传输回来的驱动的数量
	ULONG buff = 0;
	//获取驱动的数量
	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		getDriverCount,	//控制码
		NULL,		//输入缓冲区
		0,			//输入缓冲区大小
		&buff,		//输出缓冲区
		sizeof(buff),	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return buff;
}

VOID CService::EnumDriver(PVOID pBuff, ULONG DriverCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		enumDriver,	//控制码
		NULL,		//输入缓冲区
		0,			//输入缓冲区大小
		pBuff,		//输出缓冲区
		sizeof(DRIVERINFO)*DriverCount,	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return VOID();
}

VOID CService::HideDriverInfo(WCHAR* pDriverName)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		hideDriver,	//控制码
		pDriverName,		//输入缓冲区
		MAX_PATH,			//输入缓冲区大小
		NULL,		//输出缓冲区
		0,			//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);
	return VOID();
}

ULONG CService::GetProcessCount()
{
	DWORD size = 0;
	//用于接收从0环传输回来的进程的数量
	ULONG buff = 0;
	//获取驱动的数量
	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		getProcessCount,	//控制码
		NULL,		//输入缓冲区
		0,			//输入缓冲区大小
		&buff,		//输出缓冲区
		sizeof(buff),	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return buff;
}

VOID CService::EnumProcess(PVOID pBuff, ULONG ProcessCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		enumProcess,	//控制码
		NULL,		//输入缓冲区
		0,			//输入缓冲区大小
		pBuff,		//输出缓冲区
		sizeof(PROCESSINFO)*ProcessCount,	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return VOID();
}

VOID CService::HideProcessInfo(PULONG pPid)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		hideProcess,	//控制码
		pPid,		//输入缓冲区
		sizeof(ULONG),			//输入缓冲区大小
		NULL,		//输出缓冲区
		0,			//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);
	return VOID();
}

VOID CService::KillProcess(PULONG pPid)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		killProcess,	//控制码
		pPid,		//输入缓冲区
		sizeof(ULONG),			//输入缓冲区大小
		NULL,		//输出缓冲区
		0,			//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);
	return VOID();
}

ULONG CService::GetModuleCount(PULONG pEprocess)
{
	DWORD size = 0;
	//用于接收从0环传输回来的模块的数量
	ULONG buff = 0;
	//获取驱动的数量
	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		getModuleCount,	//控制码
		pEprocess,		//输入缓冲区
		sizeof(ULONG),			//输入缓冲区大小
		&buff,		//输出缓冲区
		sizeof(buff),	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return buff;
}

VOID CService::EnumModule(PULONG pEprocess, PVOID pBuff, ULONG ModuleCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		enumModule,	//控制码
		pEprocess,		//输入缓冲区
		sizeof(ULONG),			//输入缓冲区大小
		pBuff,		//输出缓冲区
		sizeof(MODULEINFO)*ModuleCount,	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return VOID();
}

ULONG CService::GetThreadCount(PULONG pEprocess)
{
	DWORD size = 0;
	//用于接收从0环传输回来的线程的数量
	ULONG buff = 0;
	//获取驱动的数量
	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		getThreadCount,	//控制码
		pEprocess,		//输入缓冲区
		sizeof(ULONG),			//输入缓冲区大小
		&buff,		//输出缓冲区
		sizeof(buff),	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return buff;
}

VOID CService::EnumThread(PULONG pEprocess, PVOID pBuff, ULONG ThreadCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		enumThread,	//控制码
		pEprocess,		//输入缓冲区
		sizeof(ULONG),			//输入缓冲区大小
		pBuff,		//输出缓冲区
		sizeof(THREADINFO)*ThreadCount,	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return VOID();
}

ULONG CService::GetFileCount(PWCHAR pFileDir, ULONG uFileDirLength)
{
	DWORD size = 0;
	//用于接收从0环传输回来的文件的数量
	ULONG buff = 0;
	//获取驱动的数量
	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		getFileCount,	//控制码
		pFileDir,		//输入缓冲区
		uFileDirLength,			//输入缓冲区大小
		&buff,		//输出缓冲区
		sizeof(buff),	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return buff;
}

VOID CService::EnumFile(PWCHAR pFileDir, ULONG uFileDirLength, PVOID pBuff, ULONG FileCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		enumFile,	//控制码
		pFileDir,		//输入缓冲区
		uFileDirLength,			//输入缓冲区大小
		pBuff,		//输出缓冲区
		sizeof(FILEINFO)*FileCount,	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return VOID();
}

VOID CService::MyDeleteFile(PWCHAR pFilePath, ULONG uFileDirLength)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		deleteFile,	//控制码
		pFilePath,		//输入缓冲区
		uFileDirLength,	//输入缓冲区大小
		NULL,		//输出缓冲区
		0,			//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);
	return VOID();
}

ULONG CService::GetRegisterChildCount(PWCHAR pRegKeyName, ULONG uRegKeyNameLength)
{
	DWORD size = 0;
	//用于接收从0环传输回来的指定键下子键和数据的数量
	ULONG buff = 0;
	//获取驱动的数量
	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		getRegKeyCount,	//控制码
		pRegKeyName,		//输入缓冲区
		uRegKeyNameLength,			//输入缓冲区大小
		&buff,		//输出缓冲区
		sizeof(buff),	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return buff;
}

VOID CService::EnumReg(PWCHAR pRegKeyName, ULONG uRegKeyNameLength, PVOID pBuff, ULONG RegCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		enumReg,	//控制码
		pRegKeyName,		//输入缓冲区
		uRegKeyNameLength,			//输入缓冲区大小
		pBuff,		//输出缓冲区
		sizeof(REGISTER)*RegCount,	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return VOID();
}

VOID CService::NewKey(PWCHAR pRegKeyName, ULONG uRegKeyNameLength)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		newKey,	//控制码
		pRegKeyName,		//输入缓冲区
		uRegKeyNameLength,			//输入缓冲区大小
		NULL,		//输出缓冲区
		0,			//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);
	return VOID();
}

VOID CService::DeleteKey(PWCHAR pRegKeyName, ULONG uRegKeyNameLength)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		deleteKey,	//控制码
		pRegKeyName,		//输入缓冲区
		uRegKeyNameLength,			//输入缓冲区大小
		NULL,		//输出缓冲区
		0,			//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);
	return VOID();
}

VOID CService::EnumIdt(PVOID pBuff)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		enumIdt,	//控制码
		NULL,		//输入缓冲区
		0,			//输入缓冲区大小
		pBuff,		//输出缓冲区
		sizeof(IDTINFO)*0x100,	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return VOID();
}

ULONG CService::GetGdtCount()
{
	DWORD size = 0;
	//用于接收从0环传输回来的段描述符的数量
	ULONG buff = 0;
	//获取驱动的数量
	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		getGdtCount,	//控制码
		NULL,		//输入缓冲区
		0,			//输入缓冲区大小
		&buff,		//输出缓冲区
		sizeof(buff),	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return buff;
}

VOID CService::EnumGdt(PVOID pBuff, ULONG GdtCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		enumGdt,	//控制码
		NULL,		//输入缓冲区
		0,			//输入缓冲区大小
		pBuff,		//输出缓冲区
		sizeof(GDTINFO) * GdtCount,	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return VOID();
}

ULONG CService::GetSsdtCount()
{
	DWORD size = 0;
	//用于接收从0环传输回来的系统服务描述符的数量
	ULONG buff = 0;
	//获取驱动的数量
	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		getSsdtCount,	//控制码
		NULL,		//输入缓冲区
		0,			//输入缓冲区大小
		&buff,		//输出缓冲区
		sizeof(buff),	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return buff;
}

VOID CService::EnumSsdt(PVOID pBuff, ULONG SsdtCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		enumSsdt,	//控制码
		NULL,		//输入缓冲区
		0,			//输入缓冲区大小
		pBuff,		//输出缓冲区
		sizeof(SSDTINFO) * SsdtCount,	//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);

	return VOID();
}

VOID CService::SendSelfPid(PULONG pPid)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//打开的设备句柄
		selfPid,	//控制码
		pPid,		//输入缓冲区
		sizeof(ULONG),			//输入缓冲区大小
		NULL,		//输出缓冲区
		0,			//输出缓冲区的大小
		&size,		//实际传输的字节数
		NULL		//是否是OVERLAPPED操作
	);
	return VOID();
}
