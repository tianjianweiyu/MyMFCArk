#include <Ntifs.h>
#include <ntimage.h>


//�豸��
#define NAME_DEVICE L"\\Device\\myark"
//���ӷ�����
#define  NAME_SYMBOL L"\\DosDevices\\myark"
//������
#define MYCTLCODE(code) CTL_CODE(FILE_DEVICE_UNKNOWN,0x800+(code),METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

#define MAKE_LONG(a, b)      ((LONG)(((UINT16)(((DWORD_PTR)(a)) & 0xffff)) | ((ULONG)((UINT16)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define MAKE_LONG2(a, b)      ((LONG)(((UINT64)(((DWORD_PTR)(a)) & 0xffffff)) | ((ULONG)((UINT64)(((DWORD_PTR)(b)) & 0xff))) << 24))

ULONG_PTR g_OldKiFastCallEntry;		//����ԭ���� KiFastCallEntry ������ַ
ULONG g_uPid;	//Ҫ�����Ľ��̵�PID

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
	WCHAR wcDriverBasePath[260];	//������
	WCHAR wcDriverFullPath[260];	//����·��
	PVOID DllBase;		//���ػ�ַ
	ULONG SizeOfImage;	//��С
}DRIVERINFO, *PDRIVERINFO;

//������Ϣ�ṹ��
typedef struct _PROCESSINFO
{
	WCHAR wcProcessFullPath[260];	//ӳ��·��
	PVOID Pid;		//����ID
	PVOID PPid;	//������ID
	PVOID pEproce;	//����ִ�п��ַ
}PROCESSINFO, *PPROCESSINFO;

//ģ����Ϣ�ṹ��
typedef struct _MODULEINFO
{
	WCHAR wcModuleFullPath[260];	//ģ��·��
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
	WCHAR wcFileName[260];	//�ļ���
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

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	union
	{
		LIST_ENTRY HashLinks;
		struct {
			PVOID SectionPointer;
			ULONG CheckSum;
		}s1;
	}u1;
	union
	{
		struct {
			ULONG TimeDateStamp;
		}s2;
		struct {
			PVOID LoadedImports;
		}s3;
	}u2;
}LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

//GDTR��IDTR�Ĵ���ָ��Ľṹ��(X��ʾi��g)
typedef struct _XDT_INFO
{
	UINT16 uXdtLimit;		//Xdt��Χ
	UINT16 xLowXdtBase;		//Xdt�ͻ�ַ
	UINT16 xHighXdtBase;	//Xdt�߻�ַ
}XDT_INFO, *PXDT_INFO;

#pragma pack(1)
//IDT�������Ľṹ��
typedef struct _IDT_ENTRY
{
	UINT32 uOffsetLow : 16;		//�������͵�ַƫ�� 
	UINT32 uSelector : 16;		//��ѡ����
	UINT32 paramCount : 5;		//��������
	UINT32 none : 3;			//�ޣ�����
	UINT32 GateType : 4;		//�ж�����
	UINT32 StorageSegment : 1;	//Ϊ0��ϵͳ�Σ�Ϊ1�����ݶλ�����
	UINT32 DPL : 2;			//��Ȩ��
	UINT32 Present : 1;		//���Ƿ���Ч
	UINT32 uOffsetHigh : 16;		//�������ߵ�ַƫ��
}IDT_ENTER, *PIDT_ENTRY;

typedef struct _GDT_ENTRY
{
	UINT64 Limit0_15 : 16;	//���޳��͵�ַƫ��
	UINT64 base0_23 : 24;	//�λ���ַ�͵�ַƫ��
	UINT64 Type : 4;		//������
	UINT64 S : 1;			//���������ͣ�0ϵͳ�Σ�1���ݶλ����Σ�
	UINT64 DPL : 2;			//��Ȩ�ȼ�
	UINT64 P : 1;			//���Ƿ���Ч
	UINT64 Limit16_19 : 4;	//���޳��ߵ�ַƫ��
	UINT64 AVL : 1;			//�ɹ�ϵͳ���ʹ��
	UINT64 L : 1;
	UINT64 D_B : 1;			//Ĭ�ϲ�����С��0=16λ�Σ�1=32λ�Σ�
	UINT64 G : 1;			//����
	UINT64 base24_31 : 8;	//�λ���ַ�ߵ�ַƫ��
}GDT_ENTER, *PGDT_ENTER;

//ϵͳ���������ṹ��
//win7-x32 ��ϵͳ����ϵͳ��������
//KeServiceDescriptorTable�����������Ϳ���ֱ��ʹ��
typedef struct _ServiceDesriptorEntry
{
	ULONG* ServiceTableBase;		//������ַ
	ULONG* ServiceCounterTableBase;	//��������ÿ�����������õĴ���
	ULONG NumberOfService;			//�������ĸ���,
	ULONG ParamTableBase;			//�������ַ
}SSDTEntry, *PSSDTEntry;
#pragma pack()

// ����SSDTȫ�ֱ���
NTSYSAPI SSDTEntry KeServiceDescriptorTable;
static char*		g_pNewNtKernel;		// ���ں�
static ULONG		g_ntKernelSize;		// �ں˵�ӳ���С
static SSDTEntry*	g_pNewSSDTEntry;	// ��ssdt����ڵ�ַ
static ULONG		g_hookAddr;			// ��hookλ�õ��׵�ַ
static ULONG		g_hookAddr_next_ins;// ��hook��ָ�����һ��ָ����׵�ַ.

//���������ж��ĺ���
NTSTATUS OnCreate(DEVICE_OBJECT *pDevice, IRP *pIrp);
NTSTATUS OnClose(DEVICE_OBJECT* pDevice, IRP* pIrp);
VOID OnUnload(DRIVER_OBJECT* driver);
NTSTATUS OnDeviceIoControl(DEVICE_OBJECT* pDevice, IRP* pIrp);

//SYSENTER-HOOK
VOID InstallSysenterHook();

//�Լ������ KiFastCallEntry ����
VOID MyKiFastCallEntry();

//ж�� SYSENTER-HOOK
VOID UninstallSysenterHook();

// ����NT�ں�ģ��
// ����ȡ���Ļ����������ݱ��浽pBuff��.
NTSTATUS loadNtKernelModule(UNICODE_STRING* ntkernelPath, char** pOut);

// �޸��ض�λ.
void fixRelocation(char* pDosHdr, ULONG curNtKernelBase);

// ���SSDT��
// char* pNewBase - �¼��ص��ں˶ѿռ��׵�ַ
// char* pCurKernelBase - ��ǰ����ʹ�õ��ں˵ļ��ػ�ַ
void initSSDT(char* pNewBase, char* pCurKernelBase);

//��װhook
void installHook();

//ж��hook
void uninstallHook();

// inline hook KiFastCallEntry�ĺ���
void myKiFastEntryHook();

/*!
*  �� �� ���� createFile
*  ��    �ڣ� 2020/05/29
*  �������ͣ� NTSTATUS
*  ��    ���� wchar_t * filepath �ļ�·��,·���������豸��"\\device\\myark\\"�����������"\\??\\C:\\1.txt"
*  ��    ���� ULONG access ����Ȩ��, GENERIC_READ, GENERIC_XXX
*  ��    ���� ULONG share �ļ�����ʽ: FILE_SHARE_XXX
*  ��    ���� ULONG openModel �򿪷�ʽ: FILE_OPEN_IF,FILE_CREATE ...
*  ��    ���� BOOLEAN isDir �Ƿ�ΪĿ¼
*  ��    ���� HANDLE * hFile �������ļ����
*  ��    �ܣ� �����ļ�
*/
NTSTATUS createFile(wchar_t* filepath,
	ULONG access,
	ULONG share,
	ULONG openModel,
	BOOLEAN isDir,
	HANDLE* hFile);

/*!
*  �� �� ���� readFile
*  ��    �ڣ� 2020/05/29
*  �������ͣ� NTSTATUS
*  ��    ���� HANDLE hFile �ļ����
*  ��    ���� ULONG offsetLow �ļ�ƫ�Ƶĵ�32λ, �Ӵ�λ�ÿ�ʼ��ȡ
*  ��    ���� ULONG offsetHig �ļ�ƫ�Ƶĸ�32λ, �Ӵ�λ�ÿ�ʼ��ȡ
*  ��    ���� ULONG sizeToRead Ҫ��ȡ���ֽ���
*  ��    ���� PVOID pBuff �����ļ����ݵĻ����� , ��Ҫ�Լ������ڴ�ռ�.
*  ��    ���� ULONG * read ʵ�ʶ�ȡ�����ֽ���
*  ��    �ܣ� ��ȡ�ļ�����
*/
NTSTATUS readFile(HANDLE hFile,
	ULONG offsetLow,
	ULONG offsetHig,
	ULONG sizeToRead,
	PVOID pBuff,
	ULONG* read);

