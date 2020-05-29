//Service.h
//用于与0环通信

#pragma once
#include <winioctl.h>
#include <winsvc.h>

//控制码
#define MYCTLCODE(code) CTL_CODE(FILE_DEVICE_UNKNOWN,0x800+(code),METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

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
	WCHAR wcDriverBasePath[MAX_PATH];	//驱动名
	WCHAR wcDriverFullPath[MAX_PATH];	//驱动路径
	PVOID DllBase;		//加载基址
	ULONG SizeOfImage;	//大小
}DRIVERINFO, *PDRIVERINFO;

//进程信息结构体
typedef struct _PROCESSINFO
{
	WCHAR wcProcessFullPath[MAX_PATH];	//映像路径
	PVOID Pid;		//进程ID
	PVOID PPid;	//父进程ID
	PVOID pEproce;	//进程执行块地址
}PROCESSINFO, *PPROCESSINFO;

//模块信息结构体
typedef struct _MODULEINFO
{
	WCHAR wcModuleFullPath[MAX_PATH];	//模块路径
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
	WCHAR wcFileName[MAX_PATH];	//文件名
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

class CService
{
private:
	HANDLE m_hDev;

	SC_HANDLE m_hService;
	SC_HANDLE m_hServiceMgr;

	//只能实例化一个类
	static CService* m_pService;

public:

	//获取单例，只允许当前创建一个对象
	static CService* GetService();

	//加载驱动
	BOOL LoadDriver();

	//打开设备，与0环建立连接
	BOOL StartMyService();

	//关闭设备
	BOOL CloseMyService();

	//获取驱动数量
	ULONG GetDriverCount();

	/*!
	*  函 数 名： EnumDriver
	*  日    期： 2020/05/22
	*  返回类型： VOID
	*  参    数： PVOID pBuff 用于接收所有遍历到的驱动的信息
	*  参    数： ULONG DriverCount 驱动数量
	*  功    能： 遍历驱动
	*/
	VOID EnumDriver(PVOID pBuff, ULONG DriverCount);

	/*!
	*  函 数 名： HideDriverInfo
	*  日    期： 2020/05/22
	*  返回类型： VOID
	*  参    数： char * pDriverName 驱动名
	*  功    能： 隐藏驱动
	*/
	VOID HideDriverInfo(WCHAR* pDriverName);

	//获取进程数量
	ULONG GetProcessCount();

	/*!
	*  函 数 名： EnumProcess
	*  日    期： 2020/05/22
	*  返回类型： VOID
	*  参    数： PVOID pBuff	用于接收所有遍历到的进程的信息
	*  参    数： ULONG ProcessCount 进程数量
	*  功    能： 遍历进程
	*/
	VOID EnumProcess(PVOID pBuff, ULONG ProcessCount);

	/*!
	*  函 数 名： HideProcessInfo
	*  日    期： 2020/05/22
	*  返回类型： VOID
	*  参    数： PULONG pPid 要隐藏的进程的ID
	*  功    能： 隐藏进程
	*/
	VOID HideProcessInfo(PULONG pPid);

	/*!
	*  函 数 名： KillProcess
	*  日    期： 2020/05/22
	*  返回类型： VOID
	*  参    数： PULONG pPid 要隐藏的进程的ID
	*  功    能： 结束进程
	*/
	VOID KillProcess(PULONG pPid);

	/*!
	*  函 数 名： GetModuleCount
	*  日    期： 2020/05/22
	*  返回类型： ULONG
	*  参    数： PULONG pEprocess 进程执行块地址
	*  功    能： 获取模块数量
	*/
	ULONG GetModuleCount(PULONG pEprocess);

	/*!
	*  函 数 名： EnumModule
	*  日    期： 2020/05/22
	*  返回类型： VOID
	*  参    数： PULONG pEprocess 进程执行块地址
	*  参    数： PVOID pBuff 用于接收所有遍历到的模块的信息
	*  参    数： ULONG ModuleCount 模块的数量
	*  功    能： 遍历模块
	*/
	VOID EnumModule(PULONG pEprocess, PVOID pBuff, ULONG ModuleCount);

	/*!
	*  函 数 名： GetThreadCount
	*  日    期： 2020/05/23
	*  返回类型： ULONG
	*  参    数： PULONG pEprocess 进程执行块地址
	*  功    能： 获取线程数量
	*/
	ULONG GetThreadCount(PULONG pEprocess);

	/*!
	*  函 数 名： EnumThread
	*  日    期： 2020/05/23
	*  返回类型： VOID
	*  参    数： PULONG pEprocess 进程执行块地址
	*  参    数： PVOID pBuff 用于接收所有遍历到的线程的信息
	*  参    数： ULONG ThreadCount 线程的数量
	*  功    能： 遍历线程
	*/
	VOID EnumThread(PULONG pEprocess, PVOID pBuff, ULONG ThreadCount);

	/*!
	*  函 数 名： GetFileCount
	*  日    期： 2020/05/24
	*  返回类型： ULONG
	*  参    数： PWCHAR pFileDir 目录名
	*  参    数： ULONG uFileDirLength 目录名长度
	*  功    能： 获取指定目录文件数量
	*/
	ULONG GetFileCount(PWCHAR pFileDir, ULONG uFileDirLength);

	/*!
	*  函 数 名： EnumFile
	*  日    期： 2020/05/25
	*  返回类型： VOID
	*  参    数： PWCHAR pFileDir 目录名
	*  参    数： ULONG uFileDirLength 目录名长度
	*  参    数： PVOID pBuff 用于接收所有遍历到的文件的信息
	*  参    数： ULONG FileCount 指定目录文件的数量
	*  功    能： 遍历文件
	*/
	VOID EnumFile(PWCHAR pFileDir, ULONG uFileDirLength, PVOID pBuff, ULONG FileCount);

	/*!
	*  函 数 名： MyDeleteFile
	*  日    期： 2020/05/26
	*  返回类型： VOID
	*  参    数： PWCHAR pFilePath 文件路径
	*  参    数： ULONG uFileDirLength	文件路径长度
	*  功    能： 删除文件
	*/
	VOID MyDeleteFile(PWCHAR pFilePath, ULONG uFileDirLength);

	/*!
	*  函 数 名： GetRegisterChildCount
	*  日    期： 2020/05/27
	*  返回类型： ULONG
	*  参    数： PWCHAR pRegKeyName 项名
	*  参    数： ULONG uRegKeyNameLength 项名长度
	*  功    能： 获取注册表指定键下的子键数量和数据数量
	*/
	ULONG GetRegisterChildCount(PWCHAR pRegKeyName, ULONG uRegKeyNameLength);

	/*!
	*  函 数 名： EnumReg
	*  日    期： 2020/05/27
	*  返回类型： VOID
	*  参    数： PWCHAR pRegKeyName 项名
	*  参    数： ULONG uRegKeyNameLength 项名长度
	*  参    数： PVOID pBuff 用于接收所有遍历到的注册表的信息
	*  参    数： ULONG RegCount 注册表指定键下的子键数量和数据数量
	*  功    能： 遍历注册表
	*/
	VOID EnumReg(PWCHAR pRegKeyName, ULONG uRegKeyNameLength, PVOID pBuff, ULONG RegCount);

	/*!
	*  函 数 名： NewKey
	*  日    期： 2020/05/27
	*  返回类型： VOID
	*  参    数： PWCHAR pRegKeyName 键名
	*  参    数： ULONG uRegKeyNameLength 键名长度
	*  功    能： 创建一个键
	*/
	VOID NewKey(PWCHAR pRegKeyName, ULONG uRegKeyNameLength);

	/*!
	*  函 数 名： DeleteKey
	*  日    期： 2020/05/27
	*  返回类型： VOID
	*  参    数： PWCHAR pRegKeyName 键名
	*  参    数： ULONG uRegKeyNameLength 键名长度
	*  功    能： 删除一个键
	*/
	VOID DeleteKey(PWCHAR pRegKeyName, ULONG uRegKeyNameLength);

	/*!
	*  函 数 名： EnumIdt
	*  日    期： 2020/05/27
	*  返回类型： VOID
	*  参    数： PVOID pBuff 用于接收所有遍历到的IDT表的信息
	*  功    能： 遍历IDT(中断描述符)表
	*/
	VOID EnumIdt(PVOID pBuff);

	/*!
	*  函 数 名： GetGdtCount
	*  日    期： 2020/05/27
	*  返回类型： ULONG
	*  功    能： 获取GDT表中段描述符的数量
	*/
	ULONG GetGdtCount();

	//遍历GDT表
	/*!
	*  函 数 名： EnumGdt
	*  日    期： 2020/05/28
	*  返回类型： VOID
	*  参    数： PVOID pBuff 用于接收所有遍历到的GDT表的信息
	*  参    数： ULONG GdtCount GDT表中段描述符的数量
	*  功    能： 遍历GDT表
	*/
	VOID EnumGdt(PVOID pBuff, ULONG GdtCount);

	/*!
	*  函 数 名： GetSsdtCount
	*  日    期： 2020/05/28
	*  返回类型： ULONG
	*  功    能： 获取SSDT表中服务函数的个数
	*/
	ULONG GetSsdtCount();

	/*!
	*  函 数 名： EnumSsdt
	*  日    期： 2020/05/28
	*  返回类型： VOID
	*  参    数： PVOID pBuff 用于接收所有遍历到的SSDT表的信息
	*  参    数： ULONG SsdtCount SSDT表中服务函数的个数
	*  功    能： 遍历SSDT表
	*/
	VOID EnumSsdt(PVOID pBuff, ULONG SsdtCount);

	/*!
	*  函 数 名： SendSelfPid
	*  日    期： 2020/05/28
	*  返回类型： VOID
	*  参    数： PULONG pPid 本进程PID
	*  功    能： 发送自己的PID到0环，实现自保护
	*/
	VOID SendSelfPid(PULONG pPid);
};

