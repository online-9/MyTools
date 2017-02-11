#include "stdafx.h"
#include "CLProcess.h"
#include <Shlwapi.h>
#include "psapi.h"
#include "Character.h"
#include "CLLog.h"
#include "CLPublic.h"
#include <algorithm>

#pragma comment(lib,"Psapi.lib")

#define _SELF L"CLProcess.cpp"
CLProcess::CLProcess()
{

}

CLProcess::~CLProcess()
{

}

DWORD CLProcess::GetPid_For_ProcName(__in LPCWSTR pszText)
{
	HANDLE hThSnap32 = NULL;
	PROCESSENTRY32W pe32;

	hThSnap32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hThSnap32 == INVALID_HANDLE_VALUE)
		return NULL;

	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32FirstW(hThSnap32, &pe32))
	{
		::CloseHandle(hThSnap32);
		return NULL;
	}

	do
	{
		//�ȶԽ�����
		if (CCharacter::wstrstr_my(CCharacter::MakeTextToLower(pe32.szExeFile).c_str(), CCharacter::MakeTextToLower(pszText).c_str()))
		{
			DWORD dwPid = pe32.th32ProcessID;
			::CloseHandle(hThSnap32);
			return dwPid;
		}

	} while (Process32NextW(hThSnap32, &pe32));
	::CloseHandle(hThSnap32);
	return NULL;
}


BOOL CLProcess::Is_Exist_Process_For_ProcId(__in DWORD dwPId)
{
	HANDLE hThSnap32 = NULL;
	PROCESSENTRY32W pe32;

	hThSnap32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hThSnap32 == INVALID_HANDLE_VALUE)
		return NULL;

	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32FirstW(hThSnap32, &pe32))
	{
		::CloseHandle(hThSnap32);
		return NULL;
	}

	do
	{
		//�ȶԽ�����
		if (pe32.th32ProcessID == dwPId)
		{
			::CloseHandle(hThSnap32);
			return TRUE;
		}

	} while (Process32NextW(hThSnap32, &pe32));
	::CloseHandle(hThSnap32);
	return FALSE;
}


BOOL CLProcess::Is_Exist_Process_For_ProcName(__in LPCWSTR pszText)
{
	return CLProcess::GetPid_For_ProcName(pszText) != NULL;
}

BOOL CLProcess::TerminateProc_For_ProcName(__in LPCWSTR pszText, __in OPTIONAL DWORD dwWaitTime /* = 3000 */, _In_ DWORD dwMaxTryCount /* = 10 */)
{
	DWORD dwPid = GetPid_For_ProcName(pszText);

	UINT uCount = 0;
	while (dwPid != NULL && uCount++ <= dwMaxTryCount)
	{
		HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, dwPid);
		if ( !hProcess )
			break;//�޷��򿪽���

		TerminateProcess(hProcess, NULL);
		WaitForSingleObject(hProcess, dwWaitTime);
		::CloseHandle(hProcess);
		hProcess = NULL;
		::Sleep(1000);
		dwPid = GetPid_For_ProcName(pszText);
	}
	
	return NULL;
}

BOOL CLProcess::TerminateProc_For_ProcId(__in DWORD dwProcId, __in OPTIONAL DWORD dwWaitTime /* = 3000 */, _In_ DWORD dwMaxTryCount /* = 10 */)
{
	DWORD dwPid = dwProcId;
	UINT uCount = 0;
	while (Is_Exist_Process_For_ProcId(dwPid) && uCount++ <= dwMaxTryCount)
	{
		HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, dwPid);
		if (!hProcess)
			break;//�޷��򿪽���

		TerminateProcess(hProcess, NULL);
		WaitForSingleObject(hProcess, dwWaitTime);
		::CloseHandle(hProcess);
		hProcess = NULL;
		::Sleep(dwWaitTime);
	}

	return NULL;
}

BOOL CLProcess::GetProcessModule_For_Name(__in LPCWSTR pwchProcName, __in LPCWSTR pwchModuleName, __out MODULEENTRY32W& me32)
{
	DWORD dwPid = GetPid_For_ProcName(pwchProcName);
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);
	TL_EXIT_ERROR(hModuleSnap != INVALID_HANDLE_VALUE);


	me32.dwSize = sizeof(MODULEENTRY32W);
	if (!Module32FirstW(hModuleSnap, &me32))
	{
		::CloseHandle(hModuleSnap);
		return FALSE;
	}

	//��������ģ��,�ҵ������ģ��
	do
	{
		if (CCharacter::wstrcmp_my(pwchModuleName, me32.szModule))
		{
			::CloseHandle(hModuleSnap);
			return TRUE;
		}
	} while (Module32NextW(hModuleSnap, &me32));

