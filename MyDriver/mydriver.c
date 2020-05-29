#include <Ntifs.h>
#include <ntimage.h>


//设备名
#define NAME_DEVICE L"\\Device\\myark"
//链接符号名
#define  NAME_SYMBOL L"\\DosDevices\\myark"
//控制码
#define MYCTLCODE(code) CTL_CODE(FILE_DEVICE_UNKNOWN,0x800+(code),METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

#define MAKE_LONG(a, b)      ((LONG)(((UINT16)(((DWORD_PTR)(a)) & 0xffff)) | ((ULONG)((UINT16)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define MAKE_LONG2(a, b)      ((LONG)(((UINT64)(((DWORD_PTR)(a)) & 0xffffff)) | ((ULONG)((UINT64)(((DWORD_PTR)(b)) & 0xff))) << 24))

ULONG_PTR g_OldKiFastCallEntry;		//保存原本的 KiFastCallEntry 函数地址
ULONG g_uPid;	//要保护的进程的PID

enum MyCtlCode
{
	getDriverCount = MYCTLCODE(1),		//获取驱动数量
	enumDriver = MYCTLCODE(2),		//遍历驱动
	hideDriver = MYCTLCODE(3),		//隐藏驱动
	getProcessCount = MYCTLCODE(4),	//获取进程数量
	enumProcess = MYCTLCODE(5),		//遍历进程
	hideProcess = MYCTLCODE(6),		//隐藏进程
	killProcess = MYCTLCODE(7),		//结束进程
	getModuleCount = MYCTLCODE(8),	//获取指定进程的模块数量
	enumModule = MYCTLCODE(9),		//遍历指定进程的模块
	getThreadCount = MYCTLCODE(10),	//获取指定进程的线程数量
	enumThread = MYCTLCODE(11),		//遍历指定进程的线程
	getFileCount = MYCTLCODE(12),	//获取文件数量
	enumFile = MYCTLCODE(13),		//遍历文件
	deleteFile = MYCTLCODE(14),		//删除文件
	getRegKeyCount = MYCTLCODE(15),	//获取注册表子项数量
	enumReg = MYCTLCODE(16),		//遍历注册表
	newKey = MYCTLCODE(17),			//创建新键
	deleteKey = MYCTLCODE(18),		//删除新键
	enumIdt = MYCTLCODE(19),		//遍历IDT表
	getGdtCount = MYCTLCODE(20),	//获取GDT表中段描述符的数量
	enumGdt = MYCTLCODE(21),		//遍历GDT表
	getSsdtCount = MYCTLCODE(22),	//获取SSDT表中系统服务描述符的数量
	enumSsdt = MYCTLCODE(23),		//遍历SSDT表
	selfPid = MYCTLCODE(24),		//将自身的进程ID传到0环
};

//驱动信息结构体
typedef struct _DRIVERINFO
{
	WCHAR wcDriverBasePath[260];	//驱动名
	WCHAR wcDriverFullPath[260];	//驱动路径
	PVOID DllBase;		//加载基址
	ULONG SizeOfImage;	//大小
}DRIVERINFO, *PDRIVERINFO;

//进程信息结构体
typedef struct _PROCESSINFO
{
	WCHAR wcProcessFullPath[260];	//映像路径
	PVOID Pid;		//进程ID
	PVOID PPid;	//父进程ID
	PVOID pEproce;	//进程执行块地址
}PROCESSINFO, *PPROCESSINFO;

//模块信息结构体
typedef struct _MODULEINFO
{
	WCHAR wcModuleFullPath[260];	//模块路径
	PVOID DllBase;		//基地址
	ULONG SizeOfImage;	//大小
}MODULEINFO, *PMODULEINFO;

//线程信息结构体
typedef struct _THREADINFO
{
	PVOID Tid;		//线程ID
	PVOID pEthread;	//线程执行块地址
	PVOID pTeb;		//Teb结构地址
	ULONG BasePriority;	//静态优先级
	ULONG ContextSwitches;	//切换次数
}THREADINFO, *PTHREADINFO;

//文件信息结构体
typedef struct _FILEINFO
{
	char FileOrDirectory;	//一个字节来保存是文件还是目录,0表示目录，1表示文件
	WCHAR wcFileName[260];	//文件名
	LONGLONG Size;				//文件大小
	LARGE_INTEGER CreateTime;	//创建时间
	LARGE_INTEGER ChangeTime;	//修改时间
}FILEINFO, *PFILEINFO;

//注册表信息结构体
typedef struct _REGISTER
{
	ULONG Type;				// 类型，0 为子项，1为值
	WCHAR KeyName[256];		// 项名
	WCHAR ValueName[256];   // 值的名字
	ULONG ValueType;		// 值的类型
	UCHAR Value[256];	    // 值
	ULONG ValueLength;		//值的长度
}REGISTER, *PREGISTER;

//IDT信息结构体
typedef struct _IDTINFO
{
	ULONG pFunction;		//处理函数的地址
	ULONG Selector;			//段选择子
	ULONG ParamCount;		//参数个数
	ULONG Dpl;				//段特权级
	ULONG GateType;			//类型
}IDTINFO, *PIDTINFO;

//GDT信息结构体
typedef struct _GDTINFO
{
	ULONG BaseAddr;	//段基址
	ULONG Limit;	//段限长
	ULONG Grain;	//段粒度
	ULONG Dpl;		//特权等级
	ULONG GateType;	//类型
}GDTINFO, *PGDTINFO;

