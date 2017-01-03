#ifndef __CLPROCESS_H__
#define __CLPROCESS_H__ TRUE

#include "ToolsPublic.h"
#include <tlhelp32.h>

#define ARRAY_SIZE 1024

typedef struct _CL_MODULEINFO {
	LPVOID lpBaseOfDll;
	DWORD SizeOfImage;
	LPVOID EntryPoint;
} CL_MODULEINFO, *LPCLMODULEINFO;

typedef struct _CL_PROCESS_MODULE
{
	WCHAR		wchModulePath[MAX_PATH];					//Path for Module DLL Path
	WCHAR		wchModuleName[32];							//Name for Module DLL
	HMODULE		hmModuleHandle;								//Handle for Module DLL
	CL_MODULEINFO ModInfo;									//include ImageBase, ImageSize, DLL EntryPoint
}CL_PROCESS_MODULE;

typedef struct _CL_PROCESS_INFO
{
	CL_PROCESS_MODULE	ProcessModule[ARRAY_SIZE];			//Module information details
	SIZE_T				ModuleCount;						//Module Count For Process
	SIZE_T				WorkingMemSize;						//Now Process Use Memory Size
	WCHAR				wchCmdLine[MAX_PATH];				//Only Current Process Commond Line
}CL_PROCESS_INFO;

class CLProcess
{
public:
	CLProcess();
	~CLProcess();

public:
	static UINT		GetProcessSnapshot(__in std::vector<PROCESSENTRY32>& vlst);
	static BOOL		Is_Exist_Process_For_ProcName(__in LPCWSTR pszText);										//���ݽ������жϽ����Ƿ����
	static BOOL		Is_Exist_Process_For_ProcId(__in DWORD dwPId);												//
	static DWORD	GetPid_For_ProcName(__in LPCWSTR pszText);													//���ݽ�������ȡ����ID
	static BOOL		GetProcName_By_Pid(_In_ DWORD dwPid, _Out_opt_ std::wstring& wsProcName);
	static BOOL		TerminateProc_For_ProcName(__in LPCWSTR pszText, __in OPTIONAL DWORD dwWaitTime = 3000, _In_ DWORD dwMaxTryCount = 10);	//���ݽ������ر�������صĽ���,���ҵȴ�N��
	static BOOL		TerminateProc_For_ProcId(__in DWORD dwProcId, __in OPTIONAL DWORD dwWaitTime = 3000, _In_ DWORD dwMaxTryCount = 10);		//���ݽ���ID�ر�������صĽ���,���ҵȴ�N��
	static BOOL		GetProcessModule_For_Name(__in LPCWSTR pwchProcName, __in LPCWSTR pwchModuleName, __out MODULEENTRY32W& me32);//���ؽ���ģ����Ϣ
	static BOOL		ExistProcModule_By_ID(_In_ DWORD dwProcId, _In_ LPCWSTR pwchModuleName);
	static CL_PROCESS_INFO* Query_Modules_For_PID(__in DWORD Pid);
	static DWORD	Query_EnumProcesses(__out std::vector<CL_PROCESS_INFO> & vlst);
	static BOOL		ClearWorkingMemory();																		//Clear Working Memory
	static SIZE_T	CalcWorkSetPrivate(__in HANDLE hProcess, __in OPTIONAL SIZE_T pageSize = 4096);				//Query Working Memory
	static VOID		RaisePrivilige(__in LPCWSTR pwszPrivilegeName);												//SE_DEBUG_NAME || SE_SECURITY_NAME
	static BOOL		SuspendProcess(__in DWORD dwPid, __in BOOL bSpspend = TRUE);
	static BOOL		LoadRemoteDLL(__in DWORD dwPid, __in LPCWSTR pwszDllPath);
	static BOOL		CreateProcess_Injector_DLL(__in LPCWSTR pwszProcPath, __in LPCWSTR pwszDLLPath, __out PROCESS_INFORMATION* pPROCESS_INFORMATION = nullptr);
	static BOOL		CreateProcess_InjectorRemoteDLL(__in LPCWSTR pwszProcPath, __in LPCWSTR pwszDLLPath, __out PROCESS_INFORMATION* pPROCESS_INFORMATION = nullptr);
private:

};

#endif