//������ں���
NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING path)
{
	//�����������δʹ�õĴ���
	UNREFERENCED_PARAMETER(path);

	//����ֵ
	NTSTATUS status = STATUS_SUCCESS;

	///////�����ں�����/////////////////////////////////////////////////////////////////////////////

	// 1. �ҵ��ں��ļ�·��
	// 1.1 ͨ�������ں�����ķ�ʽ���ҵ��ں���ģ��
	LDR_DATA_TABLE_ENTRY* pLdr = ((LDR_DATA_TABLE_ENTRY*)driver->DriverSection);
	// 1.2 �ں���ģ���������еĵ�2��.
	for (int i = 0; i < 2; ++i)
		pLdr = (LDR_DATA_TABLE_ENTRY*)pLdr->InLoadOrderLinks.Flink;

	// 1.3 �����ں˵�ӳ���С
	g_ntKernelSize = pLdr->SizeOfImage;

	// 1.4 ���浱ǰ���ػ�ַ
	char* pCurKernelBase = (char*)pLdr->DllBase;

	// 2. ��ȡntģ����ļ����ݵ��ѿռ�.
	status = loadNtKernelModule(&pLdr->FullDllName, &g_pNewNtKernel);
	if (STATUS_SUCCESS != status)
	{
		return status;
	}

	// 3. �޸���ntģ����ض�λ.
	fixRelocation(g_pNewNtKernel, (ULONG)pCurKernelBase);

	// 4. ʹ�õ�ǰ����ʹ�õ��ں˵����������
	//    ���ں˵�SSDT��.
	initSSDT(g_pNewNtKernel, pCurKernelBase);

	// 5. HOOK KiFastCallEntry,ʹ���ú������ں˵�·��
	installHook();

	///////�ں����ؽ���/////////////////////////////////////////////////////////////////////////////

	//��ж�غ���
	driver->DriverUnload = OnUnload;

	//�����豸
	//��ʼ���豸���������
	UNICODE_STRING devName = RTL_CONSTANT_STRING(NAME_DEVICE);
	//���洴������豸�����ָ��
	PDEVICE_OBJECT pDevice = NULL;

	//�����豸����
	status = IoCreateDevice(
		driver,			//��������(�´������豸����������������)
		0,				//�豸��չ��С
		&devName,		//�豸����
		FILE_DEVICE_UNKNOWN,	//�豸����(δ֪����)
		0,				//�豸������Ϣ
		FALSE,			//�豸�Ƿ�Ϊ��ռ��
		&pDevice		//������ɵ��豸����ָ��
	);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("�����豸ʧ�ܣ������룺0x%08X\n", status));
		return status;
	}

	//��д��ʽΪֱ�Ӷ�д��ʽ
	pDevice->Flags = DO_DIRECT_IO;

	//�󶨷�������
	UNICODE_STRING symbolName = RTL_CONSTANT_STRING(NAME_SYMBOL);
	IoCreateSymbolicLink(&symbolName, &devName);

	//����ǲ����(������������)
	driver->MajorFunction[IRP_MJ_CREATE] = OnCreate;
	driver->MajorFunction[IRP_MJ_CLOSE] = OnClose;
	driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = OnDeviceIoControl;

	//SYSENTER-HOOK
	InstallSysenterHook();

	return status;
}

