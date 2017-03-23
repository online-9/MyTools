#include "stdafx.h"
#include "CLThread.h"
#include <tlhelp32.h>
#include <process.h>
#include <psapi.h>
#include "Log.h"
#pragma comment (lib, "psapi.lib")

#define _SELF L"CLThread.cpp"
CLThread::CLThread()
{
}

CLThread::~CLThread()
{
}

DWORD CLThread::GetMainThreadId()
{
	THREADENTRY32 te32 = { sizeof(te32) };
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (Thread32First(hThreadSnap, &te32))
	{
		do{
			if (::GetCurrentProcessId() == te32.th32OwnerProcessID)
			{
				DWORD dwMainThreadId = te32.th32ThreadID;
				::CloseHandle(hThreadSnap);
				return dwMainThreadId;
			}
		} while (Thread32Next(hThreadSnap, &te32));
	}
	::CloseHandle(hThreadSnap);
	return NULL;
}

DWORD CLThread::SetThreadId(DWORD dwThreadId)
{
	__try
	{
		//获取当前的线程ID
		DWORD dwNowThreadId = ::GetCurrentThreadId();
		_asm
		{
			PUSHAD
			MOV EAX, DWORD PTR FS : [0x18]
			MOV EDX, dwThreadId
			MOV DWORD PTR DS : [EAX + 0x24], EDX
			POPAD
		}

		return dwNowThreadId;
	}
	_except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"SetThreadId出现异常");
	}
	return NULL;
}

BOOL CLThread::QueryThreadInfo_By_Pid(__in DWORD dwPid, __out std::vector<CL_PROCESS_THREADINFO>& vlst)
{
	/************************************************************************/
	/* 
	extern "C" LONG(__stdcall *ZwQueryInformationThread) (
	IN HANDLE ThreadHandle,
	IN THREADINFOCLASS ThreadInformationClass,
	OUT PVOID ThreadInformation,
	IN ULONG ThreadInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	) = NULL;


	extern "C" LONG(__stdcall *RtlNtStatusToDosError) (
	IN  ULONG status) = NULL;

	HINSTANCE hNTDLL = ::GetModuleHandle(TEXT("ntdll"));

	(FARPROC&)ZwQueryInformationThread =
	::GetProcAddress(hNTDLL, "ZwQueryInformationThread");

	(FARPROC&)RtlNtStatusToDosError =
	::GetProcAddress(hNTDLL, "RtlNtStatusToDosError");


	可以直接用
	status = ZwQueryInformationThread(thread,
	ThreadQuerySetWin32StartAddress,
	&startaddr,
	sizeof (startaddr),
	NULL);
	
	*/
	/************************************************************************/
	typedef enum _THREADINFOCLASS {
		ThreadBasicInformation,
		ThreadTimes,
		ThreadPriority,
		ThreadBasePriority,
		ThreadAffinityMask,
		ThreadImpersonationToken,
		ThreadDescriptorTableEntry,
		ThreadEnableAlignmentFaultFixup,
		ThreadEventPair_Reusable,
		ThreadQuerySetWin32StartAddress,
		ThreadZeroTlsCell,
		ThreadPerformanceCount,
		ThreadAmILastThread,
		ThreadIdealProcessor,
		ThreadPriorityBoost,
		ThreadSetTlsArrayAddress,
		ThreadIsIoPending,
		ThreadHideFromDebugger,
		ThreadBreakOnTermination,
		MaxThreadInfoClass
	} THREADINFOCLASS;

	typedef struct _CLIENT_ID {
		HANDLE UniqueProcess;
		HANDLE UniqueThread;
	} CLIENT_ID;
	typedef CLIENT_ID *PCLIENT_ID;

	typedef struct _THREAD_BASIC_INFORMATION { // Information Class 0
		LONG     ExitStatus;
		PVOID    TebBaseAddress;
		CLIENT_ID ClientId;
		LONG AffinityMask;
		LONG Priority;
		LONG BasePriority;
	} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

	typedef LONG(__stdcall *ZwQueryInformationThread) (
		IN HANDLE ThreadHandle,
		IN THREADINFOCLASS ThreadInformationClass,
		OUT PVOID ThreadInformation,
		IN ULONG ThreadInformationLength,
		OUT PULONG ReturnLength OPTIONAL
		);

	ZwQueryInformationThread pZwQueryInformationThread = NULL;

	auto fnSetThreadInfo_By_Tid = [&pZwQueryInformationThread](__in DWORD dwTid, __out CL_PROCESS_THREADINFO& ThreadInfo)
	{
		THREAD_BASIC_INFORMATION    tbi = { 0 };
		PVOID                       pvStartAddr = NULL;
		LONG                        lnStatus = NULL;
		HANDLE                      hThread = NULL;
		HANDLE						hProcess = NULL;

		hThread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, dwTid);
		if (hThread == NULL)
			return FALSE;

		lnStatus = pZwQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &pvStartAddr, sizeof (pvStartAddr), NULL);
		if (lnStatus < 0)
		{
			CloseHandle(hThread);
			return FALSE;
		}

		ThreadInfo.dwTid = dwTid;
		ThreadInfo.pvStartAddr = pvStartAddr;

		lnStatus = pZwQueryInformationThread(hThread, ThreadBasicInformation, &tbi, sizeof (tbi), NULL);
		if (lnStatus < 0)
		{
			CloseHandle(hThread);
			return FALSE;
		};

		hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)tbi.ClientId.UniqueProcess);
		if (hProcess == NULL)
		{
			CloseHandle(hThread);
			return FALSE;
		};

		if (pvStartAddr != NULL)
			GetMappedFileName(hProcess, pvStartAddr, ThreadInfo.wszModuleName, MAX_PATH);

		CloseHandle(hProcess);
		CloseHandle(hThread);
		return TRUE;
	};

	try
	{
		HMODULE hmNtDLL = ::GetModuleHandleW(L"ntdll.dll");
		if (hmNtDLL == NULL)
			return FALSE;

		pZwQueryInformationThread = (ZwQueryInformationThread)::GetProcAddress(hmNtDLL, "ZwQueryInformationThread");
		if (pZwQueryInformationThread == NULL)
			return FALSE;

		CL_PROCESS_THREADINFO ThreadInfo;
		HANDLE hSnapshot = NULL;
		THREADENTRY32 te = { 0 };
		te.dwSize = sizeof (te);

		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (Thread32First(hSnapshot, &te))
		{
			do
			{
				if (te.th32OwnerProcessID == dwPid && fnSetThreadInfo_By_Tid(te.th32ThreadID, ThreadInfo))
					vlst.push_back(ThreadInfo);
			} while (Thread32Next(hSnapshot, &te));
		};
		CloseHandle(hSnapshot);
		return vlst.size() != NULL ? TRUE : FALSE;
	}
	catch (...)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"QueryThreadInfo_By_Pid出现异常");
	}
	return FALSE;
}

BOOL CLThread::TryTriminateThread(HANDLE& hThread, DWORD dwTimeOut /*= 3000*/)
{
	if (hThread != NULL)
	{
		if (::WaitForSingleObject(hThread, dwTimeOut) == WAIT_TIMEOUT)
			::TerminateThread(hThread, 0);

		::CloseHandle(hThread);
		hThread = NULL;
		return TRUE;
	}
	return FALSE;
}