Function_Exit:;
	return FALSE;
}

CL_PROCESS_INFO* CLProcess::Query_Modules_For_PID(__in DWORD Pid)
{
	HMODULE hMods[ARRAY_SIZE];
	DWORD cbNeeded;
	static CL_PROCESS_INFO ProcessInfo = { 0 };
	ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));
	if (Pid == NULL)
		return &ProcessInfo;

	//use OpenProcess to get Process Query Permissions
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, Pid);
	if (hProcess != NULL)
	{
		PROCESS_MEMORY_COUNTERS pmc;
		if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		{
			//Set Process Working Memory Size
			ProcessInfo.WorkingMemSize = pmc.WorkingSetSize;
		}

		if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
		{
			for (UINT i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i)
			{
				//Set Module Handle
				ProcessInfo.ProcessModule[i].hmModuleHandle = hMods[i];

				//Query Module FULL Path
				GetModuleFileNameExW(hProcess, ProcessInfo.ProcessModule[i].hmModuleHandle, ProcessInfo.ProcessModule[i].wchModulePath, sizeof(ProcessInfo.ProcessModule[i].wchModulePath) / sizeof(WCHAR)-1);

				//Only Module File Name
				GetModuleBaseNameW(hProcess, ProcessInfo.ProcessModule[i].hmModuleHandle, ProcessInfo.ProcessModule[i].wchModuleName, sizeof(ProcessInfo.ProcessModule[i].wchModuleName) / sizeof(WCHAR)-1);

				//Get Module Information
				GetModuleInformation(hProcess, ProcessInfo.ProcessModule[i].hmModuleHandle, (LPMODULEINFO)&ProcessInfo.ProcessModule[i].ModInfo, sizeof(MODULEINFO));

				//Set Array Size
				ProcessInfo.ModuleCount++;
			}
		}

		if (::GetCurrentProcessId() == Pid)
		{
			CCharacter::wstrcpy_my(ProcessInfo.wchCmdLine, ::GetCommandLineW(), _countof(ProcessInfo.wchCmdLine) - 1);
		}
		::CloseHandle(hProcess);
	}

	return &ProcessInfo;
}

DWORD CLProcess::Query_EnumProcesses(__out std::vector<CL_PROCESS_INFO> & vlst)
{
	DWORD aProcesses[ARRAY_SIZE] = { 0 };	//ALL Pid
	DWORD cbNeeded = 0;				//aProcesses Return Count
	DWORD cProcesses = 0;			//ALL Process Count

	//Query ALL Process
	EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded);

	//Calculation Process Count
	cProcesses = cbNeeded / sizeof(DWORD);

	//Print Every ModulePath of Process
	for (UINT i = 0; i < cProcesses; ++i)
	{
		CL_PROCESS_INFO* PI = Query_Modules_For_PID(aProcesses[i]);
		//if (PI.ModuleCount != NULL)
		vlst.push_back(*PI);
	}

	return cProcesses;
}

BOOL CLProcess::ClearWorkingMemory()
{
	return EmptyWorkingSet(::GetCurrentProcess());
}

SIZE_T CLProcess::CalcWorkSetPrivate(__in HANDLE hProcess, __in OPTIONAL SIZE_T pageSize /* = 4096 */)
{
	BOOL bRet = TRUE;
	PSAPI_WORKING_SET_INFORMATION workSetInfo;
	PBYTE pByte = NULL;
	PSAPI_WORKING_SET_BLOCK *pWorkSetBlock = workSetInfo.WorkingSetInfo;
	memset(&workSetInfo, 0, sizeof(workSetInfo));
	// Ҫ��������̵�Ȩ�ޣ�PROCESS_QUERY_INFORMATION and PROCESS_VM_READ
	// ��һ�ε��û�ȡʵ�ʻ�������С
	bRet = ::QueryWorkingSet(hProcess, &workSetInfo, sizeof(workSetInfo));
	if (!bRet) // ����ʧ��
	{
		if (GetLastError() == ERROR_BAD_LENGTH) // ��Ҫ���·��仺����
		{
			DWORD realSize = sizeof(workSetInfo.NumberOfEntries)
				+ workSetInfo.NumberOfEntries*sizeof(PSAPI_WORKING_SET_BLOCK);
			try
			{
				pByte = new BYTE[realSize];
				memset(pByte, 0, realSize);
				pWorkSetBlock = (PSAPI_WORKING_SET_BLOCK *)(pByte + sizeof(workSetInfo.NumberOfEntries));
				// ���»�ȡ
				if (!::QueryWorkingSet(hProcess, pByte, realSize))
				{
					delete[] pByte; // �����ڴ�
					return 0;
				}
			}
			catch (...) // �����ڴ�ʧ��
			{
				return 0;
			}

		}
		else // ����������Ϊ��ȡʧ��
			return 0;
	}
	SIZE_T workSetPrivate = 0;
	for (ULONG_PTR i = 0; i < workSetInfo.NumberOfEntries; ++i)
	{
		if (!pWorkSetBlock[i].Shared) // ������ǹ���ҳ
			workSetPrivate += pageSize;
	}
	if (pByte)
		delete[] pByte;
	return workSetPrivate;
}

