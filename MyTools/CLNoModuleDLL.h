#ifndef __MYTOOLS_NOMODULEDLL_H__
#define __MYTOOLS_NOMODULEDLL_H__

#include "ToolsPublic.h"

class CLNoModuleDLL
{
public:
	typedef BOOL(WINAPI *DLL_MAIN)(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);
public:
	CLNoModuleDLL();
	~CLNoModuleDLL();
#ifdef _WIN64
#else
	BOOL LaunchDll(char *strName, DWORD dwReason);
	BOOL HideDllPeb(LPCWSTR lpDllName);
	BOOL HideDllHashLink(LPCWSTR lpDllName);
private:
	BOOL CallDllMain(PVOID pExecMem, DWORD dwReaseon, char* pModuleName);
	BOOL PELoader(char *lpStaticPEBuff, PVOID& pExecuMem);
	BOOL LoadDll2Mem(PVOID &pAllocMem, DWORD &dwMemSize, char* strFileName);
#endif // _WIN64

};



#endif