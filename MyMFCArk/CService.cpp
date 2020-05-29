#include "pch.h"
#include "CService.h"

CService* CService::m_pService = nullptr;

CService* CService::GetService()
{
	if (!m_pService)
	{
		MessageBoxA(NULL, "1", "1", NULL);
		m_pService = new CService;
		//��������
		m_pService->LoadDriver();
		//���豸����0����������
		m_pService->StartMyService();
	}
	return m_pService;
}

BOOL CService::LoadDriver()
{
	//1.�򿪷��������
	m_hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	//��ȡ��ǰ��������·��
	WCHAR pszFileName[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, pszFileName, MAX_PATH);
	//��ȡ��ǰ��������Ŀ¼
	(wcsrchr(pszFileName, '\\'))[0] = 0;
	//ƴ�������ļ�·��
	WCHAR pszDriverName[MAX_PATH] = { 0 };
	swprintf_s(pszDriverName, L"%s\\%s", pszFileName, L"MyDriver.sys");

	//2.��������
	m_hService = CreateService(
		m_hServiceMgr,		//SMC���
		L"MyService",		//������������
		L"MyDriver",		//����������ʾ����
		SERVICE_ALL_ACCESS,		//Ȩ�ޣ����з���Ȩ�ޣ�
		SERVICE_KERNEL_DRIVER,	//�������ͣ���������
		SERVICE_DEMAND_START,	//������ʽ
		SERVICE_ERROR_IGNORE,	//�������
		pszDriverName,		//�����ļ�·��
		NULL,	//����������
		NULL,	//TagId(ָ��һ������˳��ı�ǩֵ)
		NULL,	//�����ϵ
		NULL,	//����������
		NULL	//����
	);
	//2.1�жϷ����Ƿ����
	if (GetLastError() == ERROR_SERVICE_EXISTS)
	{
		//���������ڣ�ֻҪ��
		m_hService = OpenService(m_hServiceMgr, L"MyService", SERVICE_ALL_ACCESS);
	}

	//3.��������
	StartService(m_hService, NULL, NULL);
	return TRUE;
}

BOOL CService::StartMyService()
{
	//���豸����ȡ���
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
		MessageBox(NULL, L"[3������]�������豸ʧ��", L"��ʾ", NULL);
		return FALSE;
	}

	//�������
	MessageBox(NULL, L"[3������]�������豸�ɹ�", L"��ʾ", NULL);
	return TRUE;
}

BOOL CService::CloseMyService()
{
	//�ر��豸
	CloseHandle(m_hDev);
	//ɾ������
	DeleteService(m_hService);
	//�رշ�����
	CloseServiceHandle(m_hService);
	return TRUE;
}

ULONG CService::GetDriverCount()
{
	DWORD size = 0;
	//���ڽ��մ�0���������������������
	ULONG buff = 0;
	//��ȡ����������
	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		getDriverCount,	//������
		NULL,		//���뻺����
		0,			//���뻺������С
		&buff,		//���������
		sizeof(buff),	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return buff;
}

VOID CService::EnumDriver(PVOID pBuff, ULONG DriverCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		enumDriver,	//������
		NULL,		//���뻺����
		0,			//���뻺������С
		pBuff,		//���������
		sizeof(DRIVERINFO)*DriverCount,	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return VOID();
}

VOID CService::HideDriverInfo(WCHAR* pDriverName)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		hideDriver,	//������
		pDriverName,		//���뻺����
		MAX_PATH,			//���뻺������С
		NULL,		//���������
		0,			//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);
	return VOID();
}

ULONG CService::GetProcessCount()
{
	DWORD size = 0;
	//���ڽ��մ�0����������Ľ��̵�����
	ULONG buff = 0;
	//��ȡ����������
	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		getProcessCount,	//������
		NULL,		//���뻺����
		0,			//���뻺������С
		&buff,		//���������
		sizeof(buff),	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return buff;
}

VOID CService::EnumProcess(PVOID pBuff, ULONG ProcessCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		enumProcess,	//������
		NULL,		//���뻺����
		0,			//���뻺������С
		pBuff,		//���������
		sizeof(PROCESSINFO)*ProcessCount,	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return VOID();
}

VOID CService::HideProcessInfo(PULONG pPid)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		hideProcess,	//������
		pPid,		//���뻺����
		sizeof(ULONG),			//���뻺������С
		NULL,		//���������
		0,			//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);
	return VOID();
}

VOID CService::KillProcess(PULONG pPid)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		killProcess,	//������
		pPid,		//���뻺����
		sizeof(ULONG),			//���뻺������С
		NULL,		//���������
		0,			//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);
	return VOID();
}

ULONG CService::GetModuleCount(PULONG pEprocess)
{
	DWORD size = 0;
	//���ڽ��մ�0�����������ģ�������
	ULONG buff = 0;
	//��ȡ����������
	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		getModuleCount,	//������
		pEprocess,		//���뻺����
		sizeof(ULONG),			//���뻺������С
		&buff,		//���������
		sizeof(buff),	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return buff;
}

VOID CService::EnumModule(PULONG pEprocess, PVOID pBuff, ULONG ModuleCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		enumModule,	//������
		pEprocess,		//���뻺����
		sizeof(ULONG),			//���뻺������С
		pBuff,		//���������
		sizeof(MODULEINFO)*ModuleCount,	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return VOID();
}