VOID CLProcess::RaisePrivilige(__in LPCWSTR pwszPrivilegeName)
{
	//����Ȩ�ޣ�����ʵ��
	HANDLE hToken;              // ���ƾ��
	TOKEN_PRIVILEGES tkp;       // ���ƽṹָ��
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_READ, &hToken);

	LookupPrivilegeValue(NULL, pwszPrivilegeName, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;  // one privilege to set   
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

	CloseHandle(hToken);
}

BOOL CLProcess::SuspendProcess(__in DWORD dwPid, __in BOOL bSpspend /* = TRUE */)
{
	HANDLE hSpanshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwPid);

	THREADENTRY32 te = { sizeof(THREADENTRY32) };
	BOOL fOK = Thread32First(hSpanshot, &te);
	for (; fOK; fOK = Thread32Next(hSpanshot, &te))
	{
		if (te.th32OwnerProcessID == dwPid)
		{
			HANDLE hThread = ::OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
			if (hThread == NULL)
			{
				::CloseHandle(hSpanshot);
				return FALSE;
			}

			if (bSpspend)
				::SuspendThread(hThread);
			else
				::ResumeThread(hThread);

			::CloseHandle(hThread);
			::CloseHandle(hSpanshot);
			break;
		}
	}

	return TRUE;
}

BOOL CLProcess::LoadRemoteDLL(__in DWORD dwPid, __in LPCWSTR pwszDllPath)
{
	// RaisePrivilige
	RaisePrivilige(SE_DEBUG_NAME);
	RaisePrivilige(SE_SECURITY_NAME);

	// Debug Privilige
	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	PrintDebug_W(hProcess == NULL, L"OpenProcess Faild!");

	DWORD dwDLLSize = (wcslen(pwszDllPath) + 1) * 2;

	// Alloc in Target Process
	LPVOID pAllocMem = VirtualAllocEx(hProcess, NULL, dwDLLSize, MEM_COMMIT, PAGE_READWRITE);
	PrintDebug_W(pAllocMem == NULL, L"VirtualAllocEx in Target Process Faild!");

	//��DLL��·�������Ƶ�Զ�̽��̵ĵ�ַ�ռ�  
	BOOL bRetCode = WriteProcessMemory(hProcess, (PVOID)pAllocMem, (PVOID)pwszDllPath, dwDLLSize, NULL);
	PrintDebug_W(!bRetCode, L"WriteProcessMemory Faild!");

	//���LoadLibraryA��Kernel.dll�е�������ַ  
	HMODULE hmKernel32 = ::GetModuleHandle(TEXT("Kernel32"));
	PrintDebug_W(hmKernel32 == NULL, L"WriteProcessMemory Faild!");

	PTHREAD_START_ROUTINE pfnThreadRrn = (PTHREAD_START_ROUTINE)GetProcAddress(hmKernel32, "LoadLibraryW");
	PrintDebug_W(pfnThreadRrn == NULL,  L"pfnThreadRrn == NULL");

	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRrn, (PVOID)pAllocMem, 0, NULL);
	PrintDebug_W(hThread == NULL, L"hThread = NULL");
	//�ȴ�Զ���߳���ֹ  
	WaitForSingleObject(hThread, INFINITE);

	if (pAllocMem != NULL)
		VirtualFreeEx(hProcess, (PVOID)pAllocMem, 0, MEM_RELEASE);
	if (hThread != NULL)
		CloseHandle(hThread);
	if (hProcess != NULL)
		CloseHandle(hProcess);

	return TRUE;
}

