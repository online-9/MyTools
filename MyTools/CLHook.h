#ifndef __CLHOOK_H__
#define __CLHOOK_H__ TRUE

#include "ToolsPublic.h"

typedef struct _MYHOOK_CONTENT
{
	DWORD dwHookAddr;				//Hook的地址
	DWORD dwFunAddr;				//自己函数的地址
	UINT  uNopCount;				//要Nop掉的字节数
	DWORD dwOldCode1;				//还原的代码1
	DWORD dwOldCode2;				//还原的代码2
	DWORD dwFlag;					// 保留
	_MYHOOK_CONTENT()
	{
		Release();
	}
	VOID Release()
	{
		dwHookAddr = dwFunAddr = dwOldCode1 = dwOldCode2 = dwFlag = 0;
		uNopCount = 0;
	}
}MYHOOK_CONTENT;

// 定义API挂接项结构 
typedef struct _HOOK_ITEM {
	DWORD dwAddr;   // IAT项所在地址 
	DWORD dwOldValue;  // IAT项的原始函数地址 
	DWORD dwNewValue;  // IAT项的新函数地址 
} HOOK_ITEM, *PHOOK_ITEM;
extern HOOK_ITEM HookItem; // 定义IAT项，用于保存MessageBoxA的IAT项信息

class CLHook
{
public:
	static BOOL Hook_Fun_Jmp_MyAddr(IN OUT MYHOOK_CONTENT* pHookContent);													//普通的inline Hook
	static BOOL UnHook_Fun_Jmp_MyAddr(const MYHOOK_CONTENT* pHookContent);
	static BOOL Hook_API_Redirect(IN LPCSTR pszDllName, IN LPCSTR pszFunName, IN DWORD dwNewProc, OUT PHOOK_ITEM pItem);	//修改IAT表来Hook API
	static DWORD GetCall(DWORD dwAddr1, DWORD dwAddr2, LPCWSTR pwszModuleName);
	static DWORD GetBase(DWORD dwAddr1, DWORD dwAddr2, LPCWSTR pwszModuleName);
public:
	CLHook();
	~CLHook();

	static VOID Release();
private:
	static std::vector<MYHOOK_CONTENT>& GetHookList();
	static VOID PushHookList(_In_ MYHOOK_CONTENT* pHookContent);
	static VOID DeleteHookList(_In_ MYHOOK_CONTENT* pHookContent);
};



#endif