ULONG CService::GetThreadCount(PULONG pEprocess)
{
	DWORD size = 0;
	//���ڽ��մ�0������������̵߳�����
	ULONG buff = 0;
	//��ȡ����������
	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		getThreadCount,	//������
		pEprocess,		//���뻺����
		sizeof(ULONG),			//���뻺������С
		&buff,		//���������
		sizeof(buff),	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return buff;
}

VOID CService::EnumThread(PULONG pEprocess, PVOID pBuff, ULONG ThreadCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		enumThread,	//������
		pEprocess,		//���뻺����
		sizeof(ULONG),			//���뻺������С
		pBuff,		//���������
		sizeof(THREADINFO)*ThreadCount,	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return VOID();
}

ULONG CService::GetFileCount(PWCHAR pFileDir, ULONG uFileDirLength)
{
	DWORD size = 0;
	//���ڽ��մ�0������������ļ�������
	ULONG buff = 0;
	//��ȡ����������
	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		getFileCount,	//������
		pFileDir,		//���뻺����
		uFileDirLength,			//���뻺������С
		&buff,		//���������
		sizeof(buff),	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return buff;
}

VOID CService::EnumFile(PWCHAR pFileDir, ULONG uFileDirLength, PVOID pBuff, ULONG FileCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		enumFile,	//������
		pFileDir,		//���뻺����
		uFileDirLength,			//���뻺������С
		pBuff,		//���������
		sizeof(FILEINFO)*FileCount,	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return VOID();
}

VOID CService::MyDeleteFile(PWCHAR pFilePath, ULONG uFileDirLength)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		deleteFile,	//������
		pFilePath,		//���뻺����
		uFileDirLength,	//���뻺������С
		NULL,		//���������
		0,			//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);
	return VOID();
}

ULONG CService::GetRegisterChildCount(PWCHAR pRegKeyName, ULONG uRegKeyNameLength)
{
	DWORD size = 0;
	//���ڽ��մ�0�����������ָ�������Ӽ������ݵ�����
	ULONG buff = 0;
	//��ȡ����������
	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		getRegKeyCount,	//������
		pRegKeyName,		//���뻺����
		uRegKeyNameLength,			//���뻺������С
		&buff,		//���������
		sizeof(buff),	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return buff;
}

VOID CService::EnumReg(PWCHAR pRegKeyName, ULONG uRegKeyNameLength, PVOID pBuff, ULONG RegCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		enumReg,	//������
		pRegKeyName,		//���뻺����
		uRegKeyNameLength,			//���뻺������С
		pBuff,		//���������
		sizeof(REGISTER)*RegCount,	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return VOID();
}

VOID CService::NewKey(PWCHAR pRegKeyName, ULONG uRegKeyNameLength)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		newKey,	//������
		pRegKeyName,		//���뻺����
		uRegKeyNameLength,			//���뻺������С
		NULL,		//���������
		0,			//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);
	return VOID();
}

VOID CService::DeleteKey(PWCHAR pRegKeyName, ULONG uRegKeyNameLength)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		deleteKey,	//������
		pRegKeyName,		//���뻺����
		uRegKeyNameLength,			//���뻺������С
		NULL,		//���������
		0,			//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);
	return VOID();
}

VOID CService::EnumIdt(PVOID pBuff)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		enumIdt,	//������
		NULL,		//���뻺����
		0,			//���뻺������С
		pBuff,		//���������
		sizeof(IDTINFO)*0x100,	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return VOID();
}

ULONG CService::GetGdtCount()
{
	DWORD size = 0;
	//���ڽ��մ�0����������Ķ�������������
	ULONG buff = 0;
	//��ȡ����������
	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		getGdtCount,	//������
		NULL,		//���뻺����
		0,			//���뻺������С
		&buff,		//���������
		sizeof(buff),	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return buff;
}

VOID CService::EnumGdt(PVOID pBuff, ULONG GdtCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		enumGdt,	//������
		NULL,		//���뻺����
		0,			//���뻺������С
		pBuff,		//���������
		sizeof(GDTINFO) * GdtCount,	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return VOID();
}

ULONG CService::GetSsdtCount()
{
	DWORD size = 0;
	//���ڽ��մ�0�����������ϵͳ����������������
	ULONG buff = 0;
	//��ȡ����������
	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		getSsdtCount,	//������
		NULL,		//���뻺����
		0,			//���뻺������С
		&buff,		//���������
		sizeof(buff),	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return buff;
}

VOID CService::EnumSsdt(PVOID pBuff, ULONG SsdtCount)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		enumSsdt,	//������
		NULL,		//���뻺����
		0,			//���뻺������С
		pBuff,		//���������
		sizeof(SSDTINFO) * SsdtCount,	//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);

	return VOID();
}

VOID CService::SendSelfPid(PULONG pPid)
{
	DWORD size = 0;

	DeviceIoControl(
		m_hDev,		//�򿪵��豸���
		selfPid,	//������
		pPid,		//���뻺����
		sizeof(ULONG),			//���뻺������С
		NULL,		//���������
		0,			//����������Ĵ�С
		&size,		//ʵ�ʴ�����ֽ���
		NULL		//�Ƿ���OVERLAPPED����
	);
	return VOID();
}