BOOL CLProcess::CreateProcess_Injector_DLL(__in LPCWSTR pwszProcPath, __in LPCWSTR pwszDLLPath, __out PROCESS_INFORMATION* pPROCESS_INFORMATION)
{
	typedef struct _SHELL_CODE
	{
		char szPath[MAX_PATH];
		char szInstruction[0x20];
		_SHELL_CODE()
		{
			ZeroMemory(szPath, sizeof(szPath));
			ZeroMemory(szInstruction, sizeof(szInstruction));
		}
	} SHELL_CODE, *PSHELL_CODE;

	// init
	STARTUPINFOW		si							= { sizeof(STARTUPINFOW) };
	PROCESS_INFORMATION pi							= { 0 };
	CONTEXT				Context						= { 0 };
	WCHAR				wszProcDirectory[MAX_PATH]	= { 0 };
	WCHAR				wszProcPath[MAX_PATH]		= { 0 };
	LPVOID				Buffer						= NULL;
	SHELL_CODE			ShellCode;
	DWORD				NumberOfBytesWritten		= 0;
	CHAR				szShellCode[]				= "\x60\x68\x12\x34\x56\x78\xb8\x12\x34\x56\x78\xff\xd0\x61\xe9\x12\x34\x56\x78";
	std::string			strDllPath;

	/* szShellCode = 
	pushad
	push 0x78563412
	mov eax, 0x78563412
	call eax
	popad
	jmp 0x78563412
	*/

	//PrintDebug_W(CLPublic::FileExit(pwszProcPath), _SELF, __LINE__, L"UnExist Process Path:%s", pwszProcPath);
	PrintDebug_W(!CLPublic::FileExit(pwszDLLPath), L"UnExist Injector DLL Path:%s", pwszDLLPath);
	// CreateProcessW Parm 2 will Update Parm
	CCharacter::wstrcpy_my(wszProcPath, pwszProcPath, _countof(wszProcPath) - 1);

	// Set Current Dirctory
	CCharacter::wstrcpy_my(wszProcDirectory, pwszProcPath, _countof(wszProcDirectory) - 1);

	PrintDebug_W(!PathRemoveFileSpecW(wszProcDirectory), L"Can't Get Process Work Directory in Process Path");
	PrintDebug_W(!::CreateProcessW(NULL, wszProcPath, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi), L"CreateProcess Fiald! ErrorCode=%X", ::GetLastError());

	// Injector DLL
	Context.ContextFlags = CONTEXT_INTEGER;
	GetThreadContext(pi.hThread, &Context);
	Buffer = ::VirtualAllocEx(pi.hProcess, NULL, sizeof(SHELL_CODE), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	PrintDebug_W(Buffer == NULL,  L"VirtualAllocEx = NULL");

	*(DWORD*)(szShellCode + 2) = (DWORD)Buffer;
	*(DWORD*)(szShellCode + 7) = (DWORD)LoadLibraryA;
	*(DWORD*)(szShellCode + 15) = Context.Eax - (DWORD)((PUCHAR)Buffer + FIELD_OFFSET(SHELL_CODE, szInstruction) + sizeof(szShellCode)-1);

	strDllPath = CCharacter::UnicodeToASCII(std::wstring(pwszDLLPath));
	CopyMemory(((PSHELL_CODE)&ShellCode)->szPath, strDllPath.c_str(), strDllPath.length());
	CopyMemory(((PSHELL_CODE)&ShellCode)->szInstruction, szShellCode, sizeof(szShellCode));

	PrintDebug_W(!WriteProcessMemory(pi.hProcess, Buffer, &ShellCode, sizeof(SHELL_CODE), &NumberOfBytesWritten), L"WriteProcessMemory ShellCode Fiald!");

	Context.Eax = (DWORD)(((PSHELL_CODE)Buffer)->szInstruction);
	SetThreadContext(pi.hThread, &Context);
	ResumeThread(pi.hThread);

	if (pPROCESS_INFORMATION != nullptr)
		*pPROCESS_INFORMATION = pi;

	return TRUE;
}

UINT CLProcess::GetProcessSnapshot(__in std::vector<PROCESSENTRY32>& vlst)
{
	HANDLE hThSnap32 = NULL;
	PROCESSENTRY32W pe32;

	hThSnap32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hThSnap32 == INVALID_HANDLE_VALUE)
		return NULL;

	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32FirstW(hThSnap32, &pe32))
	{
		::CloseHandle(hThSnap32);
		return NULL;
	}

	do
	{
		vlst.push_back(pe32);
	} while (Process32NextW(hThSnap32, &pe32));
	::CloseHandle(hThSnap32);
	return vlst.size();
}