//SSDT信息结构体
typedef struct _SSDTINFO
{
	ULONG uIndex;	//回调号
	ULONG uFuntionAddr;	//函数地址
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

//GDTR与IDTR寄存器指向的结构体(X表示i或g)
typedef struct _XDT_INFO
{
	UINT16 uXdtLimit;		//Xdt范围
	UINT16 xLowXdtBase;		//Xdt低基址
	UINT16 xHighXdtBase;	//Xdt高基址
}XDT_INFO, *PXDT_INFO;

#pragma pack(1)
//IDT描述符的结构体
typedef struct _IDT_ENTRY
{
	UINT32 uOffsetLow : 16;		//处理程序低地址偏移 
	UINT32 uSelector : 16;		//段选择子
	UINT32 paramCount : 5;		//参数个数
	UINT32 none : 3;			//无，保留
	UINT32 GateType : 4;		//中断类型
	UINT32 StorageSegment : 1;	//为0是系统段，为1是数据段或代码段
	UINT32 DPL : 2;			//特权级
	UINT32 Present : 1;		//段是否有效
	UINT32 uOffsetHigh : 16;		//处理程序高地址偏移
}IDT_ENTER, *PIDT_ENTRY;

typedef struct _GDT_ENTRY
{
	UINT64 Limit0_15 : 16;	//段限长低地址偏移
	UINT64 base0_23 : 24;	//段基地址低地址偏移
	UINT64 Type : 4;		//段类型
	UINT64 S : 1;			//描述符类型（0系统段，1数据段或代码段）
	UINT64 DPL : 2;			//特权等级
	UINT64 P : 1;			//段是否有效
	UINT64 Limit16_19 : 4;	//段限长高地址偏移
	UINT64 AVL : 1;			//可供系统软件使用
	UINT64 L : 1;
	UINT64 D_B : 1;			//默认操作大小（0=16位段，1=32位段）
	UINT64 G : 1;			//粒度
	UINT64 base24_31 : 8;	//段基地址高地址偏移
}GDT_ENTER, *PGDT_ENTER;

//系统描述服务表结构体
//win7-x32 下系统导出系统描述符表
//KeServiceDescriptorTable变量，声明就可以直接使用
typedef struct _ServiceDesriptorEntry
{
	ULONG* ServiceTableBase;		//服务表基址
	ULONG* ServiceCounterTableBase;	//函数表中每个函数被调用的次数
	ULONG NumberOfService;			//服务函数的个数,
	ULONG ParamTableBase;			//参数表基址
}SSDTEntry, *PSSDTEntry;
#pragma pack()

// 导入SSDT全局变量
NTSYSAPI SSDTEntry KeServiceDescriptorTable;
static char*		g_pNewNtKernel;		// 新内核
static ULONG		g_ntKernelSize;		// 内核的映像大小
static SSDTEntry*	g_pNewSSDTEntry;	// 新ssdt的入口地址
static ULONG		g_hookAddr;			// 被hook位置的首地址
static ULONG		g_hookAddr_next_ins;// 被hook的指令的下一条指令的首地址.

//驱动对象判定的函数
NTSTATUS OnCreate(DEVICE_OBJECT *pDevice, IRP *pIrp);
NTSTATUS OnClose(DEVICE_OBJECT* pDevice, IRP* pIrp);
VOID OnUnload(DRIVER_OBJECT* driver);
NTSTATUS OnDeviceIoControl(DEVICE_OBJECT* pDevice, IRP* pIrp);

//SYSENTER-HOOK
VOID InstallSysenterHook();

//自己构造的 KiFastCallEntry 函数
VOID MyKiFastCallEntry();

//卸载 SYSENTER-HOOK
VOID UninstallSysenterHook();

// 加载NT内核模块
// 将读取到的缓冲区的内容保存到pBuff中.
NTSTATUS loadNtKernelModule(UNICODE_STRING* ntkernelPath, char** pOut);

// 修复重定位.
void fixRelocation(char* pDosHdr, ULONG curNtKernelBase);

// 填充SSDT表
// char* pNewBase - 新加载的内核堆空间首地址
// char* pCurKernelBase - 当前正在使用的内核的加载基址
void initSSDT(char* pNewBase, char* pCurKernelBase);

//安装hook
void installHook();

//卸载hook
void uninstallHook();

// inline hook KiFastCallEntry的函数
void myKiFastEntryHook();

/*!
*  函 数 名： createFile
*  日    期： 2020/05/29
*  返回类型： NTSTATUS
*  参    数： wchar_t * filepath 文件路径,路径必须是设备名"\\device\\myark\\"或符号连接名"\\??\\C:\\1.txt"
*  参    数： ULONG access 访问权限, GENERIC_READ, GENERIC_XXX
*  参    数： ULONG share 文件共享方式: FILE_SHARE_XXX
*  参    数： ULONG openModel 打开方式: FILE_OPEN_IF,FILE_CREATE ...
*  参    数： BOOLEAN isDir 是否为目录
*  参    数： HANDLE * hFile 传出的文件句柄
*  功    能： 创建文件
*/
NTSTATUS createFile(wchar_t* filepath,
	ULONG access,
	ULONG share,
	ULONG openModel,
	BOOLEAN isDir,
	HANDLE* hFile);

/*!
*  函 数 名： readFile
*  日    期： 2020/05/29
*  返回类型： NTSTATUS
*  参    数： HANDLE hFile 文件句柄
*  参    数： ULONG offsetLow 文件偏移的低32位, 从此位置开始读取
*  参    数： ULONG offsetHig 文件偏移的高32位, 从此位置开始读取
*  参    数： ULONG sizeToRead 要读取的字节数
*  参    数： PVOID pBuff 保存文件内容的缓冲区 , 需要自己申请内存空间.
*  参    数： ULONG * read 实际读取到的字节数
*  功    能： 读取文件内容
*/
NTSTATUS readFile(HANDLE hFile,
	ULONG offsetLow,
	ULONG offsetHig,
	ULONG sizeToRead,
	PVOID pBuff,
	ULONG* read);

//驱动入口函数
NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING path)
{
	//避免编译器报未使用的错误
	UNREFERENCED_PARAMETER(path);

	//返回值
	NTSTATUS status = STATUS_SUCCESS;

	///////进行内核重载/////////////////////////////////////////////////////////////////////////////

	// 1. 找到内核文件路径
	// 1.1 通过遍历内核链表的方式来找到内核主模块
	LDR_DATA_TABLE_ENTRY* pLdr = ((LDR_DATA_TABLE_ENTRY*)driver->DriverSection);
	// 1.2 内核主模块在链表中的第2项.
	for (int i = 0; i < 2; ++i)
		pLdr = (LDR_DATA_TABLE_ENTRY*)pLdr->InLoadOrderLinks.Flink;

	// 1.3 保存内核的映像大小
	g_ntKernelSize = pLdr->SizeOfImage;

	// 1.4 保存当前加载基址
	char* pCurKernelBase = (char*)pLdr->DllBase;

	// 2. 读取nt模块的文件内容到堆空间.
	status = loadNtKernelModule(&pLdr->FullDllName, &g_pNewNtKernel);
	if (STATUS_SUCCESS != status)
	{
		return status;
	}

	// 3. 修复新nt模块的重定位.
	fixRelocation(g_pNewNtKernel, (ULONG)pCurKernelBase);

	// 4. 使用当前正在使用的内核的数据来填充
	//    新内核的SSDT表.
	initSSDT(g_pNewNtKernel, pCurKernelBase);

	// 5. HOOK KiFastCallEntry,使调用号走新内核的路线
	installHook();

	///////内核重载结束/////////////////////////////////////////////////////////////////////////////

	//绑定卸载函数
	driver->DriverUnload = OnUnload;

	//创建设备
	//初始化设备对象的名字
	UNICODE_STRING devName = RTL_CONSTANT_STRING(NAME_DEVICE);
	//保存创建后的设备对象的指针
	PDEVICE_OBJECT pDevice = NULL;

	//创建设备对象
	status = IoCreateDevice(
		driver,			//驱动对象(新创建的设备对象所属驱动对象)
		0,				//设备扩展大小
		&devName,		//设备名称
		FILE_DEVICE_UNKNOWN,	//设备类型(未知类型)
		0,				//设备特征信息
		FALSE,			//设备是否为独占的
		&pDevice		//创建完成的设备对象指针
	);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("创建设备失败，错误码：0x%08X\n", status));
		return status;
	}

	//读写方式为直接读写方式
	pDevice->Flags = DO_DIRECT_IO;

	//绑定符号链接
	UNICODE_STRING symbolName = RTL_CONSTANT_STRING(NAME_SYMBOL);
	IoCreateSymbolicLink(&symbolName, &devName);

	//绑定派遣函数(在驱动对象中)
	driver->MajorFunction[IRP_MJ_CREATE] = OnCreate;
	driver->MajorFunction[IRP_MJ_CLOSE] = OnClose;
	driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = OnDeviceIoControl;

	//SYSENTER-HOOK
	InstallSysenterHook();

	return status;
}

