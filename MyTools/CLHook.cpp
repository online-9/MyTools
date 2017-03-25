#include "stdafx.h"
#include "CLHook.h"
#include <algorithm>
#include "Log.h"
#include "Character.h"
#include "CLPublic.h"
#define _SELF L"CLHook.cpp"
#define _SELF_A "CLHook.cpp"

CLHook::CLHook()
{
}

CLHook::~CLHook()
{
}

HOOK_ITEM HookItem = { 0 };
#ifdef _WIN64
#else
BOOL CLHook::Hook_Fun_Jmp_MyAddr(MYHOOK_CONTENT* pHookContent)
{
	__try
	{
		if (pHookContent == nullptr)
			return FALSE;


		// 初始化变量
		BYTE bJmpFlag = 0xE9;
		BYTE bNopFlag = 0x90;
		BOOL  bRetCode = FALSE;
		DWORD dwOldProtent = NULL;
		HANDLE curProcess = ::GetCurrentProcess();

		// 计算内存偏移地址
		DWORD dwJmpAddr = pHookContent->dwFunAddr - pHookContent->dwHookAddr - 5;

		// 获取旧的代码
		pHookContent->dwOldCode1 = CCharacter::ReadDWORD(pHookContent->dwHookAddr + 0x0);
		pHookContent->dwOldCode2 = CCharacter::ReadDWORD(pHookContent->dwHookAddr + 0x4);

		// 修改内存页属性,不然可能不让写的权限
		::VirtualProtect((LPVOID)pHookContent->dwHookAddr, 8, PAGE_EXECUTE_READWRITE, &dwOldProtent);

		// 写入Jmp指令
		//*(BYTE*)pHookContent->dwHookAddr = bJmpFlag;
		::WriteProcessMemory(curProcess, (LPVOID)pHookContent->dwHookAddr, &bJmpFlag, 1, 0);

		// 写入自己代码的Addr
		//*(DWORD*)(pHookContent->dwHookAddr + 1) = dwJmpAddr;
		bRetCode = ::WriteProcessMemory(curProcess, (LPVOID)(pHookContent->dwHookAddr + 1), &dwJmpAddr, 4, 0);

		// Nop掉多余的代码
		for (UINT i = 0; i < pHookContent->uNopCount; ++i)
		{
			::WriteProcessMemory(curProcess, (LPVOID)(pHookContent->dwHookAddr + 5 + i), &bNopFlag, 1, 0);
			//*(BYTE*)(pHookContent->dwHookAddr + 5 + i) = bNopFlag;
		}

		// 恢复现场
		DWORD dwProtect = NULL;
		::VirtualProtect((LPVOID)pHookContent->dwHookAddr, 8, dwOldProtent, &dwProtect);
		PushHookList(pHookContent);


		
		return TRUE;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"Hook_Fun_Jmp_MyAddr出现异常");
	}

	return FALSE;
}

BOOL CLHook::UnHook_Fun_Jmp_MyAddr(const MYHOOK_CONTENT* pHookContent)
{
	__try
	{
		DWORD dwProtect = NULL;

		// Update Memory Page
		::VirtualProtect((LPVOID)pHookContent->dwHookAddr, 8, PAGE_EXECUTE_READWRITE, &dwProtect);

		// Repair Code
		::WriteProcessMemory(::GetCurrentProcess(), (LPVOID)(pHookContent->dwHookAddr + 0x0), &pHookContent->dwOldCode1, 4, NULL);
		::WriteProcessMemory(::GetCurrentProcess(), (LPVOID)(pHookContent->dwHookAddr + 0x4), &pHookContent->dwOldCode2, 4, NULL);

		DWORD dwOldProtect = NULL;
		::VirtualProtect((LPVOID)pHookContent->dwHookAddr, 8, dwProtect, &dwOldProtect);
		return TRUE;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"UnHook_Fun_Jmp_MyAddr发生了异常");
	}
	return FALSE;
}

/////////可以用来做下面的测试/////////////////////////////////////////////////////////////////
typedef int (WINAPI* PFNMessageBoxA)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
int WINAPI NEW_MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	// 此处可以观察/修改调用参数，甚至可以取消调用直接返回。 
	// ……
	// 取得原函数地址 
	PFNMessageBoxA pfnMessageBoxA = (PFNMessageBoxA)HookItem.dwOldValue;

	// 输出测试信息， 
	// 如果这里直接调用MessageBoxA，就进入无限循环 
	pfnMessageBoxA(hWnd, "这是API重定向过程的消息框", "测试", 0);

	// 调用原函数 
	int ret = pfnMessageBoxA(hWnd, lpText, lpCaption, uType);

	// 此处可以查看/修改调用原函数的返回值 
	// ……

	return ret;
}

BOOL CLHook::Hook_API_Redirect(IN LPCSTR, IN LPCSTR, IN DWORD, OUT PHOOK_ITEM)
{
	return FALSE;
}

DWORD CLHook::GetCall(DWORD dwAddr1, DWORD dwAddr2, LPCWSTR pwszModuleName)
{
	DWORD dwAddr = dwAddr1 - dwAddr2 + 0x1/*0xE8*/;
	dwAddr += (DWORD)GetModuleHandleW(pwszModuleName);
	DWORD dwReaderAddr = CCharacter::ReadDWORD(dwAddr);
	dwReaderAddr += 4;
	dwReaderAddr += dwAddr;
	return dwReaderAddr & 0xFFFFFFFF;
}

DWORD CLHook::GetBase(DWORD dwAddr1, DWORD dwAddr2, LPCWSTR pwszModuleName)
{
	DWORD dwAddr = dwAddr2 - dwAddr1 - 0x5 + 0x1/*0xE8*/;
	dwAddr += (DWORD)GetModuleHandleW(pwszModuleName);
	return CCharacter::ReadDWORD(dwAddr);
}

std::vector<MYHOOK_CONTENT>& CLHook::GetHookList()
{
	static std::vector<MYHOOK_CONTENT> vlst;
	return vlst;
}

VOID CLHook::PushHookList(_In_ MYHOOK_CONTENT* pHookContent)
{
	if(CLPublic::Vec_find_if(GetHookList(), [pHookContent](CONST MYHOOK_CONTENT& itm){ return itm.dwHookAddr == pHookContent->dwHookAddr; }) == nullptr)
		GetHookList().push_back(*pHookContent);
}

VOID CLHook::DeleteHookList(_In_ MYHOOK_CONTENT* pHookContent)
{
	CLPublic::Vec_erase_if(GetHookList(), [pHookContent](CONST MYHOOK_CONTENT& itm){ return itm.dwHookAddr == pHookContent->dwHookAddr; });
}

VOID CLHook::Release()
{
	CLPublic::Vec_erase_if(GetHookList(), [](CONST MYHOOK_CONTENT& itm){ UnHook_Fun_Jmp_MyAddr(&itm); return TRUE; });
}
#endif // _WIN64