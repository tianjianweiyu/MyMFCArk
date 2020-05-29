//Service.h
//������0��ͨ��

#pragma once
#include <winioctl.h>
#include <winsvc.h>

//������
#define MYCTLCODE(code) CTL_CODE(FILE_DEVICE_UNKNOWN,0x800+(code),METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

enum MyCtlCode
{
	getDriverCount = MYCTLCODE(1),		//��ȡ��������
	enumDriver = MYCTLCODE(2),		//��������
	hideDriver = MYCTLCODE(3),		//��������
	getProcessCount = MYCTLCODE(4),	//��ȡ��������
	enumProcess = MYCTLCODE(5),		//��������
	hideProcess = MYCTLCODE(6),		//���ؽ���
	killProcess = MYCTLCODE(7),		//��������
	getModuleCount = MYCTLCODE(8),	//��ȡָ�����̵�ģ������
	enumModule = MYCTLCODE(9),		//����ָ�����̵�ģ��
	getThreadCount = MYCTLCODE(10),	//��ȡָ�����̵��߳�����
	enumThread = MYCTLCODE(11),		//����ָ�����̵��߳�
	getFileCount = MYCTLCODE(12),	//��ȡ�ļ�����
	enumFile = MYCTLCODE(13),		//�����ļ�
	deleteFile = MYCTLCODE(14),		//ɾ���ļ�
	getRegKeyCount = MYCTLCODE(15),	//��ȡע�����������
	enumReg = MYCTLCODE(16),		//����ע���
	newKey = MYCTLCODE(17),			//�����¼�
	deleteKey = MYCTLCODE(18),		//ɾ���¼�
	enumIdt = MYCTLCODE(19),		//����IDT��
	getGdtCount = MYCTLCODE(20),	//��ȡGDT���ж�������������
	enumGdt = MYCTLCODE(21),		//����GDT��
	getSsdtCount = MYCTLCODE(22),	//��ȡSSDT����ϵͳ����������������
	enumSsdt = MYCTLCODE(23),		//����SSDT��
	selfPid = MYCTLCODE(24),		//������Ľ���ID����0��
};

//������Ϣ�ṹ��
typedef struct _DRIVERINFO
{
	WCHAR wcDriverBasePath[MAX_PATH];	//������
	WCHAR wcDriverFullPath[MAX_PATH];	//����·��
	PVOID DllBase;		//���ػ�ַ
	ULONG SizeOfImage;	//��С
}DRIVERINFO, *PDRIVERINFO;

//������Ϣ�ṹ��
typedef struct _PROCESSINFO
{
	WCHAR wcProcessFullPath[MAX_PATH];	//ӳ��·��
	PVOID Pid;		//����ID
	PVOID PPid;	//������ID
	PVOID pEproce;	//����ִ�п��ַ
}PROCESSINFO, *PPROCESSINFO;

//ģ����Ϣ�ṹ��
typedef struct _MODULEINFO
{
	WCHAR wcModuleFullPath[MAX_PATH];	//ģ��·��
	PVOID DllBase;		//����ַ
	ULONG SizeOfImage;	//��С
}MODULEINFO, *PMODULEINFO;

//�߳���Ϣ�ṹ��
typedef struct _THREADINFO
{
	PVOID Tid;		//�߳�ID
	PVOID pEthread;	//�߳�ִ�п��ַ
	PVOID pTeb;		//Teb�ṹ��ַ
	ULONG BasePriority;	//��̬���ȼ�
	ULONG ContextSwitches;	//�л�����
}THREADINFO, *PTHREADINFO;

//�ļ���Ϣ�ṹ��
typedef struct _FILEINFO
{
	char FileOrDirectory;	//һ���ֽ����������ļ�����Ŀ¼,0��ʾĿ¼��1��ʾ�ļ�
	WCHAR wcFileName[MAX_PATH];	//�ļ���
	LONGLONG Size;				//�ļ���С
	LARGE_INTEGER CreateTime;	//����ʱ��
	LARGE_INTEGER ChangeTime;	//�޸�ʱ��
}FILEINFO, *PFILEINFO;

//ע�����Ϣ�ṹ��
typedef struct _REGISTER
{
	ULONG Type;				// ���ͣ�0 Ϊ���1Ϊֵ
	WCHAR KeyName[256];		// ����
	WCHAR ValueName[256];   // ֵ������
	ULONG ValueType;		// ֵ������
	UCHAR Value[256];	    // ֵ
	ULONG ValueLength;		//ֵ�ĳ���
}REGISTER, *PREGISTER;

//IDT��Ϣ�ṹ��
typedef struct _IDTINFO
{
	ULONG pFunction;		//�������ĵ�ַ
	ULONG Selector;			//��ѡ����
	ULONG ParamCount;		//��������
	ULONG Dpl;				//����Ȩ��
	ULONG GateType;			//����
}IDTINFO, *PIDTINFO;

//GDT��Ϣ�ṹ��
typedef struct _GDTINFO
{
	ULONG BaseAddr;	//�λ�ַ
	ULONG Limit;	//���޳�
	ULONG Grain;	//������
	ULONG Dpl;		//��Ȩ�ȼ�
	ULONG GateType;	//����
}GDTINFO, *PGDTINFO;

