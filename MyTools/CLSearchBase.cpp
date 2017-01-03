#include "stdafx.h"
#include "CLSearchBase.h"
#include "Character.h"

#define _SELF L"CLSearchBase.cpp"

CLSearchBase::CLSearchBase()
{
}

CLSearchBase::~CLSearchBase()
{
}

BOOL CLSearchBase::CompCode(const DWORD * pCode, const BYTE * pMem, UINT uLen)
{
	BOOL bComp = TRUE;
	for (UINT i = 0; i < uLen; ++i)
	{
		if (pCode[i] != 0x100 && (BYTE)pCode[i] != (BYTE)pMem[i])
		{
			bComp = FALSE;
			break;
		}
	}

	return bComp;
}

BOOL CLSearchBase::CL_sunday(DWORD* pKey, UINT uKeyLen, BYTE* pCode, UINT uCodeLen, std::vector<int>& vlst)
{
	//807E1000740E
	UINT uNowPos = 0;
	while (uNowPos + uKeyLen < uCodeLen)
	{
		if (CompCode(pKey, pCode + uNowPos, uKeyLen))
		{
			vlst.push_back(uNowPos);
			uNowPos += uKeyLen + 1;
			continue;
		}

		BYTE bWord = pCode[uKeyLen + uNowPos];

		int nWordPos = GetWord_By_Char(bWord, pKey, uKeyLen);
		if (nWordPos == -1)
			uNowPos += uKeyLen + 1;
		else
			uNowPos += (UINT)nWordPos;
	}

	return TRUE;
}

int CLSearchBase::GetWord_By_Char(BYTE dwWord, DWORD* pKey, UINT uKeyLen)
{
	int uLen = uKeyLen - 1;
	for (int i = uLen; i >= 0; --i)
	{
		if (pKey[i] == 0x100 || (BYTE)pKey[i] == dwWord)
		{
			return uKeyLen - i;
		}
	}
	return -1;
}

DWORD CLSearchBase::GetImageSize(HMODULE hm)
{
	DWORD dwSize = 0x0;
	_asm
	{
		PUSHAD
			MOV EBX, DWORD PTR hm
			MOV EAX, DWORD PTR DS : [EBX + 0x3C]
			LEA EAX, DWORD PTR DS : [EBX + EAX + 0x50]
			MOV EAX, DWORD PTR DS : [EAX]
			MOV DWORD PTR DS : [dwSize], EAX
			POPAD
	}
	return dwSize;
}

BOOL CLSearchBase::SearchBase(LPCSTR szCode, DWORD * pArray, UINT& puLen, LPCWSTR lpszModule)
{
	SYSTEM_INFO		si;
	MEMORY_BASIC_INFORMATION		mbi;

	BOOL	bRetCode = FALSE;

	//���ַ���ת����BYTE����
	UINT uCodeLen = strlen(szCode) / 2;
	if (strlen(szCode) % 2 != 0)
	{
		MessageBoxW(NULL, L"������2�ı���!", L"Error", NULL);
		return FALSE;
	}

	int nCount = 0;
	DWORD * pCode = new DWORD[uCodeLen];
	memset(pCode, 0, uCodeLen);

	for (UINT i = 0; i < uCodeLen; ++i)
	{
		if (szCode[i * 2] != '?')
		{
			char szText[] = { szCode[i * 2], szCode[i * 2 + 1], '\0' };
			pCode[i] = (DWORD)strtol(szText, NULL, 16);
		}
		else
		{
			pCode[i] = 0x100;
		}
	}

	//��ʼ��
	::GetSystemInfo(&si);
	HANDLE hProcess = ::GetCurrentProcess();
	DWORD dwImageBase = (DWORD)::GetModuleHandleW(lpszModule);
	DWORD dwImageSize = GetImageSize((HMODULE)dwImageBase);

	for (DWORD dwAddr = dwImageBase; dwAddr < dwImageBase + dwImageSize; dwAddr += mbi.RegionSize)
		//for (BYTE * pCurPos = (LPBYTE)si.lpMinimumApplicationAddress; pCurPos < (LPBYTE)si.lpMaximumApplicationAddress - uCodeLen; pCurPos = (PBYTE)mbi.BaseAddress + mbi.RegionSize,nCount++ )
	{
		//��ѯ��ǰ�ڴ�ҳ������
		::VirtualQueryEx(hProcess, (LPCVOID)dwAddr, &mbi, sizeof(mbi));
		if (mbi.Protect == PAGE_READONLY)//ɨ��ֻ���ڴ�
		{
			DWORD dwOldProtect;
			::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READ, &dwOldProtect);
		}
		if (mbi.State == MEM_COMMIT && (mbi.Protect == PAGE_EXECUTE_READ || mbi.Protect == PAGE_EXECUTE_READWRITE))
		{
			std::vector<int> vlst;
			CL_sunday(pCode, uCodeLen, (PBYTE)mbi.BaseAddress, mbi.RegionSize, vlst);

			for (UINT i = 0; i < vlst.size() && puLen < 10; ++i)
			{
				pArray[puLen] = (DWORD)mbi.BaseAddress + vlst.at(i);
				++puLen;
			}

		}
	}

	if (puLen >= 10)
	{
		bRetCode = TRUE;
	}
	else if (puLen < 10 && puLen > 0)
	{
		bRetCode = TRUE;
	}
	delete[] pCode;
	return bRetCode;
}