NTSTATUS OnCreate(DEVICE_OBJECT *pDevice, IRP *pIrp)
{
	//避免编译器报未使用的错误
	UNREFERENCED_PARAMETER(pDevice);

	//设置IRP完成状态
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	//设置IRP操作了多少个字节
	pIrp->IoStatus.Information = 0;
	//处理IRP
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS OnClose(DEVICE_OBJECT* pDevice, IRP* pIrp)
{
	//避免编译器报未使用的错误
	UNREFERENCED_PARAMETER(pDevice);
	KdPrint(("设备被关闭\n"));
	//设置IRP完成状态
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	//设置IRP操作了多少个字节
	pIrp->IoStatus.Information = 0;
	//处理IRP
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID OnUnload(DRIVER_OBJECT* driver)
{
	//卸载 SYSENTER-HOOK
	UninstallSysenterHook();
	//删除符号链接
	UNICODE_STRING symbolName = RTL_CONSTANT_STRING(NAME_SYMBOL);
	IoDeleteSymbolicLink(&symbolName);
	//销毁设备对象
	IoDeleteDevice(driver->DeviceObject);
	uninstallHook();
	KdPrint(("驱动被卸载\n"));

}

/*!
*  函 数 名： PsGetProcessPeb
*  日    期： 2020/05/22
*  返回类型： PPEB
*  参    数： PEPROCESS proc 进程执行块地址
*  功    能： 获取PEB的地址
*/
PPEB PsGetProcessPeb(PEPROCESS proc);

/*!
*  函 数 名： FindFirstFile
*  日    期： 2020/05/24
*  返回类型： NTSTATUS
*  参    数： const WCHAR * pszPath 要查找的文件的文件名
*  参    数： HANDLE * phFile 接收获得的文件句柄
*  参    数： FILE_BOTH_DIR_INFORMATION * pFileInfo 接收文件信息的缓冲区
*  参    数： ULONG nInfoSize 接收文件信息的缓冲区的大小
*  功    能： 获取文件中第一个文件的信息
*/
NTSTATUS FindFirstFile(const WCHAR* pszPath,
	HANDLE* phFile,
	FILE_BOTH_DIR_INFORMATION* pFileInfo,
	ULONG nInfoSize);

/*!
*  函 数 名： FindNextFile
*  日    期： 2020/05/24
*  返回类型： NTSTATUS
*  参    数： HANDLE hFile FindFirstFile获得的文件句柄
*  参    数： FILE_BOTH_DIR_INFORMATION * pFileInfo 接收文件信息的缓冲区
*  参    数： ULONG nInfoSize 接收文件信息的缓冲区的大小
*  功    能： 获取文件中下一个文件的信息
*/
NTSTATUS FindNextFile(HANDLE hFile,
	FILE_BOTH_DIR_INFORMATION* pFileInfo,
	ULONG nInfoSize);

NTSTATUS OnDeviceIoControl(DEVICE_OBJECT* pDevice, IRP* pIrp)
{
	//获取当前IRP栈
	PIO_STACK_LOCATION pIoStack = IoGetCurrentIrpStackLocation(pIrp);
	//获取控制码
	ULONG uCtrlCode = pIoStack->Parameters.DeviceIoControl.IoControlCode;
	//获取I/O缓存
	PVOID pInputBuff = NULL;
	PVOID pOutBuff = NULL;

	//获取输入缓冲区(如果存在)
	if (pIrp->AssociatedIrp.SystemBuffer != NULL)
	{
		pInputBuff = pIrp->AssociatedIrp.SystemBuffer;
	}
	//获取输出缓冲区(如果存在)
	if (pIrp->MdlAddress != NULL)
	{
		//获取MDL缓冲区在内核中的映射
		pOutBuff = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	}
	//获取输入I/O缓存的大小
	ULONG inputSize = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
	//获取输出I/O缓存的大小
	ULONG outputSize = pIoStack->Parameters.DeviceIoControl.OutputBufferLength;

	//用于文件路径拼接,放着减少内存开销，
	UNICODE_STRING ustrFolder = { 0 };
	WCHAR strSymbol[0x512] = L"\\??\\";
	WCHAR strSymbolLast[2] = L"\\";

	ULONG nCount = 0;
	//根据相应的控制码进行相应操作
	switch (uCtrlCode)
	{
	case getDriverCount:
	{
		//获取驱动链
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
		//获取驱动链
		PLDR_DATA_TABLE_ENTRY pLdr = (PLDR_DATA_TABLE_ENTRY)pDevice->DriverObject->DriverSection;
		PLIST_ENTRY pTemp = &pLdr->InLoadOrderLinks;

		//将输出缓冲区全置0
		RtlFillMemory(pOutBuff, outputSize, 0);

		PDRIVERINFO pOutDriverInfo = (PDRIVERINFO)pOutBuff;
		do
		{
			PLDR_DATA_TABLE_ENTRY pDriverInfo = (PLDR_DATA_TABLE_ENTRY)pTemp;
			pTemp = pTemp->Blink;
			if (pDriverInfo->DllBase != 0)
			{
				//获取驱动名
				RtlCopyMemory(pOutDriverInfo->wcDriverBasePath, pDriverInfo->BaseDllName.Buffer, pDriverInfo->BaseDllName.Length);
				//获取驱动路径
				RtlCopyMemory(pOutDriverInfo->wcDriverFullPath, pDriverInfo->FullDllName.Buffer, pDriverInfo->FullDllName.Length);
				//获取驱动基址
				pOutDriverInfo->DllBase = pDriverInfo->DllBase;
				//获取驱动大小
				pOutDriverInfo->SizeOfImage = pDriverInfo->SizeOfImage;
			}

			pOutDriverInfo++;

		} while (pTemp != &pLdr->InLoadOrderLinks);

		break;
	}
	case hideDriver:
	{
		//获取驱动链
		PLDR_DATA_TABLE_ENTRY pLdr = (PLDR_DATA_TABLE_ENTRY)pDevice->DriverObject->DriverSection;
		PLIST_ENTRY pTemp = &pLdr->InLoadOrderLinks;

		//初始化字符串,获取要隐藏的驱动名
		UNICODE_STRING pHideDriverName = { 0 };
		RtlInitUnicodeString(&pHideDriverName, pInputBuff);

		do
		{
			PLDR_DATA_TABLE_ENTRY pDriverInfo = (PLDR_DATA_TABLE_ENTRY)pTemp;
			if (RtlCompareUnicodeString(&pDriverInfo->BaseDllName, &pHideDriverName, FALSE) == 0)
			{
				//修改Flink和Blink指针，以跳过我们要隐藏的驱动
				//使要隐藏的驱动的下一个驱动指向的前一个驱动为我们要隐藏驱动的前一个驱动
				pTemp->Blink->Flink = pTemp->Flink;
				//使要隐藏的驱动的前一个驱动指向的下一个驱动为我们要隐藏驱动的下一个驱动
				pTemp->Flink->Blink = pTemp->Blink;
				//使被隐藏驱动LIST_ENTRY结构体的Flink,Blink域指向自己
				//因为此节点本来在链表中，那么它邻接的节点驱动被卸载时，
				//系统会把此节点的Flink,Blink域指向它相邻节点的下一个节点
				//但是，它此时已经脱离链表了，如果现在它原本相邻的节点驱动
				//被卸载了，那么此节点的Flink,Blink域将有可能指向无用的地址
				//而造成随机性的BSoD
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
		//获取当前进程的EPROCESS
		PEPROCESS proc = PsGetCurrentProcess();
		//获取活动进程链
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
		//获取当前进程的EPROCESS
		PEPROCESS proc = PsGetCurrentProcess();
		//获取活动进程链
		PLIST_ENTRY pTemp = (PLIST_ENTRY)((ULONG)proc + 0xb8);

		//将输出缓冲区全置0
		RtlFillMemory(pOutBuff, outputSize, 0);

		PPROCESSINFO pOutProcessInfo = (PPROCESSINFO)pOutBuff;
		do
		{
			//循环获取下一块进程块
			PEPROCESS pProcessInfo = (PEPROCESS)((ULONG)pTemp - 0xb8);
			pTemp = pTemp->Blink;

			//保存进程路径字符串结构体地址
			PUNICODE_STRING pName = (PUNICODE_STRING)(*(ULONG*)((ULONG)pProcessInfo + 0x1ec));

			//获取PEPROCESS中SectionObject的值
			//当此值为空时说明此进程已经作废
			PVOID SecObj = (PVOID)(*(ULONG*)((ULONG)pProcessInfo + 0x128));
			if (pName != 0 && SecObj != NULL)
			{
				//获取进程ID
				pOutProcessInfo->Pid = (PVOID)(*(ULONG*)((ULONG)pProcessInfo + 0xb4));
				//获取父进程ID
				pOutProcessInfo->PPid = (PVOID)(*(ULONG*)((ULONG)pProcessInfo + 0x140));
				//获取EPROCESS
				pOutProcessInfo->pEproce = (PVOID)pProcessInfo;
				//获取进程路径
				RtlCopyMemory(pOutProcessInfo->wcProcessFullPath, pName->Buffer, pName->Length);
			}
			pOutProcessInfo++;

		} while (pTemp != (PLIST_ENTRY)((ULONG)proc + 0xb8));

		break;
	} 
	case hideProcess:
	{
		//获取当前进程的EPROCESS
		PEPROCESS proc = PsGetCurrentProcess();
		//获取活动进程链
		PLIST_ENTRY pTemp = (PLIST_ENTRY)((ULONG)proc + 0xb8);

		do
		{
			//循环获取下一块进程块
			PEPROCESS pProcessInfo = (PEPROCESS)((ULONG)pTemp - 0xb8);

			//获取PEPROCESS中SectionObject的值
			//当此值为空时说明此进程已经作废
			PVOID SecObj = (PVOID)(*(ULONG*)((ULONG)pProcessInfo + 0x128));
			if (SecObj != NULL)
			{
				//遍历到的进程id
				ULONG pid = *(ULONG*)((ULONG)pProcessInfo + 0xb4);
				//要隐藏的进程id
				ULONG fid = *(ULONG*)pInputBuff;
				//比较进程id
				if (pid == fid)
				{
					//修改Flink和Blink指针，以跳过我们要隐藏的驱动
					//使要隐藏的驱动的下一个驱动指向的前一个驱动为我们要隐藏驱动的前一个驱动
					pTemp->Blink->Flink = pTemp->Flink;
					//使要隐藏的驱动的前一个驱动指向的下一个驱动为我们要隐藏驱动的下一个驱动
					pTemp->Flink->Blink = pTemp->Blink;
					//使被隐藏驱动LIST_ENTRY结构体的Flink,Blink域指向自己
					//因为此节点本来在链表中，那么它邻接的节点驱动被卸载时，
					//系统会把此节点的Flink,Blink域指向它相邻节点的下一个节点
					//但是，它此时已经脱离链表了，如果现在它原本相邻的节点驱动
					//被卸载了，那么此节点的Flink,Blink域将有可能指向无用的地址
					//而造成随机性的BSoD
					pTemp->Flink = (PLIST_ENTRY)&pTemp->Flink;
					pTemp->Blink = (PLIST_ENTRY)&pTemp->Blink;
					KdPrint(("隐藏进程成功 %ws 成功！\n", (UCHAR*)((ULONG)pProcessInfo + 0x16c)));
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
		//要结束的进程的id
		ClientId.UniqueProcess = *(HANDLE*)pInputBuff;
		ClientId.UniqueThread = 0;
		//打开进程，如果句柄有效，则结束进程
		ZwOpenProcess(
			&hProcess,	//返回打开的句柄
			1,			//访问权限
			&objAttribut,	//对象属性
			&ClientId	//进程ID结构
		);
		if (hProcess)
		{
			//结束进程
			ZwTerminateProcess(hProcess, 0);
			//关闭句柄
			ZwClose(hProcess);
		}

		break;
	}
	case getModuleCount:
	{

		//获取要查看模块所在进程的进程块EPROCESS
		PEPROCESS proc = (PEPROCESS)(*(ULONG*)pInputBuff);

		//进程挂靠
		KAPC_STATE ks = { 0 };
		KeStackAttachProcess(proc, &ks);

		//获取PEB地址
		PPEB peb = PsGetProcessPeb(proc);

		//获取模块链表
		PLIST_ENTRY pTemp = (PLIST_ENTRY)(*(ULONG*)((ULONG)peb + 0xc) + 0xc);

		do
		{
			pTemp = pTemp->Blink;
			nCount++;
		} while (pTemp != (PLIST_ENTRY)(*(ULONG*)((ULONG)peb + 0xc) + 0xc));

		*(ULONG*)pOutBuff = nCount;

		//解除挂靠
		KeUnstackDetachProcess(&ks);

		break;
	}
	case enumModule:
	{
		//获取要查看模块所在进程的进程块EPROCESS
		PEPROCESS proc = (PEPROCESS)(*(ULONG*)pInputBuff);

		//进程挂靠
		KAPC_STATE ks = { 0 };
		KeStackAttachProcess(proc, &ks);

		//获取PEB地址
		PPEB peb = PsGetProcessPeb(proc);

		//获取模块链表
		PLIST_ENTRY pTemp = (PLIST_ENTRY)(*(ULONG*)((ULONG)peb + 0xc) + 0xc);

		//将输出缓冲区全置0
		RtlFillMemory(pOutBuff, outputSize, 0);

		PMODULEINFO pOutModuleInfo = (PMODULEINFO)pOutBuff;

		do
		{
			//循环获取下一块模块
			PLDR_DATA_TABLE_ENTRY pModuleInfo = (PLDR_DATA_TABLE_ENTRY)pTemp;
			pTemp = pTemp->Blink;

			if (pModuleInfo->FullDllName.Buffer != 0)
			{
				//获取模块路径
				RtlCopyMemory(pOutModuleInfo->wcModuleFullPath, pModuleInfo->FullDllName.Buffer, pModuleInfo->FullDllName.Length);
				//获取模块基址
				pOutModuleInfo->DllBase = pModuleInfo->DllBase;
				//获取模块大小
				pOutModuleInfo->SizeOfImage = pModuleInfo->SizeOfImage;
			}

			pOutModuleInfo++;

		} while (pTemp != (PLIST_ENTRY)(*(ULONG*)((ULONG)peb + 0xc) + 0xc));

		//解除挂靠
		KeUnstackDetachProcess(&ks);

		break;
	}
	case getThreadCount:
	{
		//获取要查看模块所在进程的进程块EPROCESS
		PEPROCESS proc = (PEPROCESS)(*(ULONG*)pInputBuff);

		//获取线程链表
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
		//获取要查看模块所在进程的进程块EPROCESS
		PEPROCESS proc = (PEPROCESS)(*(ULONG*)pInputBuff);

		//获取线程链表
		PLIST_ENTRY pTemp = (PLIST_ENTRY)((ULONG)proc + 0x188);

		//将输出缓冲区全置0
		RtlFillMemory(pOutBuff, outputSize, 0);

		PTHREADINFO pOutThreadInfo = (PTHREADINFO)pOutBuff;
		do
		{
			//循环获取下一块线程块
			PETHREAD pThreadInfo = (PETHREAD)((ULONG)pTemp - 0x268);
			pTemp = pTemp->Blink;

			//获取线程ID
			pOutThreadInfo->Tid = (PVOID)(*(PULONG)((ULONG)pThreadInfo + 0x230));
			//获取线程执行块地址
			pOutThreadInfo->pEthread = (PVOID)pThreadInfo;
			//获取Teb结构地址
			pOutThreadInfo->pTeb = (PVOID)(*(PULONG)((ULONG)pThreadInfo + 0x88));
			//获取静态优先级
			pOutThreadInfo->BasePriority = (ULONG)*(CHAR*)((ULONG)pThreadInfo + 0x135);
			//获取切换次数
			pOutThreadInfo->ContextSwitches = *(PULONG)((ULONG)pThreadInfo + 0x64);

			pOutThreadInfo++;

		} while (pTemp != (PLIST_ENTRY)((ULONG)proc + 0x188));

		break;
	}
	case getFileCount:
	{
		//将路径组装为链接符号名
		wcscat_s(strSymbol, 0x512, (PWCHAR)pInputBuff);
		wcscat_s(strSymbol, 0x512, strSymbolLast);
		RtlInitUnicodeString(&ustrFolder, strSymbol);

		HANDLE hFile = NULL;
		//后面加上266*2是用来存放文件名的
		char buff[sizeof(FILE_BOTH_DIR_INFORMATION) + 266 * 2];
		FILE_BOTH_DIR_INFORMATION* pFileInfo = (FILE_BOTH_DIR_INFORMATION*)buff;

		NTSTATUS status = FindFirstFile(
			ustrFolder.Buffer,
			&hFile,
			pFileInfo,
			sizeof(buff));
		if (!NT_SUCCESS(status)) {
			DbgPrint("查找第一个文件失败:0x%08X\n", status);
			return status;
		}

		do
		{
			DbgPrint("文件名: %ls\n", pFileInfo->FileName);
			nCount++;
		} while (STATUS_SUCCESS == FindNextFile(hFile, pFileInfo, sizeof(buff)));

		*(ULONG*)pOutBuff = nCount;

		break;
	}
	case enumFile:
	{
		//将路径组装为链接符号名
		wcscat_s(strSymbol, 0x512, (PWCHAR)pInputBuff);
		wcscat_s(strSymbol, 0x512, strSymbolLast);
		RtlInitUnicodeString(&ustrFolder, strSymbol);

		//将输出缓冲区全置0
		RtlFillMemory(pOutBuff, outputSize, 0);

		PFILEINFO pOutFileInfo = (PFILEINFO)pOutBuff;

		HANDLE hFile = NULL;
		//后面加上266*2是用来存放文件名的
		char buff[sizeof(FILE_BOTH_DIR_INFORMATION) + 266 * 2];
		FILE_BOTH_DIR_INFORMATION* pFileInfo = (FILE_BOTH_DIR_INFORMATION*)buff;

		NTSTATUS status = FindFirstFile(
			ustrFolder.Buffer,
			&hFile,
			pFileInfo,
			sizeof(buff));
		if (!NT_SUCCESS(status)) {
			DbgPrint("查找第一个文件失败:0x%08X\n", status);
			return status;
		}

		do
		{
			DbgPrint("文件名: %ls\n", pFileInfo->FileName);
			//文件名
			RtlCopyMemory(pOutFileInfo->wcFileName, pFileInfo->FileName, pFileInfo->FileNameLength);
			//创建时间
			pOutFileInfo->CreateTime = pFileInfo->CreationTime;
			//修改时间
			pOutFileInfo->ChangeTime = pFileInfo->ChangeTime;
			//文件大小
			pOutFileInfo->Size = pFileInfo->EndOfFile.QuadPart;

			//文件是目录还是文件
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
		//将路径组装为链接符号名
		wcscat_s(strSymbol, 0x512, (PWCHAR)pInputBuff);
		RtlInitUnicodeString(&ustrFolder, strSymbol);

		//初始化OBJECT_ATTRIBUTES的内容
		OBJECT_ATTRIBUTES oa = { 0 };

		InitializeObjectAttributes(
			&oa,/*要初始化的对象属性结构体*/
			&ustrFolder,/*文件路径*/
			OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,/*属性:路径不区分大小写,打开的句柄是内核句柄*/
			NULL,
			NULL);

		//删除指定文件
		ZwDeleteFile(&oa);

		break;
	}
	case getRegKeyCount:
	{
		
		// 1. 打开此路径.得到路径对应的句柄.
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
		//打开注册表
		status = ZwOpenKey(&hKey,
			GENERIC_ALL,
			&objAtt);
		if (status != STATUS_SUCCESS) {
			return status;
		}
		//查询VALUE的大小
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
		// 1. 打开此路径.得到路径对应的句柄.
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
		//打开注册表
		status = ZwOpenKey(&hKey,
			GENERIC_ALL,
			&objAtt);
		if (status != STATUS_SUCCESS) {
			return status;
		}
		//查询VALUE的大小
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

		//将输出缓冲区全置0
		RtlFillMemory(pOutBuff, outputSize, 0);

		PREGISTER pReg = (PREGISTER)pOutBuff;
		//遍历子项的名字
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
			//查询单个VALUE的大小
			ZwEnumerateValueKey(hKey, i, KeyValueFullInformation, NULL, 0, &size);
			PKEY_VALUE_FULL_INFORMATION pValueInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool(PagedPool, size);
			//查询单个VALUE的详情
			ZwEnumerateValueKey(hKey, i, KeyValueFullInformation, pValueInfo, size, &size);
			//获取到值的名字
			RtlCopyMemory(pReg[nCount].ValueName, pValueInfo->Name, pValueInfo->NameLength);
			//获取值的类型
			pReg[nCount].ValueType = pValueInfo->Type;
			//获取值的数据
			RtlCopyMemory(pReg[nCount].Value, (PVOID)((ULONG)pValueInfo + pValueInfo->DataOffset), pValueInfo->DataLength);
			//获取值的长度
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
		// 1. 打开此路径.得到路径对应的句柄.
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
		//创建键
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
		// 1. 打开此路径.得到路径对应的句柄.
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
		//打开键
		status = ZwOpenKey(&hKey,
			KEY_ALL_ACCESS,
			&objAtt);
		if (NT_SUCCESS(status)) {
			//删除键
			status = ZwDeleteKey(hKey);
			ZwClose(hKey);
		}
		break;
	}
	case enumIdt:
	{

		XDT_INFO sidtInfo = { 0,0,0 };
		PIDT_ENTRY pIDTEntry = NULL;

		//获取IDTR寄存器的值
		__asm sidt sidtInfo;

		//获取IDT表数组首地址
		pIDTEntry = (PIDT_ENTRY)MAKE_LONG(sidtInfo.xLowXdtBase, sidtInfo.xHighXdtBase);

		PIDTINFO pOutIdtInfo = (PIDTINFO)pOutBuff;

		//获取IDT信息
		for (ULONG i = 0; i < 0x100; i++)
		{
			//处理函数地址
			pOutIdtInfo->pFunction = MAKE_LONG(pIDTEntry[i].uOffsetLow, pIDTEntry[i].uOffsetHigh);
			//段选择子
			pOutIdtInfo->Selector = pIDTEntry[i].uSelector;
			//类型
			pOutIdtInfo->GateType = pIDTEntry[i].GateType;
			//特权等级
			pOutIdtInfo->Dpl = pIDTEntry[i].DPL;
			//参数个数
			pOutIdtInfo->ParamCount = pIDTEntry[i].paramCount;

			pOutIdtInfo++;
		}

		break;
	}
	case getGdtCount:
	{
		XDT_INFO sgdtInfo = { 0,0,0 };
		PGDT_ENTER pGdtEntry = NULL;

		//获取GDTR寄存器的值
		_asm sgdt sgdtInfo;

		//获取GDT表数组首地址
		pGdtEntry = (PGDT_ENTER)MAKE_LONG(sgdtInfo.xLowXdtBase, sgdtInfo.xHighXdtBase);
		//获取GDT表数组个数
		ULONG gdtCount = sgdtInfo.uXdtLimit / 8;

		//获取GDT信息
		for (ULONG i = 0; i < gdtCount; i++)
		{
			//如果段无效，则不遍历
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

		//获取GDTR寄存器的值
		__asm sgdt sgdtInfo;
		//获取GDT表数组首地址
		pGdtEntry = (PGDT_ENTER)MAKE_LONG(sgdtInfo.xLowXdtBase, sgdtInfo.xHighXdtBase);
		//获取GDT表数组个数
		ULONG gdtCount = sgdtInfo.uXdtLimit / 8;

		PGDTINFO pOutGdtInfo = (PGDTINFO)pOutBuff;

		//获取GDT信息
		for (ULONG i = 0; i < gdtCount; i++)
		{
			//如果段无效，则不遍历
			if (pGdtEntry[i].P == 0)
			{
				continue;
			}
			//段基址
			pOutGdtInfo->BaseAddr = MAKE_LONG2(pGdtEntry[i].base0_23, pGdtEntry[i].base24_31);
			//段特权等级
			pOutGdtInfo->Dpl = pGdtEntry[i].DPL;
			//段类型
			pOutGdtInfo->GateType = pGdtEntry[i].Type;
			//段粒度
			pOutGdtInfo->Grain = pGdtEntry[i].G;
			//段限长
			pOutGdtInfo->Limit = MAKE_LONG(pGdtEntry[i].Limit0_15, pGdtEntry[i].Limit16_19);
			pOutGdtInfo++;
		}

		break;
	}
	case getSsdtCount:
	{
		//获取SSDT表中服务函数的个数
		*(ULONG*)pOutBuff = KeServiceDescriptorTable.NumberOfService;
		break;
	}
	case enumSsdt:
	{
		PSSDTINFO pOutSsdtInfo = (PSSDTINFO)pOutBuff;

		for (int i = 0; i < KeServiceDescriptorTable.NumberOfService; i++)
		{
			//函数地址
			pOutSsdtInfo->uFuntionAddr = (ULONG)KeServiceDescriptorTable.ServiceTableBase[i];
			//调用号
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

	//设置IRP完成状态
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	//设置IRP操作了多少个字节
	pIrp->IoStatus.Information = outputSize ? outputSize : inputSize;
	//处理IRP
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
	// 1. 打开文件夹,得到文件夹的文件句柄
	HANDLE hFile = NULL;				//保存文件句柄
	OBJECT_ATTRIBUTES oa = { 0 };
	UNICODE_STRING path;	//保存文件路径
	RtlInitUnicodeString(&path, pszPath);

	InitializeObjectAttributes(
		&oa,/*要初始化的对象属性结构体*/
		&path,/*文件路径*/
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,/*属性:路径不区分大小写,打开的句柄是内核句柄*/
		NULL,
		NULL);

	IO_STATUS_BLOCK isb = { 0 };		//保存函数的操作结果
	status = ZwCreateFile(
		&hFile,/*输出的文件句柄*/
		GENERIC_READ,
		&oa,/*对象属性,需要提前将文件夹路径初始化进去*/
		&isb,
		NULL,/*文件预分配大小*/
		FILE_ATTRIBUTE_NORMAL,/*文件属性*/
		FILE_SHARE_READ,/*共享方式*/
		FILE_OPEN,/*创建描述: 存在则打开*/
		FILE_DIRECTORY_FILE,/*创建选项: 目录文件*/
		NULL,
		0);

	if (!NT_SUCCESS(isb.Status)) {
		return isb.Status;
	}

	// 2. 通过文件夹的文件句柄查询文件夹下的文件信息.
	status = ZwQueryDirectoryFile(
		hFile,
		NULL,/*用于异步IO*/
		NULL,
		NULL,
		&isb,
		pFileInfo,/*保存文件信息的缓冲区*/
		nInfoSize,/*缓冲区的字节数.*/
		FileBothDirectoryInformation,/*要获取的信息的类型*/
		TRUE,/*是否只返回一个文件信息*/
		NULL,/*用于过滤文件的表达式: *.txt*/
		TRUE/*是否重新开始扫描,TRUE从目录中的第一个条目开始,FALSE从上次呼叫开始恢复扫描*/
	);
	if (!NT_SUCCESS(isb.Status)) {
		return isb.Status;
	}
	// 传出文件句柄
	*phFile = hFile;
	return STATUS_SUCCESS;
}

NTSTATUS FindNextFile(HANDLE hFile,
	FILE_BOTH_DIR_INFORMATION* pFileInfo,
	ULONG nInfoSize)
{
	IO_STATUS_BLOCK isb = { 0 };	//保存函数的操作结果
	ZwQueryDirectoryFile(
		hFile,
		NULL,/*用于异步IO*/
		NULL,
		NULL,
		&isb,
		pFileInfo,/*保存文件信息的缓冲区*/
		nInfoSize,/*缓冲区的字节数.*/
		FileBothDirectoryInformation,/*要获取的信息的类型*/
		TRUE,/*是否只返回一个文件信息*/
		NULL,/*用于过滤文件的表达式: *.txt*/
		FALSE/*是否重新开始扫描,TRUE从目录中的第一个条目开始,FALSE从上次呼叫开始恢复扫描*/
	);
	return isb.Status;
}



VOID _declspec(naked) InstallSysenterHook()
{
	__asm
	{
		push edx;						//保存寄存器
		push eax;
		push ecx;

		// 备份原始函数
		mov ecx, 0x176;					// SYSTENTER_EIP_MSR寄存器的编号
		rdmsr;							// 将ECX指定的MSR加载到EDX：EAX
		mov[g_OldKiFastCallEntry], eax;	// 将旧的地址保存到全局变量中

		// 将新的函数设置进去
		mov eax, MyKiFastCallEntry;
		xor edx, edx;
		wrmsr;							// 将edx:eax 写入ECX指定的MSR寄存器中

		pop ecx;						//恢复寄存器
		pop eax;
		pop edx;

		ret;
	}
}

VOID _declspec(naked) MyKiFastCallEntry()
{
	__asm
	{
		// 检查调用号
		cmp eax, 0xBE;	//0xBE(ZwOpenProcess的调用号)
		jne _DONE;		// 调用号不是0xBE,执行调用原来的 KiFastCallEntry 函数

		push eax;				//保存寄存器

		mov eax, [edx + 0x14];	// eax保存了PCLIENT_ID
		mov eax, [eax];			// eax保存了PID

		cmp eax, [g_uPid];		//判断是否是要保护的进程的ID
		pop eax;				// 恢复寄存器
		jne _DONE;				// 不是要保护的进程就跳转

		mov[edx + 0xc], 0;		// 是要保护的进程就将访问权限设置为0，让后续函数调用失败

	_DONE:
		// 调用原来的KiFastCallEntry
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


// 关闭内存页写入保护
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

// 开启内存页写入保护
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

// 加载NT内核模块
// 将读取到的缓冲区的内容保存到pBuff中.
NTSTATUS loadNtKernelModule(UNICODE_STRING* ntkernelPath, char** pOut)
{
	NTSTATUS status = STATUS_SUCCESS;
	// 2. 获取文件中的内核模块
	// 2.1 将内核模块作为文件来打开.
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
		KdPrint(("打开文件失败\n"));
		goto _DONE;
	}

	// 2.2 将PE文件头部读取到内存
	status = readFile(hFile, 0, 0, 0x1000, pKernelBuff, &read);
	if (STATUS_SUCCESS != status)
	{
		KdPrint(("读取文件内容失败\n"));
		goto _DONE;
	}

	// 3. 加载PE文件到内存.
	// 3.1 得到扩展头,获取映像大小. 
	IMAGE_DOS_HEADER* pDos = (IMAGE_DOS_HEADER*)pKernelBuff;
	IMAGE_NT_HEADERS* pnt = (IMAGE_NT_HEADERS*)((ULONG)pDos + pDos->e_lfanew);
	ULONG imgSize = pnt->OptionalHeader.SizeOfImage;

	// 3.2 申请内存以保存各个区段的内容.
	pBuff = ExAllocatePool(NonPagedPool, imgSize);
	if (pBuff == NULL)
	{
		KdPrint(("内存申请失败失败\n"));
		status = STATUS_BUFFER_ALL_ZEROS;//随便返回个错误码
		goto _DONE;
	}

	// 3.2.1 拷贝头部到堆空间
	RtlCopyMemory(pBuff,
		pKernelBuff,
		pnt->OptionalHeader.SizeOfHeaders);

	// 3.3 得到区段头, 并将按照区段头将区段读取到内存中.
	IMAGE_SECTION_HEADER* pScnHdr = IMAGE_FIRST_SECTION(pnt);
	ULONG scnCount = pnt->FileHeader.NumberOfSections;
	for (ULONG i = 0; i < scnCount; ++i)
	{
		//
		// 3.3.1 读取文件内容到堆空间指定位置.
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
	// 保存新内核的加载的首地址
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

// 修复重定位.
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
			// 减去默认加载基址
			//
			*pFixAddr -= pNt->OptionalHeader.ImageBase;

			//
			// 加上新的加载基址(使用的是当前内核的加载基址,这样做
			// 是为了让新内核使用当前内核的数据(全局变量,未初始化变量等数据).)
			//
			*pFixAddr += (ULONG)curNtKernelBase;
		}

		pRel = (IMAGE_BASE_RELOCATION*)((ULONG)pRel + pRel->SizeOfBlock);
	}
}

// 填充SSDT表
// char* pNewBase - 新加载的内核堆空间首地址
// char* pCurKernelBase - 当前正在使用的内核的加载基址
void initSSDT(char* pNewBase, char* pCurKernelBase)
{
	// 1. 分别获取当前内核,新加载的内核的`KeServiceDescriptorTable`
	//    的地址
	SSDTEntry* pCurSSDTEnt = &KeServiceDescriptorTable;
	g_pNewSSDTEntry = (SSDTEntry*)((ULONG)pCurSSDTEnt - (ULONG)pCurKernelBase + (ULONG)pNewBase);

	// 2. 获取新加载的内核以下三张表的地址:
	// 2.1 服务函数表基址
	g_pNewSSDTEntry->ServiceTableBase = (ULONG*)
		((ULONG)pCurSSDTEnt->ServiceTableBase - (ULONG)pCurKernelBase + (ULONG)pNewBase);

	// 2.3 服务函数参数字节数表基址
	g_pNewSSDTEntry->ParamTableBase = (ULONG)
		((ULONG)pCurSSDTEnt->ParamTableBase - (ULONG)pCurKernelBase + (ULONG)pNewBase);

	// 2.2 服务函数调用次数表基址(有时候这个表并不存在.)
	if (pCurSSDTEnt->ServiceCounterTableBase)
	{
		g_pNewSSDTEntry->ServiceCounterTableBase = (ULONG*)
			((ULONG)pCurSSDTEnt->ServiceCounterTableBase - (ULONG)pCurKernelBase + (ULONG)pNewBase);
	}

	// 2.3 设置新SSDT表的服务个数
	g_pNewSSDTEntry->NumberOfService = pCurSSDTEnt->NumberOfService;

	//3. 将服务函数的地址填充到新SSDT表(重定位时其实已经修复好了,
	//   但是,在修复重定位的时候,是使用当前内核的加载基址的, 修复重定位
	//   为之后, 新内核的SSDT表保存的服务函数的地址都是当前内核的地址,
	//   在这里要将这些服务函数的地址改回新内核中的函数地址.)
	disablePageWriteProtect();
	for (ULONG i = 0; i < g_pNewSSDTEntry->NumberOfService; ++i)
	{
		// 减去当前内核的加载基址
		g_pNewSSDTEntry->ServiceTableBase[i] -= (ULONG)pCurKernelBase;
		// 换上新内核的加载基址.
		g_pNewSSDTEntry->ServiceTableBase[i] += (ULONG)pNewBase;
	}
	enablePageWriteProtect();
}

void installHook()
{
	g_hookAddr = 0;

	// 1. 找到KiFastCallEntry函数首地址
	ULONG uKiFastCallEntry = 0;
	_asm
	{
		;// KiFastCallEntry函数地址保存
		;// 在特殊模组寄存器的0x176号寄存器中
		push ecx;
		push eax;
		push edx;
		mov ecx, 0x176; // 设置编号
		rdmsr; ;// 读取到edx:eax
		mov uKiFastCallEntry, eax;// 保存到变量
		pop edx;
		pop eax;
		pop ecx;
	}

	// 2. 找到HOOK的位置, 并保存5字节的数据
	// 2.1 HOOK的位置选定为(此处正好5字节,):
	//  2be1            sub     esp, ecx ;
	// 	c1e902          shr     ecx, 2   ;
	UCHAR hookCode[5] = { 0x2b,0xe1,0xc1,0xe9,0x02 }; //保存inline hook覆盖的5字节
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
		KdPrint(("在KiFastCallEntry函数中没有找到HOOK位置,可能KiFastCallEntry已经被HOOK过了\n"));
		uninstallHook();
		return;
	}

	g_hookAddr = uKiFastCallEntry + i;
	g_hookAddr_next_ins = g_hookAddr + 5;

	// 3. 开始inline hook
	UCHAR jmpCode[5] = { 0xe9 };// jmp xxxx
	disablePageWriteProtect();

	// 3.1 计算跳转偏移
	// 跳转偏移 = 目标地址 - 当前地址 - 5
	*(ULONG*)(jmpCode + 1) = (ULONG)myKiFastEntryHook - g_hookAddr - 5;

	// 3.2 将跳转指令写入
	RtlCopyMemory(uKiFastCallEntry + i,
		jmpCode,
		5);

	enablePageWriteProtect();
}

void uninstallHook()
{
	if (g_hookAddr)
	{

		// 将原始数据写回.
		UCHAR srcCode[5] = { 0x2b,0xe1,0xc1,0xe9,0x02 };
		disablePageWriteProtect();

		_asm sti
		// 3.2 将跳转指令写入
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

// SSDT过滤函数.
ULONG SSDTFilter(ULONG index,/*索引号,也是调用号*/
	ULONG tableAddress,/*表的地址,可能是SSDT表的地址,也可能是Shadow SSDT表的地址*/
	ULONG funAddr/*从表中取出的函数地址*/)
{
	// 如果是SSDT表的话
	if (tableAddress == KeServiceDescriptorTable.ServiceTableBase)
	{
		// 判断调用号(190是ZwOpenProcess函数的调用号)
		if (index == 190)
		{
			// 返回新SSDT表的函数地址
			// 也就是新内核的函数地址.
			return g_pNewSSDTEntry->ServiceTableBase[190];
		}
	}
	// 返回旧的函数地址
	return funAddr;
}

// inline hook KiFastCallEntry的函数
void _declspec(naked) myKiFastEntryHook()
{

	_asm
	{
		pushad;
		pushfd;

		push edx; // 从表中取出的函数地址
		push edi; // 表的地址
		push eax; // 调用号
		call SSDTFilter; // 调用过滤桉树

		;// 函数调用完毕之后栈控件布局,指令pushad将
		;// 32位的通用寄存器保存在栈中,栈空间布局为:
		;// [esp + 00] <=> eflag
		;// [esp + 04] <=> edi
		;// [esp + 08] <=> esi
		;// [esp + 0C] <=> ebp
		;// [esp + 10] <=> esp
		;// [esp + 14] <=> ebx
		;// [esp + 18] <=> edx <<-- 使用函数返回值来修改这个位置
		;// [esp + 1C] <=> ecx
		;// [esp + 20] <=> eax
		mov dword ptr ds : [esp + 0x18], eax;
		popfd; // popfd时,实际上edx的值就回被修改
		popad;

		; //执行被hook覆盖的两条指令
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

	// 1. 初始化OBJECT_ATTRIBUTES的内容
	OBJECT_ATTRIBUTES objAttrib = { 0 };
	ULONG ulAttributes = OBJ_CASE_INSENSITIVE/*不区分大小写*/ | OBJ_KERNEL_HANDLE/*内核句柄*/;
	InitializeObjectAttributes(&objAttrib,    // 返回初始化完毕的结构体
		&path,      // 文件对象名称
		ulAttributes,  // 对象属性
		NULL, NULL);   // 一般为NULL

	// 2. 创建文件对象
	ulCreateOpt |= isDir ? FILE_DIRECTORY_FILE : FILE_NON_DIRECTORY_FILE;

	status = ZwCreateFile(hFile,                 // 返回文件句柄
		access,				 // 文件操作描述
		&objAttrib,            // OBJECT_ATTRIBUTES
		&StatusBlock,          // 接受函数的操作结果
		0,                     // 初始文件大小
		FILE_ATTRIBUTE_NORMAL, // 新建文件的属性
		ulShareAccess,         // 文件共享方式
		openModel,			 // 文件存在则打开不存在则创建
		ulCreateOpt,           // 打开操作的附加标志位
		NULL,                  // 扩展属性区
		0);                    // 扩展属性区长度
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
	// 设置要读取的文件偏移
	offset.HighPart = offsetHig;
	offset.LowPart = offsetLow;

	status = ZwReadFile(hFile,/*文件句柄*/
		NULL,/*事件对象,用于异步IO*/
		NULL,/*APC的完成通知例程:用于异步IO*/
		NULL,/*完成通知例程序的附加参数*/
		&isb,/*IO状态*/
		pBuff,/*保存文件数据的缓冲区*/
		sizeToRead,/*要读取的字节数*/
		&offset,/*要读取的文件位置*/
		NULL);
	if (status == STATUS_SUCCESS)
		*read = isb.Information;
	return status;
}