//SSDT��Ϣ�ṹ��
typedef struct _SSDTINFO
{
	ULONG uIndex;	//�ص���
	ULONG uFuntionAddr;	//������ַ
}SSDTINFO, *PSSDTINFO;

class CService
{
private:
	HANDLE m_hDev;

	SC_HANDLE m_hService;
	SC_HANDLE m_hServiceMgr;

	//ֻ��ʵ����һ����
	static CService* m_pService;

public:

	//��ȡ������ֻ����ǰ����һ������
	static CService* GetService();

	//��������
	BOOL LoadDriver();

	//���豸����0����������
	BOOL StartMyService();

	//�ر��豸
	BOOL CloseMyService();

	//��ȡ��������
	ULONG GetDriverCount();

	/*!
	*  �� �� ���� EnumDriver
	*  ��    �ڣ� 2020/05/22
	*  �������ͣ� VOID
	*  ��    ���� PVOID pBuff ���ڽ������б���������������Ϣ
	*  ��    ���� ULONG DriverCount ��������
	*  ��    �ܣ� ��������
	*/
	VOID EnumDriver(PVOID pBuff, ULONG DriverCount);

	/*!
	*  �� �� ���� HideDriverInfo
	*  ��    �ڣ� 2020/05/22
	*  �������ͣ� VOID
	*  ��    ���� char * pDriverName ������
	*  ��    �ܣ� ��������
	*/
	VOID HideDriverInfo(WCHAR* pDriverName);

	//��ȡ��������
	ULONG GetProcessCount();

	/*!
	*  �� �� ���� EnumProcess
	*  ��    �ڣ� 2020/05/22
	*  �������ͣ� VOID
	*  ��    ���� PVOID pBuff	���ڽ������б������Ľ��̵���Ϣ
	*  ��    ���� ULONG ProcessCount ��������
	*  ��    �ܣ� ��������
	*/
	VOID EnumProcess(PVOID pBuff, ULONG ProcessCount);

	/*!
	*  �� �� ���� HideProcessInfo
	*  ��    �ڣ� 2020/05/22
	*  �������ͣ� VOID
	*  ��    ���� PULONG pPid Ҫ���صĽ��̵�ID
	*  ��    �ܣ� ���ؽ���
	*/
	VOID HideProcessInfo(PULONG pPid);

	/*!
	*  �� �� ���� KillProcess
	*  ��    �ڣ� 2020/05/22
	*  �������ͣ� VOID
	*  ��    ���� PULONG pPid Ҫ���صĽ��̵�ID
	*  ��    �ܣ� ��������
	*/
	VOID KillProcess(PULONG pPid);

	/*!
	*  �� �� ���� GetModuleCount
	*  ��    �ڣ� 2020/05/22
	*  �������ͣ� ULONG
	*  ��    ���� PULONG pEprocess ����ִ�п��ַ
	*  ��    �ܣ� ��ȡģ������
	*/
	ULONG GetModuleCount(PULONG pEprocess);

	/*!
	*  �� �� ���� EnumModule
	*  ��    �ڣ� 2020/05/22
	*  �������ͣ� VOID
	*  ��    ���� PULONG pEprocess ����ִ�п��ַ
	*  ��    ���� PVOID pBuff ���ڽ������б�������ģ�����Ϣ
	*  ��    ���� ULONG ModuleCount ģ�������
	*  ��    �ܣ� ����ģ��
	*/
	VOID EnumModule(PULONG pEprocess, PVOID pBuff, ULONG ModuleCount);

	/*!
	*  �� �� ���� GetThreadCount
	*  ��    �ڣ� 2020/05/23
	*  �������ͣ� ULONG
	*  ��    ���� PULONG pEprocess ����ִ�п��ַ
	*  ��    �ܣ� ��ȡ�߳�����
	*/
	ULONG GetThreadCount(PULONG pEprocess);

	/*!
	*  �� �� ���� EnumThread
	*  ��    �ڣ� 2020/05/23
	*  �������ͣ� VOID
	*  ��    ���� PULONG pEprocess ����ִ�п��ַ
	*  ��    ���� PVOID pBuff ���ڽ������б��������̵߳���Ϣ
	*  ��    ���� ULONG ThreadCount �̵߳�����
	*  ��    �ܣ� �����߳�
	*/
	VOID EnumThread(PULONG pEprocess, PVOID pBuff, ULONG ThreadCount);

	/*!
	*  �� �� ���� GetFileCount
	*  ��    �ڣ� 2020/05/24
	*  �������ͣ� ULONG
	*  ��    ���� PWCHAR pFileDir Ŀ¼��
	*  ��    ���� ULONG uFileDirLength Ŀ¼������
	*  ��    �ܣ� ��ȡָ��Ŀ¼�ļ�����
	*/
	ULONG GetFileCount(PWCHAR pFileDir, ULONG uFileDirLength);