DWORD CLSearchBase::FindBase(LPCSTR lpszCode, int nOffset, int nMov, int nOrderNum, LPCWSTR lpszModule, DWORD dwAddrLen)
{
	DWORD	dwArray[10] = { 0 };
	UINT	uArrayLen = 0x0;
	DWORD	dwBase = 0x0;
	DWORD	dwAddr = 0x0;

	//��ʼ������ַ
	if (SearchBase(lpszCode, dwArray, uArrayLen, lpszModule))
	{
		if (uArrayLen == 1)//�ж�ֻ��һ����ʱ��
			dwAddr = dwArray[0];
		else
			dwAddr = dwArray[nOrderNum];
	}

	if (dwAddr != 0x0)
	{
		if (nOffset >= 0)
			dwAddr -= abs(nOffset);
		else
			dwAddr += abs(nOffset);

		dwAddr += nMov;
		dwBase = CCharacter::ReadDWORD(dwAddr)&dwAddrLen;
	}

	return dwBase;
}

DWORD CLSearchBase::FindCALL(LPCSTR lpszCode, int nOffset, DWORD dwModuleAddr, int nMov, int nOrderNum, LPCWSTR lpszModule)
{
	DWORD	dwArray[10] = { 0 };
	UINT	uArrayLen = 0x0;
	DWORD	dwCALL = 0x0;
	DWORD	dwAddr = 0x0;

	//��ʼ������ַ
	if (SearchBase(lpszCode, dwArray, uArrayLen, lpszModule))
	{
		if (uArrayLen == 1)//�ж�ֻ��һ����ʱ��
			dwAddr = dwArray[0];
		else
			dwAddr = dwArray[nOrderNum];
	}

	if (dwAddr != 0x0)
	{
		if (nOffset >= 0)
			dwAddr -= abs(nOffset);
		else
			dwAddr += abs(nOffset);

		// ����CALL!
		if (CCharacter::ReadBYTE(dwAddr) != 0xE8)
			return 0x0;

		//���ȼ�����Ե�ַ
		DWORD dwRelativeAddr = dwAddr - (dwModuleAddr + 0x1000) + 0x1000 + nMov;
		dwRelativeAddr += dwModuleAddr;
		DWORD dwReadAddr = CCharacter::ReadDWORD(dwRelativeAddr);
		dwReadAddr += 4;
		dwReadAddr += dwRelativeAddr;
		dwCALL = dwReadAddr & 0xFFFFFFFF;
	}

	return dwCALL;
}

DWORD CLSearchBase::FindAddr(LPCSTR lpszCode, int nOffset, int nOrderNum, LPCWSTR lpszModule)
{
	DWORD	dwArray[10] = { 0 };
	UINT	uArrayLen = 0x0;
	DWORD	dwAddr = 0x0;

	//��ʼ������ַ
	if (SearchBase(lpszCode, dwArray, uArrayLen, lpszModule))
	{
		if (uArrayLen == 1)//�ж�ֻ��һ����ʱ��
			dwAddr = dwArray[0];
		else
			dwAddr = dwArray[nOrderNum];
	}

	if (dwAddr != 0x0){
		if (nOffset >= 0)
			dwAddr -= abs(nOffset);
		else
			dwAddr += abs(nOffset);
	}


	return dwAddr;
}

DWORD CLSearchBase::FindBase_ByCALL(LPCSTR lpszCode, int nOffset, DWORD dwModuleAddr, int nMov, int nOrderNum, LPCWSTR lpszModule, int nBaseOffset, DWORD dwAddrLen /*= 0xFFFFFFFF*/)
{
	DWORD dwCALL = FindCALL(lpszCode, nOffset, dwModuleAddr, nMov, nOrderNum, lpszModule);
	if (dwCALL == NULL)
		return NULL;

	dwCALL += nBaseOffset;
	return CCharacter::ReadDWORD(dwCALL) & dwAddrLen;
}