BOOL CLProcess::CreateProcess_InjectorRemoteDLL(__in LPCWSTR pwszProcPath, __in LPCWSTR pwszDLLPath, __out PROCESS_INFORMATION* pPROCESS_INFORMATION /*= nullptr*/)
{
	STARTUPINFOW		si = { sizeof(STARTUPINFOW) };
	PROCESS_INFORMATION pi = { 0 };
	WCHAR				wszProcPath[MAX_PATH] = { 0 };

	//PrintDebug_W(CLPublic::FileExit(pwszProcPath), _SELF, __LINE__, L"UnExist Process Path:%s", pwszProcPath);
	//PrintDebug_W(!CLPublic::FileExit(pwszDLLPath), L"UnExist Injector DLL Path:%s", pwszDLLPath);

	// CreateProcessW Parm 2 will Update Parm
	CCharacter::wstrcpy_my(wszProcPath, pwszProcPath, _countof(wszProcPath) - 1);
	PrintDebug_W(!::CreateProcessW(NULL, wszProcPath, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi), L"CreateProcess Fiald! ErrorCode=%X", ::GetLastError());

	if (pwszDLLPath != nullptr && CLPublic::FileExit(pwszDLLPath))
		LoadRemoteDLL(pi.dwProcessId, pwszDLLPath);

	CLPublic::SetPtr(pPROCESS_INFORMATION, pi);
	
	::ResumeThread(pi.hThread);
	::CloseHandle(pi.hProcess);
	return TRUE;
}

BOOL CLProcess::ExistProcModule_By_ID(_In_ DWORD dwProcId, _In_ LPCWSTR pwchModuleName)
{
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcId);
	TL_EXIT_ERROR(hModuleSnap != INVALID_HANDLE_VALUE);

	MODULEENTRY32W me32;
	me32.dwSize = sizeof(MODULEENTRY32W);
	if (!Module32FirstW(hModuleSnap, &me32))
	{
		::CloseHandle(hModuleSnap);
		return FALSE;
	}

	//��������ģ��,�ҵ������ģ��
	do
	{
		if (CCharacter::wstrcmp_my(pwchModuleName, me32.szModule))
		{
			::CloseHandle(hModuleSnap);
			return TRUE;
		}
	} while (Module32NextW(hModuleSnap, &me32));

Function_Exit:;
	return FALSE;
}

BOOL CLProcess::GetProcName_By_Pid(_In_ DWORD dwPid, _Out_opt_ std::wstring& wsProcName)
{
	std::vector<PROCESSENTRY32> vlst;
	GetProcessSnapshot(vlst);

	return std::find_if(vlst.begin(), vlst.end(), [&dwPid, &wsProcName](PROCESSENTRY32& PROCESSENTRY32_){
		if (PROCESSENTRY32_.th32ProcessID == dwPid)
		{
			wsProcName = PROCESSENTRY32_.szExeFile;
			return TRUE;
		}
		return FALSE;
	}) != vlst.end();
}

BOOL CLProcess::TerminateProc_By_DupHandle(_In_ DWORD dwPid)
{
	HANDLE hProcess = ::OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwPid);
	if (hProcess == NULL)
		return FALSE;

	DuplicateHandle(hProcess, INVALID_HANDLE_VALUE, GetCurrentProcess(), &hProcess, PROCESS_ALL_ACCESS, FALSE, DUPLICATE_SAME_ACCESS);
	::TerminateProcess(hProcess, 0);
	::CloseHandle(hProcess);
	return TRUE;
}

BOOL CLProcess::TerminateProc_By_UnLoad_NtDLL(_In_ DWORD dwPid)
{
	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	if (hProcess == NULL)
		return FALSE;

	typedef ULONG(WINAPI *PFNNtUnmapViewOfSection)(IN HANDLE ProcessHandle, IN PVOID BaseAddress);
	PFNNtUnmapViewOfSection pNtUnmapViewOfSection = (PFNNtUnmapViewOfSection)::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "NtUnmapViewOfSection");
	if (pNtUnmapViewOfSection == NULL)
		return FALSE;

	pNtUnmapViewOfSection(hProcess, ::GetModuleHandleW(L"ntdll.dll"));
	return TRUE;
}
