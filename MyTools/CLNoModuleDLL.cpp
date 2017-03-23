#include "stdafx.h"
#include "CLNoModuleDLL.h"
#include "CLNoModuleDLL_Struct.h"
#include "Character.h"
#include "Log.h"

#define _SELF L"CLNoModuleDLL.cpp"

CLNoModuleDLL::CLNoModuleDLL()
{
}

CLNoModuleDLL::~CLNoModuleDLL()
{
}

BOOL CLNoModuleDLL::LoadDll2Mem(PVOID &pAllocMem, DWORD &dwMemSize, char* strFileName)
{
	HANDLE hFile = CreateFileA(strFileName, GENERIC_READ,
		FILE_SHARE_READ, NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"hFile = NULL, strFileName = %s",  CCharacter::ASCIIToUnicode(strFileName).c_str());
		return FALSE;
	}

	PVOID pFileBuff = NULL;

	int nFileSize = GetFileSize(hFile, NULL);

	if (nFileSize == 0)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"nFileSize = 0");
		return FALSE;
	}
	else
	{
		pFileBuff = VirtualAlloc(NULL, nFileSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	}
	DWORD dwReadSize = 0;
	if (!ReadFile(hFile, pFileBuff, nFileSize, &dwReadSize, NULL))
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"ReadFile = 0");
		return FALSE;
	}

	//PVOID pBase = pFileBuff;

	//ÅÐ¶ÏÊÇ·ñÊÇPE
	PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER)pFileBuff;
	if (IMAGE_DOS_SIGNATURE != pIDH->e_magic)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"!= MZ");
		return FALSE;
	}

	PIMAGE_NT_HEADERS pINH = (PIMAGE_NT_HEADERS)((ULONG)pFileBuff + pIDH->e_lfanew);

	if (IMAGE_NT_SIGNATURE != pINH->Signature)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"!= PE");
		return FALSE;
	}

	dwMemSize = nFileSize;


	pAllocMem = pFileBuff;

	return TRUE;
}

