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


		// ��ʼ������
		BYTE bJmpFlag = 0xE9;
		BYTE bNopFlag = 0x90;
		BOOL  bRetCode = FALSE;
		DWORD dwOldProtent = NULL;
		HANDLE curProcess = ::GetCurrentProcess();

		// �����ڴ�ƫ�Ƶ�ַ
		DWORD dwJmpAddr = pHookContent->dwFunAddr - pHookContent->dwHookAddr - 5;

		// ��ȡ�ɵĴ���
		pHookContent->dwOldCode1 = CCharacter::ReadDWORD(pHookContent->dwHookAddr + 0x0);
		pHookContent->dwOldCode2 = CCharacter::ReadDWORD(pHookContent->dwHookAddr + 0x4);

		// �޸��ڴ�ҳ����,��Ȼ���ܲ���д��Ȩ��
		::VirtualProtect((LPVOID)pHookContent->dwHookAddr, 8, PAGE_EXECUTE_READWRITE, &dwOldProtent);

		// д��Jmpָ��
		//*(BYTE*)pHookContent->dwHookAddr = bJmpFlag;
		::WriteProcessMemory(curProcess, (LPVOID)pHookContent->dwHookAddr, &bJmpFlag, 1, 0);

		// д���Լ������Addr
		//*(DWORD*)(pHookContent->dwHookAddr + 1) = dwJmpAddr;
		bRetCode = ::WriteProcessMemory(curProcess, (LPVOID)(pHookContent->dwHookAddr + 1), &dwJmpAddr, 4, 0);

		// Nop������Ĵ���
		for (UINT i = 0; i < pHookContent->uNopCount; ++i)
		{
			::WriteProcessMemory(curProcess, (LPVOID)(pHookContent->dwHookAddr + 5 + i), &bNopFlag, 1, 0);
			//*(BYTE*)(pHookContent->dwHookAddr + 5 + i) = bNopFlag;
		}

		// �ָ��ֳ�
		DWORD dwProtect = NULL;
		::VirtualProtect((LPVOID)pHookContent->dwHookAddr, 8, dwOldProtent, &dwProtect);
		PushHookList(pHookContent);


		
		return TRUE;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"Hook_Fun_Jmp_MyAddr�����쳣");
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
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"UnHook_Fun_Jmp_MyAddr�������쳣");
	}
	return FALSE;
}

/////////��������������Ĳ���/////////////////////////////////////////////////////////////////
typedef int (WINAPI* PFNMessageBoxA)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
int WINAPI NEW_MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	// �˴����Թ۲�/�޸ĵ��ò�������������ȡ������ֱ�ӷ��ء� 
	// ����
	// ȡ��ԭ������ַ 
	PFNMessageBoxA pfnMessageBoxA = (PFNMessageBoxA)HookItem.dwOldValue;

	// ���������Ϣ�� 
	// �������ֱ�ӵ���MessageBoxA���ͽ�������ѭ�� 
	pfnMessageBoxA(hWnd, "����API�ض�����̵���Ϣ��", "����", 0);

	// ����ԭ���� 
	int ret = pfnMessageBoxA(hWnd, lpText, lpCaption, uType);

	// �˴����Բ鿴/�޸ĵ���ԭ�����ķ���ֵ 
	// ����

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