NTSTATUS OnCreate(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	//�����������δʹ�õĴ���
	UNREFERENCED_PARAMETER(pDevice);

	//����IRP���״̬
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	//����IRP�����˶��ٸ��ֽ�
	pIrp->IoStatus.Information = 0;
	//����IRP
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS OnClose(DEVICE_OBJECT* pDevice, IRP* pIrp)
{
	//�����������δʹ�õĴ���
	UNREFERENCED_PARAMETER(pDevice);
	KdPrint(("�豸���ر�\n"));
	//����IRP���״̬
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	//����IRP�����˶��ٸ��ֽ�
	pIrp->IoStatus.Information = 0;
	//����IRP
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID OnUnload(DRIVER_OBJECT* driver)
{
	//ж�� SYSENTER-HOOK
	UninstallSysenterHook();
	//ɾ����������
	UNICODE_STRING symbolName = RTL_CONSTANT_STRING(NAME_SYMBOL);
	IoDeleteSymbolicLink(&symbolName);
	//�����豸����
	IoDeleteDevice(driver->DeviceObject);
	uninstallHook();
	KdPrint(("������ж��\n"));

}

/*!
*  �� �� ���� PsGetProcessPeb
*  ��    �ڣ� 2020/05/22
*  �������ͣ� PPEB
*  ��    ���� PEPROCESS proc ����ִ�п��ַ
*  ��    �ܣ� ��ȡPEB�ĵ�ַ
*/
PPEB PsGetProcessPeb(PEPROCESS proc);

/*!
*  �� �� ���� FindFirstFile
*  ��    �ڣ� 2020/05/24
*  �������ͣ� NTSTATUS
*  ��    ���� const WCHAR * pszPath Ҫ���ҵ��ļ����ļ���
*  ��    ���� HANDLE * phFile ���ջ�õ��ļ����
*  ��    ���� FILE_BOTH_DIR_INFORMATION * pFileInfo �����ļ���Ϣ�Ļ�����
*  ��    ���� ULONG nInfoSize �����ļ���Ϣ�Ļ������Ĵ�С
*  ��    �ܣ� ��ȡ�ļ��е�һ���ļ�����Ϣ
*/
NTSTATUS FindFirstFile(const WCHAR* pszPath,
	HANDLE* phFile,
	FILE_BOTH_DIR_INFORMATION* pFileInfo,
	ULONG nInfoSize);

/*!
*  �� �� ���� FindNextFile
*  ��    �ڣ� 2020/05/24
*  �������ͣ� NTSTATUS
*  ��    ���� HANDLE hFile FindFirstFile��õ��ļ����
*  ��    ���� FILE_BOTH_DIR_INFORMATION * pFileInfo �����ļ���Ϣ�Ļ�����
*  ��    ���� ULONG nInfoSize �����ļ���Ϣ�Ļ������Ĵ�С
*  ��    �ܣ� ��ȡ�ļ�����һ���ļ�����Ϣ
*/
NTSTATUS FindNextFile(HANDLE hFile,
	FILE_BOTH_DIR_INFORMATION* pFileInfo,
	ULONG nInfoSize);

NTSTATUS OnDeviceIoControl(DEVICE_OBJECT* pDevice, IRP* pIrp)
{
	//��ȡ��ǰIRPջ
	PIO_STACK_LOCATION pIoStack = IoGetCurrentIrpStackLocation(pIrp);
	//��ȡ������
	ULONG uCtrlCode = pIoStack->Parameters.DeviceIoControl.IoControlCode;
	//��ȡI/O����
	PVOID pInputBuff = NULL;
	PVOID pOutBuff = NULL;

	//��ȡ���뻺����(�������)
	if (pIrp->AssociatedIrp.SystemBuffer != NULL)
	{
		pInputBuff = pIrp->AssociatedIrp.SystemBuffer;
	}
	//��ȡ���������(�������)
	if (pIrp->MdlAddress != NULL)
	{
		//��ȡMDL���������ں��е�ӳ��
		pOutBuff = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	}
	//��ȡ����I/O����Ĵ�С
	ULONG inputSize = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
	//��ȡ���I/O����Ĵ�С
	ULONG outputSize = pIoStack->Parameters.DeviceIoControl.OutputBufferLength;

	//�����ļ�·��ƴ��,���ż����ڴ濪����
	UNICODE_STRING ustrFolder = { 0 };
	WCHAR strSymbol[0x512] = L"\\??\\";
	WCHAR strSymbolLast[2] = L"\\";

	ULONG nCount = 0;
	//������Ӧ�Ŀ����������Ӧ����
	switch (uCtrlCode)
	{
	case getDriverCount:
	{
		//��ȡ������
		PLDR_DATA_TABLE_ENTRY pLdr = (PLDR_DATA_TABLE_ENTRY)pDevice->DriverObject->DriverSection;
		PLIST_ENTRY pTemp = &pLdr->InLoadOrderLinks;
		do
		{
			pTemp = pTemp->Blink;
			nCount++;
		} while (pTemp != &pLdr->InLoadOrderLinks);

		*(ULONG*)pOutBuff = nCount;

		break;
	}
	case enumDriver:
	{
		//��ȡ������
		PLDR_DATA_TABLE_ENTRY pLdr = (PLDR_DATA_TABLE_ENTRY)pDevice->DriverObject->DriverSection;
		PLIST_ENTRY pTemp = &pLdr->InLoadOrderLinks;

		//�����������ȫ��0
		RtlFillMemory(pOutBuff, outputSize, 0);

		PDRIVERINFO pOutDriverInfo = (PDRIVERINFO)pOutBuff;
		do
		{
			PLDR_DATA_TABLE_ENTRY pDriverInfo = (PLDR_DATA_TABLE_ENTRY)pTemp;
			pTemp = pTemp->Blink;
			if (pDriverInfo->DllBase != 0)
			{
				//��ȡ������
				RtlCopyMemory(pOutDriverInfo->wcDriverBasePath, pDriverInfo->BaseDllName.Buffer, pDriverInfo->BaseDllName.Length);
				//��ȡ����·��
				RtlCopyMemory(pOutDriverInfo->wcDriverFullPath, pDriverInfo->FullDllName.Buffer, pDriverInfo->FullDllName.Length);
				//��ȡ������ַ
				pOutDriverInfo->DllBase = pDriverInfo->DllBase;
				//��ȡ������С
				pOutDriverInfo->SizeOfImage = pDriverInfo->SizeOfImage;
			}

			pOutDriverInfo++;

		} while (pTemp != &pLdr->InLoadOrderLinks);

		break;
	}
	case hideDriver:
	{
		//��ȡ������
		PLDR_DATA_TABLE_ENTRY pLdr = (PLDR_DATA_TABLE_ENTRY)pDevice->DriverObject->DriverSection;
		PLIST_ENTRY pTemp = &pLdr->InLoadOrderLinks;

		//��ʼ���ַ���,��ȡҪ���ص�������
		UNICODE_STRING pHideDriverName = { 0 };
		RtlInitUnicodeString(&pHideDriverName, pInputBuff);

		do
		{
			PLDR_DATA_TABLE_ENTRY pDriverInfo = (PLDR_DATA_TABLE_ENTRY)pTemp;
			if (RtlCompareUnicodeString(&pDriverInfo->BaseDllName, &pHideDriverName, FALSE) == 0)
			{
				//�޸�Flink��Blinkָ�룬����������Ҫ���ص�����
				//ʹҪ���ص���������һ������ָ���ǰһ������Ϊ����Ҫ����������ǰһ������
				pTemp->Blink->Flink = pTemp->Flink;
				//ʹҪ���ص�������ǰһ������ָ�����һ������Ϊ����Ҫ������������һ������
				pTemp->Flink->Blink = pTemp->Blink;
				//ʹ����������LIST_ENTRY�ṹ���Flink,Blink��ָ���Լ�
				//��Ϊ�˽ڵ㱾���������У���ô���ڽӵĽڵ�������ж��ʱ��
				//ϵͳ��Ѵ˽ڵ��Flink,Blink��ָ�������ڽڵ����һ���ڵ�
				//���ǣ�����ʱ�Ѿ����������ˣ����������ԭ�����ڵĽڵ�����
				//��ж���ˣ���ô�˽ڵ��Flink,Blink���п���ָ�����õĵ�ַ
				//���������Ե�BSoD
				pTemp->Flink = (PLIST_ENTRY)&pTemp->Flink;
				pTemp->Blink = (PLIST_ENTRY)&pTemp->Blink;

				break;
			}
			pTemp = pTemp->Blink;

		} while (pTemp != &pLdr->InLoadOrderLinks);
		break;
	}
	case getProcessCount:
	{
		//��ȡ��ǰ���̵�EPROCESS
		PEPROCESS proc = PsGetCurrentProcess();
		//��ȡ�������
		PLIST_ENTRY pTemp = (PLIST_ENTRY)((ULONG)proc + 0xb8);
		do
		{
			pTemp = pTemp->Blink;
			nCount++;
		} while (pTemp != (PLIST_ENTRY)((ULONG)proc + 0xb8));

		*(ULONG*)pOutBuff = nCount;

		break;
	}
	case enumProcess:
	{
		//��ȡ��ǰ���̵�EPROCESS
		PEPROCESS proc = PsGetCurrentProcess();
		//��ȡ�������
		PLIST_ENTRY pTemp = (PLIST_ENTRY)((ULONG)proc + 0xb8);

		//�����������ȫ��0
		RtlFillMemory(pOutBuff, outputSize, 0);

		PPROCESSINFO pOutProcessInfo = (PPROCESSINFO)pOutBuff;
		do
		{
			//ѭ����ȡ��һ����̿�
			PEPROCESS pProcessInfo = (PEPROCESS)((ULONG)pTemp - 0xb8);
			pTemp = pTemp->Blink;

			//�������·���ַ����ṹ���ַ
			PUNICODE_STRING pName = (PUNICODE_STRING)(*(ULONG*)((ULONG)pProcessInfo + 0x1ec));

			//��ȡPEPROCESS��SectionObject��ֵ
			//����ֵΪ��ʱ˵���˽����Ѿ�����
			PVOID SecObj = (PVOID)(*(ULONG*)((ULONG)pProcessInfo + 0x128));
			if (pName != 0 && SecObj != NULL)
			{
				//��ȡ����ID
				pOutProcessInfo->Pid = (PVOID)(*(ULONG*)((ULONG)pProcessInfo + 0xb4));
				//��ȡ������ID
				pOutProcessInfo->PPid = (PVOID)(*(ULONG*)((ULONG)pProcessInfo + 0x140));
				//��ȡEPROCESS
				pOutProcessInfo->pEproce = (PVOID)pProcessInfo;
				//��ȡ����·��
				RtlCopyMemory(pOutProcessInfo->wcProcessFullPath, pName->Buffer, pName->Length);
			}
			pOutProcessInfo++;

		} while (pTemp != (PLIST_ENTRY)((ULONG)proc + 0xb8));

		break;
	} 
	case hideProcess:
	{
		//��ȡ��ǰ���̵�EPROCESS
		PEPROCESS proc = PsGetCurrentProcess();
		//��ȡ�������
		PLIST_ENTRY pTemp = (PLIST_ENTRY)((ULONG)proc + 0xb8);

		do
		{
			//ѭ����ȡ��һ����̿�
			PEPROCESS pProcessInfo = (PEPROCESS)((ULONG)pTemp - 0xb8);

			//��ȡPEPROCESS��SectionObject��ֵ
			//����ֵΪ��ʱ˵���˽����Ѿ�����
			PVOID SecObj = (PVOID)(*(ULONG*)((ULONG)pProcessInfo + 0x128));
			if (SecObj != NULL)
			{
				//�������Ľ���id
				ULONG pid = *(ULONG*)((ULONG)pProcessInfo + 0xb4);
				//Ҫ���صĽ���id
				ULONG fid = *(ULONG*)pInputBuff;
				//�ȽϽ���id
				if (pid == fid)
				{
					//�޸�Flink��Blinkָ�룬����������Ҫ���ص�����
					//ʹҪ���ص���������һ������ָ���ǰһ������Ϊ����Ҫ����������ǰһ������
					pTemp->Blink->Flink = pTemp->Flink;
					//ʹҪ���ص�������ǰһ������ָ�����һ������Ϊ����Ҫ������������һ������
					pTemp->Flink->Blink = pTemp->Blink;
					//ʹ����������LIST_ENTRY�ṹ���Flink,Blink��ָ���Լ�
					//��Ϊ�˽ڵ㱾���������У���ô���ڽӵĽڵ�������ж��ʱ��
					//ϵͳ��Ѵ˽ڵ��Flink,Blink��ָ�������ڽڵ����һ���ڵ�
					//���ǣ�����ʱ�Ѿ����������ˣ����������ԭ�����ڵĽڵ�����
					//��ж���ˣ���ô�˽ڵ��Flink,Blink���п���ָ�����õĵ�ַ
					//���������Ե�BSoD
					pTemp->Flink = (PLIST_ENTRY)&pTemp->Flink;
					pTemp->Blink = (PLIST_ENTRY)&pTemp->Blink;
					KdPrint(("���ؽ��̳ɹ� %ws �ɹ���\n", (UCHAR*)((ULONG)pProcessInfo + 0x16c)));
					break;
				}
			}

			pTemp = pTemp->Blink;

		} while (pTemp != (PLIST_ENTRY)((ULONG)proc + 0xb8));

		break;
	}
	case killProcess:
	{
		HANDLE hProcess = NULL;
		CLIENT_ID ClientId = { 0 };
		OBJECT_ATTRIBUTES objAttribut = { sizeof(OBJECT_ATTRIBUTES) };
		//Ҫ�����Ľ��̵�id
		ClientId.UniqueProcess = *(HANDLE*)pInputBuff;
		ClientId.UniqueThread = 0;
		//�򿪽��̣���������Ч�����������
		ZwOpenProcess(
			&hProcess,	//���ش򿪵ľ��
			1,			//����Ȩ��
			&objAttribut,	//��������
			&ClientId	//����ID�ṹ
		);
		if (hProcess)
		{
			//��������
			ZwTerminateProcess(hProcess, 0);
			//�رվ��
			ZwClose(hProcess);
		}

		break;
	}
	case getModuleCount:
	{

		//��ȡҪ�鿴ģ�����ڽ��̵Ľ��̿�EPROCESS
		PEPROCESS proc = (PEPROCESS)(*(ULONG*)pInputBuff);

		//���̹ҿ�
		KAPC_STATE ks = { 0 };
		KeStackAttachProcess(proc, &ks);

		//��ȡPEB��ַ
		PPEB peb = PsGetProcessPeb(proc);

		//��ȡģ������
		PLIST_ENTRY pTemp = (PLIST_ENTRY)(*(ULONG*)((ULONG)peb + 0xc) + 0xc);

		do
		{
			pTemp = pTemp->Blink;
			nCount++;
		} while (pTemp != (PLIST_ENTRY)(*(ULONG*)((ULONG)peb + 0xc) + 0xc));

		*(ULONG*)pOutBuff = nCount;

		//����ҿ�
		KeUnstackDetachProcess(&ks);

		break;
	}
	case enumModule:
	{
		//��ȡҪ�鿴ģ�����ڽ��̵Ľ��̿�EPROCESS
		PEPROCESS proc = (PEPROCESS)(*(ULONG*)pInputBuff);

		//���̹ҿ�
		KAPC_STATE ks = { 0 };
		KeStackAttachProcess(proc, &ks);

		//��ȡPEB��ַ
		PPEB peb = PsGetProcessPeb(proc);

		//��ȡģ������
		PLIST_ENTRY pTemp = (PLIST_ENTRY)(*(ULONG*)((ULONG)peb + 0xc) + 0xc);

		//�����������ȫ��0
		RtlFillMemory(pOutBuff, outputSize, 0);

		PMODULEINFO pOutModuleInfo = (PMODULEINFO)pOutBuff;

		do
		{
			//ѭ����ȡ��һ��ģ��
			PLDR_DATA_TABLE_ENTRY pModuleInfo = (PLDR_DATA_TABLE_ENTRY)pTemp;
			pTemp = pTemp->Blink;

			if (pModuleInfo->FullDllName.Buffer != 0)
			{
				//��ȡģ��·��
				RtlCopyMemory(pOutModuleInfo->wcModuleFullPath, pModuleInfo->FullDllName.Buffer, pModuleInfo->FullDllName.Length);
				//��ȡģ���ַ
				pOutModuleInfo->DllBase = pModuleInfo->DllBase;
				//��ȡģ���С
				pOutModuleInfo->SizeOfImage = pModuleInfo->SizeOfImage;
			}

			pOutModuleInfo++;

		} while (pTemp != (PLIST_ENTRY)(*(ULONG*)((ULONG)peb + 0xc) + 0xc));

		//����ҿ�
		KeUnstackDetachProcess(&ks);

		break;
	}
	case getThreadCount:
	{
		//��ȡҪ�鿴ģ�����ڽ��̵Ľ��̿�EPROCESS
		PEPROCESS proc = (PEPROCESS)(*(ULONG*)pInputBuff);

		//��ȡ�߳�����
		PLIST_ENTRY pTemp = (PLIST_ENTRY)((ULONG)proc + 0x188);

		do
		{
			pTemp = pTemp->Blink;
			nCount++;
		} while (pTemp != (PLIST_ENTRY)((ULONG)proc + 0x188));

		*(ULONG*)pOutBuff = nCount;

		break;
	}
	case enumThread:
	{
		//��ȡҪ�鿴ģ�����ڽ��̵Ľ��̿�EPROCESS
		PEPROCESS proc = (PEPROCESS)(*(ULONG*)pInputBuff);

		//��ȡ�߳�����
		PLIST_ENTRY pTemp = (PLIST_ENTRY)((ULONG)proc + 0x188);

		//�����������ȫ��0
		RtlFillMemory(pOutBuff, outputSize, 0);

		PTHREADINFO pOutThreadInfo = (PTHREADINFO)pOutBuff;
		do
		{
			//ѭ����ȡ��һ���߳̿�
			PETHREAD pThreadInfo = (PETHREAD)((ULONG)pTemp - 0x268);
			pTemp = pTemp->Blink;

			//��ȡ�߳�ID
			pOutThreadInfo->Tid = (PVOID)(*(PULONG)((ULONG)pThreadInfo + 0x230));
			//��ȡ�߳�ִ�п��ַ
			pOutThreadInfo->pEthread = (PVOID)pThreadInfo;
			//��ȡTeb�ṹ��ַ
			pOutThreadInfo->pTeb = (PVOID)(*(PULONG)((ULONG)pThreadInfo + 0x88));
			//��ȡ��̬���ȼ�
			pOutThreadInfo->BasePriority = (ULONG)*(CHAR*)((ULONG)pThreadInfo + 0x135);
			//��ȡ�л�����
			pOutThreadInfo->ContextSwitches = *(PULONG)((ULONG)pThreadInfo + 0x64);

			pOutThreadInfo++;

		} while (pTemp != (PLIST_ENTRY)((ULONG)proc + 0x188));

		break;
	}
	case getFileCount:
	{
		//��·����װΪ���ӷ�����
		wcscat_s(strSymbol, 0x512, (PWCHAR)pInputBuff);
		wcscat_s(strSymbol, 0x512, strSymbolLast);
		RtlInitUnicodeString(&ustrFolder, strSymbol);

		HANDLE hFile = NULL;
		//�������266*2����������ļ�����
		char buff[sizeof(FILE_BOTH_DIR_INFORMATION) + 266 * 2];
		FILE_BOTH_DIR_INFORMATION* pFileInfo = (FILE_BOTH_DIR_INFORMATION*)buff;

		NTSTATUS status = FindFirstFile(
			ustrFolder.Buffer,
			&hFile,
			pFileInfo,
			sizeof(buff));
		if (!NT_SUCCESS(status)) {
			DbgPrint("���ҵ�һ���ļ�ʧ��:0x%08X\n", status);
			return status;
		}

		do
		{
			DbgPrint("�ļ���: %ls\n", pFileInfo->FileName);
			nCount++;
		} while (STATUS_SUCCESS == FindNextFile(hFile, pFileInfo, sizeof(buff)));

		*(ULONG*)pOutBuff = nCount;

		break;
	}
	case enumFile:
	{
		//��·����װΪ���ӷ�����
		wcscat_s(strSymbol, 0x512, (PWCHAR)pInputBuff);
		wcscat_s(strSymbol, 0x512, strSymbolLast);
		RtlInitUnicodeString(&ustrFolder, strSymbol);

		//�����������ȫ��0
		RtlFillMemory(pOutBuff, outputSize, 0);

		PFILEINFO pOutFileInfo = (PFILEINFO)pOutBuff;

		HANDLE hFile = NULL;
		//�������266*2����������ļ�����
		char buff[sizeof(FILE_BOTH_DIR_INFORMATION) + 266 * 2];
		FILE_BOTH_DIR_INFORMATION* pFileInfo = (FILE_BOTH_DIR_INFORMATION*)buff;

		NTSTATUS status = FindFirstFile(
			ustrFolder.Buffer,
			&hFile,
			pFileInfo,
			sizeof(buff));
		if (!NT_SUCCESS(status)) {
			DbgPrint("���ҵ�һ���ļ�ʧ��:0x%08X\n", status);
			return status;
		}

		do
		{
			DbgPrint("�ļ���: %ls\n", pFileInfo->FileName);
			//�ļ���
			RtlCopyMemory(pOutFileInfo->wcFileName, pFileInfo->FileName, pFileInfo->FileNameLength);
			//����ʱ��
			pOutFileInfo->CreateTime = pFileInfo->CreationTime;
			//�޸�ʱ��
			pOutFileInfo->ChangeTime = pFileInfo->ChangeTime;
			//�ļ���С
			pOutFileInfo->Size = pFileInfo->EndOfFile.QuadPart;

			//�ļ���Ŀ¼�����ļ�
			if (pFileInfo->FileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				pOutFileInfo->FileOrDirectory = 0;
			}
			else
			{
				pOutFileInfo->FileOrDirectory = 1;
			}

			pOutFileInfo++;
		} while (STATUS_SUCCESS == FindNextFile(hFile, pFileInfo, sizeof(buff)));

		break;
	}
	case deleteFile:
	{
		//��·����װΪ���ӷ�����
		wcscat_s(strSymbol, 0x512, (PWCHAR)pInputBuff);
		RtlInitUnicodeString(&ustrFolder, strSymbol);

		//��ʼ��OBJECT_ATTRIBUTES������
		OBJECT_ATTRIBUTES oa = { 0 };

		InitializeObjectAttributes(
			&oa,/*Ҫ��ʼ���Ķ������Խṹ��*/
			&ustrFolder,/*�ļ�·��*/
			OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,/*����:·�������ִ�Сд,�򿪵ľ�����ں˾��*/
			NULL,
			NULL);

		//ɾ��ָ���ļ�
		ZwDeleteFile(&oa);

		break;
	}
	case getRegKeyCount:
	{
		
		// 1. �򿪴�·��.�õ�·����Ӧ�ľ��.
		NTSTATUS status;
		HANDLE hKey;
		OBJECT_ATTRIBUTES objAtt = { 0 };
		UNICODE_STRING name;
		RtlInitUnicodeString(&name, (PWCHAR)pInputBuff);
		InitializeObjectAttributes(&objAtt,
			&name,
			OBJ_CASE_INSENSITIVE,
			0,
			0);
		//��ע���
		status = ZwOpenKey(&hKey,
			GENERIC_ALL,
			&objAtt);
		if (status != STATUS_SUCCESS) {
			return status;
		}
		//��ѯVALUE�Ĵ�С
		ULONG size = 0;
		ZwQueryKey(hKey,
			KeyFullInformation,
			0,
			0,
			&size);
		KEY_FULL_INFORMATION* pKeyInfo = (KEY_FULL_INFORMATION*)
			ExAllocatePool(PagedPool, size);
		ZwQueryKey(hKey,
			KeyFullInformation,
			pKeyInfo,
			size,
			&size);

		*(ULONG*)pOutBuff = pKeyInfo->SubKeys+pKeyInfo->Values;

		ExFreePool(pKeyInfo);
		ZwClose(hKey);
		break;
	}
	case enumReg:
	{
		// 1. �򿪴�·��.�õ�·����Ӧ�ľ��.
		NTSTATUS status;
		HANDLE hKey;
		OBJECT_ATTRIBUTES objAtt = { 0 };
		UNICODE_STRING name;
		RtlInitUnicodeString(&name, (PWCHAR)pInputBuff);
		InitializeObjectAttributes(&objAtt,
			&name,
			OBJ_CASE_INSENSITIVE,
			0,
			0);
		//��ע���
		status = ZwOpenKey(&hKey,
			GENERIC_ALL,
			&objAtt);
		if (status != STATUS_SUCCESS) {
			return status;
		}
		//��ѯVALUE�Ĵ�С
		ULONG size = 0;
		ZwQueryKey(hKey,
			KeyFullInformation,
			0,
			0,
			&size);
		KEY_FULL_INFORMATION* pKeyInfo = (KEY_FULL_INFORMATION*)
			ExAllocatePool(PagedPool, size);
		ZwQueryKey(hKey,
			KeyFullInformation,
			pKeyInfo,
			size,
			&size);

		//�����������ȫ��0
		RtlFillMemory(pOutBuff, outputSize, 0);

		PREGISTER pReg = (PREGISTER)pOutBuff;
		//�������������
		for (ULONG i = 0; i < pKeyInfo->SubKeys; i++)
		{
			pReg[nCount].Type = 0;
			ZwEnumerateKey(hKey, i, KeyBasicInformation, NULL, 0, &size);
			PKEY_BASIC_INFORMATION pKeyBaseInfo = (PKEY_BASIC_INFORMATION)ExAllocatePool(PagedPool, size);
			ZwEnumerateKey(hKey, i, KeyBasicInformation, pKeyBaseInfo, size, &size);
			RtlCopyMemory(pReg[nCount].KeyName, pKeyBaseInfo->Name, pKeyBaseInfo->NameLength);
			ExFreePool(pKeyBaseInfo);
			nCount++;
		}
		for (ULONG i = 0; i < pKeyInfo->Values; i++)
		{
			pReg[nCount].Type = 1;
			//��ѯ����VALUE�Ĵ�С
			ZwEnumerateValueKey(hKey, i, KeyValueFullInformation, NULL, 0, &size);
			PKEY_VALUE_FULL_INFORMATION pValueInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool(PagedPool, size);
			//��ѯ����VALUE������
			ZwEnumerateValueKey(hKey, i, KeyValueFullInformation, pValueInfo, size, &size);
			//��ȡ��ֵ������
			RtlCopyMemory(pReg[nCount].ValueName, pValueInfo->Name, pValueInfo->NameLength);
			//��ȡֵ������
			pReg[nCount].ValueType = pValueInfo->Type;
			//��ȡֵ������
			RtlCopyMemory(pReg[nCount].Value, (PVOID)((ULONG)pValueInfo + pValueInfo->DataOffset), pValueInfo->DataLength);
			//��ȡֵ�ĳ���
			pReg[nCount].ValueLength = pValueInfo->DataLength;
			ExFreePool(pValueInfo);
			nCount++;
		}
		ExFreePool(pKeyInfo);
		ZwClose(hKey);
		break;
	}
	case newKey:
	{
		// 1. �򿪴�·��.�õ�·����Ӧ�ľ��.
		NTSTATUS status;
		HANDLE hKey;
		OBJECT_ATTRIBUTES objAtt = { 0 };
		UNICODE_STRING name;
		RtlInitUnicodeString(&name, (PWCHAR)pInputBuff);
		InitializeObjectAttributes(&objAtt,
			&name,
			OBJ_CASE_INSENSITIVE,
			0,
			0);
		//������
		status = ZwCreateKey(&hKey,
			KEY_ALL_ACCESS,
			&objAtt,
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			NULL);
		if (NT_SUCCESS(status)) {
			ZwClose(hKey);
		}
		break;
	}
	case deleteKey:
	{
		// 1. �򿪴�·��.�õ�·����Ӧ�ľ��.
		NTSTATUS status;
		HANDLE hKey;
		OBJECT_ATTRIBUTES objAtt = { 0 };
		UNICODE_STRING name;
		RtlInitUnicodeString(&name, (PWCHAR)pInputBuff);
		InitializeObjectAttributes(&objAtt,
			&name,
			OBJ_CASE_INSENSITIVE,
			0,
			0);
		//�򿪼�
		status = ZwOpenKey(&hKey,
			KEY_ALL_ACCESS,
			&objAtt);
		if (NT_SUCCESS(status)) {
			//ɾ����
			status = ZwDeleteKey(hKey);
			ZwClose(hKey);
		}
		break;
	}
	case enumIdt:
	{

		XDT_INFO sidtInfo = { 0,0,0 };
		PIDT_ENTRY pIDTEntry = NULL;

		//��ȡIDTR�Ĵ�����ֵ
		__asm sidt sidtInfo;

		//��ȡIDT�������׵�ַ
		pIDTEntry = (PIDT_ENTRY)MAKE_LONG(sidtInfo.xLowXdtBase, sidtInfo.xHighXdtBase);

		PIDTINFO pOutIdtInfo = (PIDTINFO)pOutBuff;

		//��ȡIDT��Ϣ
		for (ULONG i = 0; i < 0x100; i++)
		{
			//��������ַ
			pOutIdtInfo->pFunction = MAKE_LONG(pIDTEntry[i].uOffsetLow, pIDTEntry[i].uOffsetHigh);
			//��ѡ����
			pOutIdtInfo->Selector = pIDTEntry[i].uSelector;
			//����
			pOutIdtInfo->GateType = pIDTEntry[i].GateType;
			//��Ȩ�ȼ�
			pOutIdtInfo->Dpl = pIDTEntry[i].DPL;
			//��������
			pOutIdtInfo->ParamCount = pIDTEntry[i].paramCount;

			pOutIdtInfo++;
		}

		break;
	}
	case getGdtCount:
	{
		XDT_INFO sgdtInfo = { 0,0,0 };
		PGDT_ENTER pGdtEntry = NULL;

		//��ȡGDTR�Ĵ�����ֵ
		_asm sgdt sgdtInfo;

		//��ȡGDT�������׵�ַ
		pGdtEntry = (PGDT_ENTER)MAKE_LONG(sgdtInfo.xLowXdtBase, sgdtInfo.xHighXdtBase);
		//��ȡGDT���������
		ULONG gdtCount = sgdtInfo.uXdtLimit / 8;

		//��ȡGDT��Ϣ
		for (ULONG i = 0; i < gdtCount; i++)
		{
			//�������Ч���򲻱���
			if (pGdtEntry[i].P == 0)
			{
				continue;
			}
			nCount++;
		}

		*(ULONG*)pOutBuff = nCount;
		break;
	}
	case enumGdt:
	{
		XDT_INFO sgdtInfo = { 0,0 ,0 };
		PGDT_ENTER pGdtEntry = NULL;

		//��ȡGDTR�Ĵ�����ֵ
		__asm sgdt sgdtInfo;
		//��ȡGDT�������׵�ַ
		pGdtEntry = (PGDT_ENTER)MAKE_LONG(sgdtInfo.xLowXdtBase, sgdtInfo.xHighXdtBase);
		//��ȡGDT���������
		ULONG gdtCount = sgdtInfo.uXdtLimit / 8;

		PGDTINFO pOutGdtInfo = (PGDTINFO)pOutBuff;

		//��ȡGDT��Ϣ
		for (ULONG i = 0; i < gdtCount; i++)
		{
			//�������Ч���򲻱���
			if (pGdtEntry[i].P == 0)
			{
				continue;
			}
			//�λ�ַ
			pOutGdtInfo->BaseAddr = MAKE_LONG2(pGdtEntry[i].base0_23, pGdtEntry[i].base24_31);
			//����Ȩ�ȼ�
			pOutGdtInfo->Dpl = pGdtEntry[i].DPL;
			//������
			pOutGdtInfo->GateType = pGdtEntry[i].Type;
			//������
			pOutGdtInfo->Grain = pGdtEntry[i].G;
			//���޳�
			pOutGdtInfo->Limit = MAKE_LONG(pGdtEntry[i].Limit0_15, pGdtEntry[i].Limit16_19);
			pOutGdtInfo++;
		}

		break;
	}
	case getSsdtCount:
	{
		//��ȡSSDT���з������ĸ���
		*(ULONG*)pOutBuff = KeServiceDescriptorTable.NumberOfService;
		break;
	}
	case enumSsdt:
	{
		PSSDTINFO pOutSsdtInfo = (PSSDTINFO)pOutBuff;

		for (int i = 0; i < KeServiceDescriptorTable.NumberOfService; i++)
		{
			//������ַ
			pOutSsdtInfo->uFuntionAddr = (ULONG)KeServiceDescriptorTable.ServiceTableBase[i];
			//���ú�
			pOutSsdtInfo->uIndex = i;
			pOutSsdtInfo++;
		}
		break;
	}
	case selfPid:
	{
		g_uPid = *(ULONG*)pInputBuff;
		break;
	}
	}

	//����IRP���״̬
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	//����IRP�����˶��ٸ��ֽ�
	pIrp->IoStatus.Information = outputSize ? outputSize : inputSize;
	//����IRP
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

PPEB PsGetProcessPeb(PEPROCESS proc)
{
	PPEB res = (PPEB)(*(ULONG*)((ULONG)proc + 0x1a8));
	return res;
}

NTSTATUS FindFirstFile(const WCHAR* pszPath,
	HANDLE* phFile,
	FILE_BOTH_DIR_INFORMATION* pFileInfo,
	ULONG nInfoSize)
{
	NTSTATUS status = STATUS_SUCCESS;
	// 1. ���ļ���,�õ��ļ��е��ļ����
	HANDLE hFile = NULL;				//�����ļ����
	OBJECT_ATTRIBUTES oa = { 0 };
	UNICODE_STRING path;	//�����ļ�·��
	RtlInitUnicodeString(&path, pszPath);

	InitializeObjectAttributes(
		&oa,/*Ҫ��ʼ���Ķ������Խṹ��*/
		&path,/*�ļ�·��*/
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,/*����:·�������ִ�Сд,�򿪵ľ�����ں˾��*/
		NULL,
		NULL);

	IO_STATUS_BLOCK isb = { 0 };		//���溯���Ĳ������
	status = ZwCreateFile(
		&hFile,/*������ļ����*/
		GENERIC_READ,
		&oa,/*��������,��Ҫ��ǰ���ļ���·����ʼ����ȥ*/
		&isb,
		NULL,/*�ļ�Ԥ�����С*/
		FILE_ATTRIBUTE_NORMAL,/*�ļ�����*/
		FILE_SHARE_READ,/*����ʽ*/
		FILE_OPEN,/*��������: �������*/
		FILE_DIRECTORY_FILE,/*����ѡ��: Ŀ¼�ļ�*/
		NULL,
		0);

	if (!NT_SUCCESS(isb.Status)) {
		return isb.Status;
	}

	// 2. ͨ���ļ��е��ļ������ѯ�ļ����µ��ļ���Ϣ.
	status = ZwQueryDirectoryFile(
		hFile,
		NULL,/*�����첽IO*/
		NULL,
		NULL,
		&isb,
		pFileInfo,/*�����ļ���Ϣ�Ļ�����*/
		nInfoSize,/*���������ֽ���.*/
		FileBothDirectoryInformation,/*Ҫ��ȡ����Ϣ������*/
		TRUE,/*�Ƿ�ֻ����һ���ļ���Ϣ*/
		NULL,/*���ڹ����ļ��ı��ʽ: *.txt*/
		TRUE/*�Ƿ����¿�ʼɨ��,TRUE��Ŀ¼�еĵ�һ����Ŀ��ʼ,FALSE���ϴκ��п�ʼ�ָ�ɨ��*/
	);
	if (!NT_SUCCESS(isb.Status)) {
		return isb.Status;
	}
	// �����ļ����
	*phFile = hFile;
	return STATUS_SUCCESS;
}

NTSTATUS FindNextFile(HANDLE hFile,
	FILE_BOTH_DIR_INFORMATION* pFileInfo,
	ULONG nInfoSize)
{
	IO_STATUS_BLOCK isb = { 0 };	//���溯���Ĳ������
	ZwQueryDirectoryFile(
		hFile,
		NULL,/*�����첽IO*/
		NULL,
		NULL,
		&isb,
		pFileInfo,/*�����ļ���Ϣ�Ļ�����*/
		nInfoSize,/*���������ֽ���.*/
		FileBothDirectoryInformation,/*Ҫ��ȡ����Ϣ������*/
		TRUE,/*�Ƿ�ֻ����һ���ļ���Ϣ*/
		NULL,/*���ڹ����ļ��ı��ʽ: *.txt*/
		FALSE/*�Ƿ����¿�ʼɨ��,TRUE��Ŀ¼�еĵ�һ����Ŀ��ʼ,FALSE���ϴκ��п�ʼ�ָ�ɨ��*/
	);
	return isb.Status;
}



VOID _declspec(naked) InstallSysenterHook()
{
	__asm
	{
		push edx;						//����Ĵ���
		push eax;
		push ecx;

		// ����ԭʼ����
		mov ecx, 0x176;					// SYSTENTER_EIP_MSR�Ĵ����ı��
		rdmsr;							// ��ECXָ����MSR���ص�EDX��EAX
		mov[g_OldKiFastCallEntry], eax;	// ���ɵĵ�ַ���浽ȫ�ֱ�����

		// ���µĺ������ý�ȥ
		mov eax, MyKiFastCallEntry;
		xor edx, edx;
		wrmsr;							// ��edx:eax д��ECXָ����MSR�Ĵ�����

		pop ecx;						//�ָ��Ĵ���
		pop eax;
		pop edx;

		ret;
	}
}

VOID _declspec(naked) MyKiFastCallEntry()
{
	__asm
	{
		// �����ú�
		cmp eax, 0xBE;	//0xBE(ZwOpenProcess�ĵ��ú�)
		jne _DONE;		// ���úŲ���0xBE,ִ�е���ԭ���� KiFastCallEntry ����

		push eax;				//����Ĵ���

		mov eax, [edx + 0x14];	// eax������PCLIENT_ID
		mov eax, [eax];			// eax������PID

		cmp eax, [g_uPid];		//�ж��Ƿ���Ҫ�����Ľ��̵�ID
		pop eax;				// �ָ��Ĵ���
		jne _DONE;				// ����Ҫ�����Ľ��̾���ת

		mov[edx + 0xc], 0;		// ��Ҫ�����Ľ��̾ͽ�����Ȩ������Ϊ0���ú�����������ʧ��

	_DONE:
		// ����ԭ����KiFastCallEntry
		jmp g_OldKiFastCallEntry;
	}
}

VOID UninstallSysenterHook()
{
	__asm
	{
		push edx;
		push eax;
		push ecx;

		mov eax, [g_OldKiFastCallEntry];
		xor edx, edx;
		mov ecx, 0x176;
		wrmsr;

		pop ecx;
		pop eax;
		pop edx;
	}
}


// �ر��ڴ�ҳд�뱣��
void _declspec(naked) disablePageWriteProtect()
{
	_asm
	{
		push eax;
		mov eax, cr0;
		and eax, ~0x10000;
		mov cr0, eax;
		pop eax;
		ret;
	}
}

// �����ڴ�ҳд�뱣��
void _declspec(naked) enablePageWriteProtect()
{
	_asm
	{
		push eax;
		mov eax, cr0;
		or eax, 0x10000;
		mov cr0, eax;
		pop eax;
		ret;
	}
}

// ����NT�ں�ģ��
// ����ȡ���Ļ����������ݱ��浽pBuff��.
NTSTATUS loadNtKernelModule(UNICODE_STRING* ntkernelPath, char** pOut)
{
	NTSTATUS status = STATUS_SUCCESS;
	// 2. ��ȡ�ļ��е��ں�ģ��
	// 2.1 ���ں�ģ����Ϊ�ļ�����.
	HANDLE hFile = NULL;
	char* pBuff = NULL;
	ULONG read = 0;
	char pKernelBuff[0x1000];

	status = createFile(ntkernelPath->Buffer,
		GENERIC_READ,
		FILE_SHARE_READ,
		FILE_OPEN_IF,
		FALSE,
		&hFile);
	if (status != STATUS_SUCCESS)
	{
		KdPrint(("���ļ�ʧ��\n"));
		goto _DONE;
	}

	// 2.2 ��PE�ļ�ͷ����ȡ���ڴ�
	status = readFile(hFile, 0, 0, 0x1000, pKernelBuff, &read);
	if (STATUS_SUCCESS != status)
	{
		KdPrint(("��ȡ�ļ�����ʧ��\n"));
		goto _DONE;
	}

	// 3. ����PE�ļ����ڴ�.
	// 3.1 �õ���չͷ,��ȡӳ���С. 
	IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)pKernelBuff;
	IMAGE_NT_HEADERS* pnt = (IMAGE_NT_HEADERS*)((ULONG)pDos + pDos->e_lfanew);
	ULONG imgSize = pnt->OptionalHeader.SizeOfImage;

	// 3.2 �����ڴ��Ա���������ε�����.
	pBuff = ExAllocatePool(NonPagedPool, imgSize);
	if (pBuff == NULL)
	{
		KdPrint(("�ڴ�����ʧ��ʧ��\n"));
		status = STATUS_BUFFER_ALL_ZEROS;//��㷵�ظ�������
		goto _DONE;
	}

	// 3.2.1 ����ͷ�����ѿռ�
	RtlCopyMemory(pBuff,
		pKernelBuff,
		pnt->OptionalHeader.SizeOfHeaders);

	// 3.3 �õ�����ͷ, ������������ͷ�����ζ�ȡ���ڴ���.
	IMAGE_SECTION_HEADER* pScnHdr = IMAGE_FIRST_SECTION(pnt);
	ULONG scnCount = pnt->FileHeader.NumberOfSections;
	for (ULONG i = 0; i < scnCount; ++i)
	{
		//
		// 3.3.1 ��ȡ�ļ����ݵ��ѿռ�ָ��λ��.
		//
		status = readFile(hFile,
			pScnHdr[i].PointerToRawData,
			0,
			pScnHdr[i].SizeOfRawData,
			pScnHdr[i].VirtualAddress + pBuff,
			&read);
		if (status != STATUS_SUCCESS)
			goto _DONE;
	}

_DONE:
	ZwClose(hFile);
	//
	// �������ں˵ļ��ص��׵�ַ
	//
	*pOut = pBuff;

	if (status != STATUS_SUCCESS)
	{
		if (pBuff != NULL)
		{
			ExFreePool(pBuff);
			*pOut = pBuff = NULL;
		}
	}
	return status;
}

// �޸��ض�λ.
void fixRelocation(char* pDosHdr, ULONG curNtKernelBase)
{
	IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)pDosHdr;
	IMAGE_NT_HEADERS* pNt = (IMAGE_NT_HEADERS*)((ULONG)pDos + pDos->e_lfanew);
	ULONG uRelRva = pNt->OptionalHeader.DataDirectory[5].VirtualAddress;
	IMAGE_BASE_RELOCATION* pRel =
		(IMAGE_BASE_RELOCATION*)(uRelRva + (ULONG)pDos);

	while (pRel->SizeOfBlock)
	{
		typedef struct
		{
			USHORT offset : 12;
			USHORT type : 4;
		}TypeOffset;

		ULONG count = (pRel->SizeOfBlock - 8) / 2;
		TypeOffset* pTypeOffset = (TypeOffset*)(pRel + 1);
		for (ULONG i = 0; i < count; ++i)
		{
			if (pTypeOffset[i].type != 3)
			{
				continue;
			}

			ULONG* pFixAddr = (ULONG*)(pTypeOffset[i].offset + pRel->VirtualAddress + (ULONG)pDos);
			//
			// ��ȥĬ�ϼ��ػ�ַ
			//
			*pFixAddr -= pNt->OptionalHeader.ImageBase;

			//
			// �����µļ��ػ�ַ(ʹ�õ��ǵ�ǰ�ں˵ļ��ػ�ַ,������
			// ��Ϊ�������ں�ʹ�õ�ǰ�ں˵�����(ȫ�ֱ���,δ��ʼ������������).)
			//
			*pFixAddr += (ULONG)curNtKernelBase;
		}

		pRel = (IMAGE_BASE_RELOCATION*)((ULONG)pRel + pRel->SizeOfBlock);
	}
}

// ���SSDT��
// char* pNewBase - �¼��ص��ں˶ѿռ��׵�ַ
// char* pCurKernelBase - ��ǰ����ʹ�õ��ں˵ļ��ػ�ַ
void initSSDT(char* pNewBase, char* pCurKernelBase)
{
	// 1. �ֱ��ȡ��ǰ�ں�,�¼��ص��ں˵�`KeServiceDescriptorTable`
	//    �ĵ�ַ
	SSDTEntry* pCurSSDTEnt = &KeServiceDescriptorTable;
	g_pNewSSDTEntry = (SSDTEntry*)((ULONG)pCurSSDTEnt - (ULONG)pCurKernelBase + (ULONG)pNewBase);

	// 2. ��ȡ�¼��ص��ں��������ű�ĵ�ַ:
	// 2.1 ���������ַ
	g_pNewSSDTEntry->ServiceTableBase = (ULONG*)
		((ULONG)pCurSSDTEnt->ServiceTableBase - (ULONG)pCurKernelBase + (ULONG)pNewBase);

	// 2.3 �����������ֽ������ַ
	g_pNewSSDTEntry->ParamTableBase = (ULONG)
		((ULONG)pCurSSDTEnt->ParamTableBase - (ULONG)pCurKernelBase + (ULONG)pNewBase);

	// 2.2 ���������ô������ַ(��ʱ�������������.)
	if (pCurSSDTEnt->ServiceCounterTableBase)
	{
		g_pNewSSDTEntry->ServiceCounterTableBase = (ULONG*)
			((ULONG)pCurSSDTEnt->ServiceCounterTableBase - (ULONG)pCurKernelBase + (ULONG)pNewBase);
	}

	// 2.3 ������SSDT��ķ������
	g_pNewSSDTEntry->NumberOfService = pCurSSDTEnt->NumberOfService;

	//3. ���������ĵ�ַ��䵽��SSDT��(�ض�λʱ��ʵ�Ѿ��޸�����,
	//   ����,���޸��ض�λ��ʱ��,��ʹ�õ�ǰ�ں˵ļ��ػ�ַ��, �޸��ض�λ
	//   Ϊ֮��, ���ں˵�SSDT����ķ������ĵ�ַ���ǵ�ǰ�ں˵ĵ�ַ,
	//   ������Ҫ����Щ�������ĵ�ַ�Ļ����ں��еĺ�����ַ.)
	disablePageWriteProtect();
	for (ULONG i = 0; i < g_pNewSSDTEntry->NumberOfService; ++i)
	{
		// ��ȥ��ǰ�ں˵ļ��ػ�ַ
		g_pNewSSDTEntry->ServiceTableBase[i] -= (ULONG)pCurKernelBase;
		// �������ں˵ļ��ػ�ַ.
		g_pNewSSDTEntry->ServiceTableBase[i] += (ULONG)pNewBase;
	}
	enablePageWriteProtect();
}

void installHook()
{
	g_hookAddr = 0;

	// 1. �ҵ�KiFastCallEntry�����׵�ַ
	ULONG uKiFastCallEntry = 0;
	_asm
	{
		;// KiFastCallEntry������ַ����
		;// ������ģ��Ĵ�����0x176�żĴ�����
		push ecx;
		push eax;
		push edx;
		mov ecx, 0x176; // ���ñ��
		rdmsr; ;// ��ȡ��edx:eax
		mov uKiFastCallEntry, eax;// ���浽����
		pop edx;
		pop eax;
		pop ecx;
	}

	// 2. �ҵ�HOOK��λ��, ������5�ֽڵ�����
	// 2.1 HOOK��λ��ѡ��Ϊ(�˴�����5�ֽ�,):
	//  2be1            sub     esp, ecx ;
	// 	c1e902          shr     ecx, 2   ;
	UCHAR hookCode[5] = { 0x2b,0xe1,0xc1,0xe9,0x02 }; //����inline hook���ǵ�5�ֽ�
	ULONG i = 0;
	for (; i < 0x1FF; ++i)
	{
		if (RtlCompareMemory((UCHAR*)uKiFastCallEntry + i,
			hookCode,
			5) == 5)
		{
			break;
		}
	}
	if (i >= 0x1FF)
	{
		KdPrint(("��KiFastCallEntry������û���ҵ�HOOKλ��,����KiFastCallEntry�Ѿ���HOOK����\n"));
		uninstallHook();
		return;
	}

	g_hookAddr = uKiFastCallEntry + i;
	g_hookAddr_next_ins = g_hookAddr + 5;

	// 3. ��ʼinline hook
	UCHAR jmpCode[5] = { 0xe9 };// jmp xxxx
	disablePageWriteProtect();

	// 3.1 ������תƫ��
	// ��תƫ�� = Ŀ���ַ - ��ǰ��ַ - 5
	*(ULONG*)(jmpCode + 1) = (ULONG)myKiFastEntryHook - g_hookAddr - 5;

	// 3.2 ����תָ��д��
	RtlCopyMemory(uKiFastCallEntry + i,
		jmpCode,
		5);

	enablePageWriteProtect();
}

void uninstallHook()
{
	if (g_hookAddr)
	{

		// ��ԭʼ����д��.
		UCHAR srcCode[5] = { 0x2b,0xe1,0xc1,0xe9,0x02 };
		disablePageWriteProtect();

		_asm sti
		// 3.2 ����תָ��д��
		RtlCopyMemory(g_hookAddr,
			srcCode,
			5);

		_asm cli

		g_hookAddr = 0;
		enablePageWriteProtect();
	}

	if (g_pNewNtKernel)
	{
		ExFreePool(g_pNewNtKernel);
		g_pNewNtKernel = NULL;
	}
}

// SSDT���˺���.
ULONG SSDTFilter(ULONG index,/*������,Ҳ�ǵ��ú�*/
	ULONG tableAddress,/*��ĵ�ַ,������SSDT��ĵ�ַ,Ҳ������Shadow SSDT��ĵ�ַ*/
	ULONG funAddr/*�ӱ���ȡ���ĺ�����ַ*/)
{
	// �����SSDT��Ļ�
	if (tableAddress == KeServiceDescriptorTable.ServiceTableBase)
	{
		// �жϵ��ú�(190��ZwOpenProcess�����ĵ��ú�)
		if (index == 190)
		{
			// ������SSDT��ĺ�����ַ
			// Ҳ�������ں˵ĺ�����ַ.
			return g_pNewSSDTEntry->ServiceTableBase[190];
		}
	}
	// ���ؾɵĺ�����ַ
	return funAddr;
}

// inline hook KiFastCallEntry�ĺ���
void _declspec(naked) myKiFastEntryHook()
{

	_asm
	{
		pushad;
		pushfd;

		push edx; // �ӱ���ȡ���ĺ�����ַ
		push edi; // ��ĵ�ַ
		push eax; // ���ú�
		call SSDTFilter; // ���ù�������

		;// �����������֮��ջ�ؼ�����,ָ��pushad��
		;// 32λ��ͨ�üĴ���������ջ��,ջ�ռ䲼��Ϊ:
		;// [esp + 00] <=> eflag
		;// [esp + 04] <=> edi
		;// [esp + 08] <=> esi
		;// [esp + 0C] <=> ebp
		;// [esp + 10] <=> esp
		;// [esp + 14] <=> ebx
		;// [esp + 18] <=> edx <<-- ʹ�ú�������ֵ���޸����λ��
		;// [esp + 1C] <=> ecx
		;// [esp + 20] <=> eax
		mov dword ptr ds : [esp + 0x18], eax;
		popfd; // popfdʱ,ʵ����edx��ֵ�ͻر��޸�
		popad;

		; //ִ�б�hook���ǵ�����ָ��
		sub esp, ecx;
		shr ecx, 2;
		jmp g_hookAddr_next_ins;
	}
}

NTSTATUS createFile(wchar_t * filepath,
	ULONG access,
	ULONG share,
	ULONG openModel,
	BOOLEAN isDir,
	HANDLE * hFile)
{

	NTSTATUS status = STATUS_SUCCESS;

	IO_STATUS_BLOCK StatusBlock = { 0 };
	ULONG           ulShareAccess = share;
	ULONG ulCreateOpt = FILE_SYNCHRONOUS_IO_NONALERT;

	UNICODE_STRING path;
	RtlInitUnicodeString(&path, filepath);

	// 1. ��ʼ��OBJECT_ATTRIBUTES������
	OBJECT_ATTRIBUTES objAttrib = { 0 };
	ULONG ulAttributes = OBJ_CASE_INSENSITIVE/*�����ִ�Сд*/ | OBJ_KERNEL_HANDLE/*�ں˾��*/;
	InitializeObjectAttributes(&objAttrib,    // ���س�ʼ����ϵĽṹ��
		&path,      // �ļ���������
		ulAttributes,  // ��������
		NULL, NULL);   // һ��ΪNULL

	// 2. �����ļ�����
	ulCreateOpt |= isDir ? FILE_DIRECTORY_FILE : FILE_NON_DIRECTORY_FILE;

	status = ZwCreateFile(hFile,                 // �����ļ����
		access,				 // �ļ���������
		&objAttrib,            // OBJECT_ATTRIBUTES
		&StatusBlock,          // ���ܺ����Ĳ������
		0,                     // ��ʼ�ļ���С
		FILE_ATTRIBUTE_NORMAL, // �½��ļ�������
		ulShareAccess,         // �ļ�����ʽ
		openModel,			 // �ļ�������򿪲������򴴽�
		ulCreateOpt,           // �򿪲����ĸ��ӱ�־λ
		NULL,                  // ��չ������
		0);                    // ��չ����������
	return status;
}

NTSTATUS readFile(HANDLE hFile,
	ULONG offsetLow,
	ULONG offsetHig,
	ULONG sizeToRead,
	PVOID pBuff,
	ULONG* read)
{
	NTSTATUS status;
	IO_STATUS_BLOCK isb = { 0 };
	LARGE_INTEGER offset;
	// ����Ҫ��ȡ���ļ�ƫ��
	offset.HighPart = offsetHig;
	offset.LowPart = offsetLow;

	status = ZwReadFile(hFile,/*�ļ����*/
		NULL,/*�¼�����,�����첽IO*/
		NULL,/*APC�����֪ͨ����:�����첽IO*/
		NULL,/*���֪ͨ������ĸ��Ӳ���*/
		&isb,/*IO״̬*/
		pBuff,/*�����ļ����ݵĻ�����*/
		sizeToRead,/*Ҫ��ȡ���ֽ���*/
		&offset,/*Ҫ��ȡ���ļ�λ��*/
		NULL);
	if (status == STATUS_SUCCESS)
		*read = isb.Information;
	return status;
}