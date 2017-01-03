#ifndef __CLHOOK_H__
#define __CLHOOK_H__ TRUE

#include "ToolsPublic.h"

typedef struct _MYHOOK_CONTENT
{
	DWORD dwHookAddr;				//Hook�ĵ�ַ
	DWORD dwFunAddr;				//�Լ������ĵ�ַ
	UINT  uNopCount;				//ҪNop�����ֽ���
	DWORD dwOldCode1;				//��ԭ�Ĵ���1
	DWORD dwOldCode2;				//��ԭ�Ĵ���2
	DWORD dwFlag;					// ����
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

// ����API�ҽ���ṹ 
typedef struct _HOOK_ITEM {
	DWORD dwAddr;   // IAT�����ڵ�ַ 
	DWORD dwOldValue;  // IAT���ԭʼ������ַ 
	DWORD dwNewValue;  // IAT����º�����ַ 
} HOOK_ITEM, *PHOOK_ITEM;
extern HOOK_ITEM HookItem; // ����IAT����ڱ���MessageBoxA��IAT����Ϣ

class CLHook
{
public:
	static BOOL Hook_Fun_Jmp_MyAddr(IN OUT MYHOOK_CONTENT* pHookContent);													//��ͨ��inline Hook
	static BOOL UnHook_Fun_Jmp_MyAddr(const MYHOOK_CONTENT* pHookContent);
	static BOOL Hook_API_Redirect(IN LPCSTR pszDllName, IN LPCSTR pszFunName, IN DWORD dwNewProc, OUT PHOOK_ITEM pItem);	//�޸�IAT����Hook API
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