BOOL CLNoModuleDLL::PELoader(char *lpStaticPEBuff, PVOID& pExecuMem)
{
	long lPESignOffset = *(long *)(lpStaticPEBuff + 0x3c);
	IMAGE_NT_HEADERS *pINH = (IMAGE_NT_HEADERS *)(lpStaticPEBuff + lPESignOffset);

	long lImageSize = pINH->OptionalHeader.SizeOfImage;
	char *lpDynPEBuff = (char *)VirtualAlloc(NULL, lImageSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (lpDynPEBuff == NULL)
	{
		return FALSE;
	}
	memset(lpDynPEBuff, 0, lImageSize);

	long lSectionNum = pINH->FileHeader.NumberOfSections;

	IMAGE_SECTION_HEADER *pISH = (IMAGE_SECTION_HEADER *)((char *)pINH + sizeof(IMAGE_NT_HEADERS));

	memcpy(lpDynPEBuff, lpStaticPEBuff, pISH->VirtualAddress);

	//long lFileAlignMask = pINH->OptionalHeader.FileAlignment - 1;
	//long lSectionAlignMask = pINH->OptionalHeader.SectionAlignment - 1;

	for (int nIndex = 0; nIndex < lSectionNum; nIndex++, pISH++)
	{
		memcpy(lpDynPEBuff + pISH->VirtualAddress, lpStaticPEBuff + pISH->PointerToRawData, pISH->SizeOfRawData);
	}

	if (pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size > 0)
	{
		IMAGE_IMPORT_DESCRIPTOR *pIID = (IMAGE_IMPORT_DESCRIPTOR *)(lpDynPEBuff + \
			pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		for (; pIID->Name != NULL; pIID++)
		{
			IMAGE_THUNK_DATA *pITD = (IMAGE_THUNK_DATA *)(lpDynPEBuff + pIID->FirstThunk);

			char* pLoadName = lpDynPEBuff + pIID->Name;
			HINSTANCE hInstance = LoadLibraryA(pLoadName);
			if (hInstance == NULL)
			{

				VirtualFree(lpDynPEBuff, lImageSize, MEM_DECOMMIT);
				return FALSE;
			}

			for (; pITD->u1.Ordinal != 0; pITD++)
			{
				FARPROC fpFun;
				if (pITD->u1.Ordinal & IMAGE_ORDINAL_FLAG32)
				{
					fpFun = GetProcAddress(hInstance, (LPCSTR)(pITD->u1.Ordinal & 0x0000ffff));
				}
				else
				{
					IMAGE_IMPORT_BY_NAME * pIIBN = (IMAGE_IMPORT_BY_NAME *)(lpDynPEBuff + pITD->u1.Ordinal);
					fpFun = GetProcAddress(hInstance, (LPCSTR)pIIBN->Name);
				}

				if (fpFun == NULL)
				{
					delete lpDynPEBuff;
					return false;
				}

				pITD->u1.Ordinal = (long)fpFun;
			}
		}
	}



	if (pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size > 0)
	{
		IMAGE_BASE_RELOCATION *pIBR = (IMAGE_BASE_RELOCATION *)(lpDynPEBuff + \
			pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

		long lDifference = (long)lpDynPEBuff - pINH->OptionalHeader.ImageBase;

		for (; pIBR->VirtualAddress != 0;)
		{
			char *lpMemPage = lpDynPEBuff + pIBR->VirtualAddress;
			long lCount = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) >> 1;

			short int *pRelocationItem = (short int *)((char *)pIBR + sizeof(IMAGE_BASE_RELOCATION));
			for (int nIndex = 0; nIndex < lCount; nIndex++)
			{
				int nOffset = pRelocationItem[nIndex] & 0x0fff;
				int nType = pRelocationItem[nIndex] >> 12;

				if (nType == 3)
				{
					*(long *)(lpMemPage + nOffset) += lDifference;
				}
				else if (nType == 0)
				{
				}
			}

			pIBR = (IMAGE_BASE_RELOCATION *)(pRelocationItem + lCount);
		}

	}

	pExecuMem = lpDynPEBuff;

	return TRUE;
}

BOOL CLNoModuleDLL::CallDllMain(PVOID pExecMem, DWORD dwReaseon, char* pModuleName)
{
	PIMAGE_NT_HEADERS pINH = (PIMAGE_NT_HEADERS)((ULONG)pExecMem + ((PIMAGE_DOS_HEADER)pExecMem)->e_lfanew);
	DWORD dwEP = pINH->OptionalHeader.AddressOfEntryPoint;
	DLL_MAIN lpDllMain = (DLL_MAIN)((DWORD)pExecMem + dwEP);
	lpDllMain((HMODULE)pExecMem, dwReaseon, pModuleName);

	return TRUE;
}

BOOL CLNoModuleDLL::LaunchDll(char *strName, DWORD dwReason)
{
	PVOID pRelocMem = NULL;
	PVOID pExecuMem = NULL;
	DWORD dwMemSize = 0;
	if (LoadDll2Mem(pRelocMem, dwMemSize, strName))
	{
		PELoader((char *)pRelocMem, pExecuMem);
		CallDllMain(pExecuMem, dwReason, strName);
		ZeroMemory(pRelocMem, dwMemSize);
		VirtualFree(pRelocMem, dwMemSize, MEM_DECOMMIT);
	}
	return TRUE;
}


BOOL CLNoModuleDLL::HideDllPeb(LPCWSTR lpDllName)
{
#define RemoveEntryList(_Entry) \
	{ \
	_CL_LIST_ENTRY* _OldFlink; \
	_CL_LIST_ENTRY* _OldBlink; \
	_OldFlink = (_Entry)->Flink; \
	_OldBlink = (_Entry)->Blink; \
	_OldFlink->Blink = _OldBlink; \
	_OldBlink->Flink = _OldFlink; \
	(_Entry)->Flink = NULL; \
	(_Entry)->Blink = NULL; \
}

	PLDR_DATA_TABLE_ENTRY pldteDllEntry;
	CL_LIST_ENTRY* pleCurrentDll;
	CL_LIST_ENTRY* pleHeadDll;
	PPEB_LDR_DATA ppldLoaderData;
	PPEB ppPEB = (PPEB)__readfsdword(0x30);
	ppldLoaderData = ppPEB->Ldr;
	if (ppldLoaderData)
	{
		pleHeadDll = &ppldLoaderData->InLoadOrderModuleList;
		pleCurrentDll = pleHeadDll;
		while (pleCurrentDll && (pleHeadDll != (pleCurrentDll = pleCurrentDll->Flink)))
		{
			pldteDllEntry = CONTAINING_RECORD(pleCurrentDll, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList);
			if (pldteDllEntry && pldteDllEntry->Flags & 0x00000004)
			{
				if (CCharacter::wstrstr_my(pldteDllEntry->FullDllName.Buffer, lpDllName))
				{
					RemoveEntryList(&pldteDllEntry->InLoadOrderModuleList);
					RemoveEntryList(&pldteDllEntry->InInitializationOrderModuleList);
					RemoveEntryList(&pldteDllEntry->InMemoryOrderModuleList);
					RemoveEntryList(&pldteDllEntry->HashLinks);
				}
			}
		}
	}
	return TRUE;
}

static CL_LIST_ENTRY* GetHashTableAddress()
{
	HANDLE hModule = GetModuleHandle(TEXT("ntdll.dll"));
	BYTE *p = NULL;
	CL_LIST_ENTRY *retval = NULL;
	BYTE pSign[] = { 0x83, 0xE0, 0x1F, 0x8D, 0x04, 0xC5 };
	DWORD SignLen = 6;

	if (hModule != NULL)
	{
		__try
		{
			DWORD dwAddress = (DWORD)GetProcAddress((HMODULE)hModule, "LdrLoadDll");
			for (DWORD i = 0; i < 0x100000; i++)
			{
				//
				if (memcmp((BYTE *)(dwAddress + i), pSign, SignLen) == 0)
				{
					p = (BYTE *)((DWORD)dwAddress + i);
					p += 6;
					retval = (CL_LIST_ENTRY *)(*(DWORD *)p);
					break;
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{

		}
	}
	


	return retval;
}

BOOL CLNoModuleDLL::HideDllHashLink(LPCWSTR lpDllName)
{
	CL_LIST_ENTRY *LdrpHashTable;
	LdrpHashTable = GetHashTableAddress();
	if (LdrpHashTable)
	{
		CL_LIST_ENTRY *pListEntry, *pListHead;
		LDR_DATA_TABLE_ENTRY *LdrDataEntry;
		for (DWORD i = 0; i < 32; i++)
		{
			pListEntry = LdrpHashTable + i;
			pListEntry = pListEntry->Flink;
			pListHead = LdrpHashTable + i;

			while (pListEntry != pListHead)
			{
				LdrDataEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY, HashLinks);
				pListEntry = LdrDataEntry->HashLinks.Flink;
				if (LdrDataEntry && LdrDataEntry->Flags & 0x00000004)
				{
					if (CCharacter::wstrstr_my(LdrDataEntry->FullDllName.Buffer, lpDllName))
					{
						RemoveEntryList(&LdrDataEntry->InLoadOrderModuleList);
						RemoveEntryList(&LdrDataEntry->InInitializationOrderModuleList);
						RemoveEntryList(&LdrDataEntry->InMemoryOrderModuleList);
						RemoveEntryList(&LdrDataEntry->HashLinks);
					}
				}

			}
		}
	}
	return TRUE;
}