	/*!
	*  �� �� ���� EnumFile
	*  ��    �ڣ� 2020/05/25
	*  �������ͣ� VOID
	*  ��    ���� PWCHAR pFileDir Ŀ¼��
	*  ��    ���� ULONG uFileDirLength Ŀ¼������
	*  ��    ���� PVOID pBuff ���ڽ������б��������ļ�����Ϣ
	*  ��    ���� ULONG FileCount ָ��Ŀ¼�ļ�������
	*  ��    �ܣ� �����ļ�
	*/
	VOID EnumFile(PWCHAR pFileDir, ULONG uFileDirLength, PVOID pBuff, ULONG FileCount);

	/*!
	*  �� �� ���� MyDeleteFile
	*  ��    �ڣ� 2020/05/26
	*  �������ͣ� VOID
	*  ��    ���� PWCHAR pFilePath �ļ�·��
	*  ��    ���� ULONG uFileDirLength	�ļ�·������
	*  ��    �ܣ� ɾ���ļ�
	*/
	VOID MyDeleteFile(PWCHAR pFilePath, ULONG uFileDirLength);

	/*!
	*  �� �� ���� GetRegisterChildCount
	*  ��    �ڣ� 2020/05/27
	*  �������ͣ� ULONG
	*  ��    ���� PWCHAR pRegKeyName ����
	*  ��    ���� ULONG uRegKeyNameLength ��������
	*  ��    �ܣ� ��ȡע���ָ�����µ��Ӽ���������������
	*/
	ULONG GetRegisterChildCount(PWCHAR pRegKeyName, ULONG uRegKeyNameLength);

	/*!
	*  �� �� ���� EnumReg
	*  ��    �ڣ� 2020/05/27
	*  �������ͣ� VOID
	*  ��    ���� PWCHAR pRegKeyName ����
	*  ��    ���� ULONG uRegKeyNameLength ��������
	*  ��    ���� PVOID pBuff ���ڽ������б�������ע������Ϣ
	*  ��    ���� ULONG RegCount ע���ָ�����µ��Ӽ���������������
	*  ��    �ܣ� ����ע���
	*/
	VOID EnumReg(PWCHAR pRegKeyName, ULONG uRegKeyNameLength, PVOID pBuff, ULONG RegCount);

	/*!
	*  �� �� ���� NewKey
	*  ��    �ڣ� 2020/05/27
	*  �������ͣ� VOID
	*  ��    ���� PWCHAR pRegKeyName ����
	*  ��    ���� ULONG uRegKeyNameLength ��������
	*  ��    �ܣ� ����һ����
	*/
	VOID NewKey(PWCHAR pRegKeyName, ULONG uRegKeyNameLength);

	/*!
	*  �� �� ���� DeleteKey
	*  ��    �ڣ� 2020/05/27
	*  �������ͣ� VOID
	*  ��    ���� PWCHAR pRegKeyName ����
	*  ��    ���� ULONG uRegKeyNameLength ��������
	*  ��    �ܣ� ɾ��һ����
	*/
	VOID DeleteKey(PWCHAR pRegKeyName, ULONG uRegKeyNameLength);

	/*!
	*  �� �� ���� EnumIdt
	*  ��    �ڣ� 2020/05/27
	*  �������ͣ� VOID
	*  ��    ���� PVOID pBuff ���ڽ������б�������IDT�����Ϣ
	*  ��    �ܣ� ����IDT(�ж�������)��
	*/
	VOID EnumIdt(PVOID pBuff);

	/*!
	*  �� �� ���� GetGdtCount
	*  ��    �ڣ� 2020/05/27
	*  �������ͣ� ULONG
	*  ��    �ܣ� ��ȡGDT���ж�������������
	*/
	ULONG GetGdtCount();

	//����GDT��
	/*!
	*  �� �� ���� EnumGdt
	*  ��    �ڣ� 2020/05/28
	*  �������ͣ� VOID
	*  ��    ���� PVOID pBuff ���ڽ������б�������GDT�����Ϣ
	*  ��    ���� ULONG GdtCount GDT���ж�������������
	*  ��    �ܣ� ����GDT��
	*/
	VOID EnumGdt(PVOID pBuff, ULONG GdtCount);

	/*!
	*  �� �� ���� GetSsdtCount
	*  ��    �ڣ� 2020/05/28
	*  �������ͣ� ULONG
	*  ��    �ܣ� ��ȡSSDT���з������ĸ���
	*/
	ULONG GetSsdtCount();

	/*!
	*  �� �� ���� EnumSsdt
	*  ��    �ڣ� 2020/05/28
	*  �������ͣ� VOID
	*  ��    ���� PVOID pBuff ���ڽ������б�������SSDT�����Ϣ
	*  ��    ���� ULONG SsdtCount SSDT���з������ĸ���
	*  ��    �ܣ� ����SSDT��
	*/
	VOID EnumSsdt(PVOID pBuff, ULONG SsdtCount);

	/*!
	*  �� �� ���� SendSelfPid
	*  ��    �ڣ� 2020/05/28
	*  �������ͣ� VOID
	*  ��    ���� PULONG pPid ������PID
	*  ��    �ܣ� �����Լ���PID��0����ʵ���Ա���
	*/
	VOID SendSelfPid(PULONG pPid);
};

