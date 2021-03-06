#include "stdafx.h"
#include "Character.h"
#include <time.h>
#include <cctype>
#include "Log.h"
#include "CLLock.h"

#define _SELF_A "Character.cpp"
#define _SELF_W L"Character.cpp"
#define _SELF _SELF_W

ReadMemoryType CCharacter::s_ReadMemoryType = Read_API;

LPSTR CCharacter::strcpy_my(LPSTR strDest,LPCSTR strSrc,size_t len)
{
	size_t			i = 0;
	LPSTR pDest = strDest;

	__try
	{
		while (*strSrc != '\0' && i++ < len)
			*strDest++ = *strSrc++;

		*strDest = '\0';
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"strcpy_my发生了异常");
		
	}
	return pDest;
}

LPWSTR CCharacter::wstrcpy_my(LPWSTR strDest,LPCWSTR strSrc,size_t len)
{
	size_t	i = 0;
	LPWSTR	pDest = strDest;

	__try
	{
		while (*strSrc != '\0' && i++ < len)//空格是32,结束是0
			*strDest++ = *strSrc++;

		*strDest = '\0';//将'\0'放入到里面
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"wstrcpy_my发生了异常");
	}
	return pDest;
}

bool CCharacter::strcmp_my(LPCSTR strDest,LPCSTR strSrc)
{
	__try
	{
		if (strlen(strDest) == strlen(strSrc)) //用长度判断
		{
			while (*strDest != '\0')
			{
				if (*strDest++ != *strSrc++)
					return false;
			}

			return true;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"strcmp_my发生了异常");
	}
	return false;
}

bool CCharacter::wstrcmp_my(LPCWSTR strDest,LPCWSTR strSrc)
{
	__try
	{
		if (wcslen(strDest) == wcslen(strSrc)) //用长度判断
		{
			while (*strDest != '\0')
			{
				if (*strDest++ != *strSrc++)
					return false;
			}

			return true;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"wstrcmp_my发生了异常");
	}

	return false;
}

LPSTR CCharacter::Trim_A(LPSTR str)
{
	//先去除头部就可以吃,中间部分不去除
	auto fn_Seh_Trim_A = [&str](_In_ LPSTR pszText)
	{
		__try
		{
			char *	pszstr = str;
			bool	bflag = true;
			BOOL	IsTrim = FALSE;
			size_t	i = 0;

			while (*str != '\0')
			{
				if (*str == ' ' && bflag)
				{
					IsTrim = TRUE;
					continue;
				}
				else if (bflag)
				{
					bflag = false;
				}
				pszText[i++] = *(str++);
			}
			pszText[i] = '\0';
			str = pszstr;

			//再去除尾部,嘎嘣脆
			for (i = strlen(pszText); i > 0; i--)
			{
				if (pszText[i] != 0 && pszText[i] != ' ')
				{
					if (i != strlen(pszText) - 1)
					{
						IsTrim = TRUE;
					}
					pszText[++i] = '\0';
					break;
				}
			}
			if (IsTrim)
				strcpy_my(str, (LPCSTR)pszText);

		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"fn_Seh_Trim_A发生了异常");
		}
	};

	std::shared_ptr<char> pszText(new char[strlen(str) + 1], [](char* p){ delete[] p; });

	fn_Seh_Trim_A(pszText.get());
	return str;
}

std::string& CCharacter::Trim_A(std::string& s)
{
	try
	{
		if (s.empty())
			return s;

		std::string::iterator c;
		for (c = s.begin(); c != s.end() && iswspace(*c++);); s.erase(s.begin(), --c);

		// Erase whitespace after the string

		for (c = s.end(); c != s.begin() && iswspace(*--c);); s.erase(++c, s.end());

	}
	catch(...)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"Trim_A出现异常");
	}
	return s;
}

std::wstring& CCharacter::Trim_W(std::wstring& s)
{
	try
	{
		if (s.empty())
			return s;

		std::wstring::iterator c;
		for (c = s.begin(); c != s.end() && iswspace(*c++);); s.erase(s.begin(), --c);

		// Erase whitespace after the string

		for (c = s.end(); c != s.begin() && iswspace(*--c);); s.erase(++c, s.end());
	}
	catch(...)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"Trim_W出现异常");
	}
	return s;
}

LPWSTR CCharacter::Trim_W(LPWSTR str)
{
	//先去除头部,中间部分不去除
	auto fn_Seh_Trim_W = [&str](_In_ LPWSTR pszText)
	{
		__try
		{
			wchar_t *	pszstr = str;
			bool		bflag = true;
			BOOL		IsTrim = FALSE;
			size_t		i = 0;

			while (*str != '\0')
			{
				if ((*str == ' ' || *str == 0xA || *str == 0x9) && bflag)
				{
					str++;
					IsTrim = TRUE;
					continue;
				}
				else if (bflag)
					bflag = false;

				pszText[i++] = *(str++);
			}
			pszText[i] = '\0';
			str = pszstr;

			//再去除尾部
			for (i = wcslen(pszText); i > 0; i--)
			{
				if (pszText[i] != 0 && pszText[i] != ' ')
				{
					if (i != wcslen(pszText) - 1)
					{
						IsTrim = TRUE;
					}
					pszText[++i] = '\0';
					break;
				}
			}
			if (IsTrim)
				CCharacter::wstrcpy_my(str, pszText);

		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"fn_Seh_Trim_W发生了异常");
		}
	};

	std::shared_ptr<wchar_t> pszText(new wchar_t[wcslen(str) + 1], [](wchar_t* p){ delete[] p; });
	fn_Seh_Trim_W(pszText.get());
	return str;
}

bool CCharacter::strstr_my(LPCSTR strDest,LPCSTR strSrc)
{
	__try
	{
		while (*strDest != '\0')
		{
			char * pDest = (char *)strDest;
			char * pSrc = (char *)strSrc;
			do
			{
				if (*pSrc == '\0')
					return true;
			} while (*pDest++ == *pSrc++);
			strDest += 1;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"strstr_my发生了异常");
	}
	return false;
}

bool CCharacter::wstrstr_my(LPCWSTR strDest,LPCWSTR strSrc)
{
	__try
	{
		while (*strDest != '\0')
		{
			wchar_t * pDest = (wchar_t *)strDest;
			wchar_t * pSrc = (wchar_t *)strSrc;
			do
			{
				if (*pSrc == '\0')
					return true;
			} while (*pDest++ == *pSrc++);
			strDest += 1;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"wstrstr_my发生了异常");
	}

	return false;
}


LPWSTR CCharacter::GetRemoveLeft(LPCWSTR lpszText,LPCWSTR parm,LPWSTR RetParm)
{
	__try
	{

		wchar_t *	pText = (wchar_t *)lpszText;
		size_t		i = 0;

		while (*pText != '\0')
		{
			wchar_t * pDest = (wchar_t *)pText;
			wchar_t * pSrc = (wchar_t *)parm;

			do
			{
				if (*pSrc == '\0')
				{
					CCharacter::wstrcpy_my(RetParm, lpszText, i);
						return RetParm;
				}

			} while (*pDest++ == *pSrc++);
			i++;
			pText += 1;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"GetRemoveLeft发生了异常");
	}
	return RetParm;
}

std::wstring& CCharacter::GetRemoveLeft(_In_ CONST std::wstring& wsText, _In_ CONST std::wstring& wsParm, _Out_ std::wstring& wsRetText)
{
	try
	{
		size_t nIndex = wsText.find(wsParm);
		if (nIndex == -1)
			wsRetText = wsText;
		else
			wsRetText = wsText.substr(0, nIndex);
	}
	catch (...)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"GetRemoveLeft出现异常");
	}
	return wsRetText;
}

LPSTR CCharacter::GetRemoveLeftA(LPCSTR lpszText, LPCSTR parm, LPSTR RetParm)
{
	__try
	{

		char *	pText = (char *)lpszText;
		size_t		i = 0;

		while (*pText != '\0')
		{
			char * pDest = (char *)pText;
			char * pSrc = (char *)parm;

			do
			{
				if (*pSrc == '\0')
				{
					CCharacter::strcpy_my(RetParm, lpszText, i);
					return RetParm;
				}

			} while (*pDest++ == *pSrc++);
			i++;
			pText += 1;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"GetRemoveLeftA发生了异常");
	}
	return RetParm;
}

LPWSTR CCharacter::GetRemoveRight(LPCWSTR lpszText,LPCWSTR parm,LPWSTR RetParm)
{
	__try
	{
		wchar_t *	pText = (wchar_t *)lpszText;
		size_t		i = 0;

		while (*pText != '\0')
		{
			wchar_t * pDest = (wchar_t *)pText;
			wchar_t * pSrc = (wchar_t *)parm;

			do
			{
				if (*pSrc == '\0')
				{
					CCharacter::wstrcpy_my(RetParm, pDest, wcslen(pDest) + 1);
					return RetParm;
				}

			} while (*pDest++ == *pSrc++);
			i++;
			pText += 1;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"GetRemoveRight发生了异常");
	}
	return RetParm;
}

std::wstring& CCharacter::GetRemoveRight(_In_ CONST std::wstring& wsText, _In_ CONST std::wstring& wsParm, _Out_ std::wstring& wsRetText)
{
	try
	{
		size_t nIndex = wsText.find(wsParm);
		if (nIndex == -1)
			wsRetText = wsText;
		else
			wsRetText = wsText.substr(nIndex + wsParm.length());
	}
	catch(...)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"GetRemoveRight出现异常");
	}
	return wsRetText;
}

LPSTR CCharacter::GetRemoveRightA(LPCSTR lpszText, LPCSTR parm, LPSTR RetParm)
{
	__try
	{
		char *	pText = (char *)lpszText;
		size_t		i = 0;

		while (*pText != '\0')
		{
			char * pDest = (char *)pText;
			char * pSrc = (char *)parm;

			do
			{
				if (*pSrc == '\0')
				{
					CCharacter::strcpy_my(RetParm, pDest, strlen(pDest) + 1);
					return RetParm;
				}

			} while (*pDest++ == *pSrc++);
			i++;
			pText += 1;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"GetRemoveRightA发生了异常");
	}
	return RetParm;
}

LPWSTR CCharacter::RemoveQuotation(LPWSTR lpszText)
{
	wchar_t * p = lpszText;
	wchar_t t[32];
	int i = 0;
	while(*p != '\0')
	{
		if (*p != '\"')
		{
			t[i++] = *p;
		}
		p++;
	}
	t[i] = '\0';
	CCharacter::wstrcpy_my(lpszText,t);
	return lpszText;
}

UINT CCharacter::GetCount_By_CharacterA(LPCSTR pszText, LPCSTR pszContent)
{
	UINT uCount = 0;

	__try
	{
		
		while (*pszText != '\0')
		{
			char * pDest = (char *)pszText;
			char * pSrc = (char *)pszContent;
			do
			{
				if (*pSrc == '\0')
				{
					uCount++;
					pszText += strlen(pszContent) - 1;
					break;
				}

			} while (*pDest++ == *pSrc++);
			pszText += 1;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"GetCount_By_CharacterA发生了异常");
	}

	return uCount;
}

UINT CCharacter::GetCount_By_CharacterW(LPCWSTR pwszText, LPCWSTR pwszContent)
{
	UINT uCount = 0;

	__try
	{
		while (*pwszText != '\0')
		{
			WCHAR * pDest = (WCHAR *)pwszText;
			WCHAR * pSrc = (WCHAR *)pwszContent;
			do
			{
				if (*pSrc == '\0')
				{
					uCount++;
					pwszText += wcslen(pwszContent) - 1;
					break;
				}

			} while (*pDest++ == *pSrc++);
			pwszText += 1;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"GetCount_By_CharacterW发生了异常");
	}

	return uCount;
}

UINT CCharacter::GetCount_By_CharacterW(_In_ CONST std::wstring& wsText, _In_ CONST std::wstring& wsContent)
{
	UINT uCount = 0;
	std::size_t pos = 0;
	
	do 
	{
		pos = wsText.find(wsContent, pos);
		if (pos != std::wstring::npos)
			pos += 1;
	} while (pos != std::wstring::npos && ++uCount);
	return uCount;
}

template<typename ReturnValueType>
ReturnValueType ReadMemoryValue(_In_ UINT_PTR dwAddr)
{
	ReturnValueType Value = 0;
	if (CCharacter::s_ReadMemoryType == Read_Pointer)
	{
		if ((dwAddr == 0x0 || IsBadCodePtr(FARPROC(dwAddr))))
			return NULL;

		Value = *(ReturnValueType*)dwAddr;
	}
	else if (CCharacter::s_ReadMemoryType == Read_API)
	{
		::ReadProcessMemory(::GetCurrentProcess(), (LPCVOID)dwAddr, (LPVOID)&Value, sizeof(Value), NULL);
	}
	return Value;
}

DWORD64 CCharacter::ReadDWORD64(DWORD64 dwAddr)
{
	return ReadMemoryValue<DWORD64>(static_cast<DWORD>(dwAddr));
}


UINT_PTR CCharacter::ReadDWORD(UINT_PTR dwAddr)
{
	return ReadMemoryValue<UINT_PTR>(dwAddr);
}


WORD CCharacter::ReadWORD(UINT_PTR dwAddr)
{
	return ReadMemoryValue<WORD>(dwAddr);
}

BYTE CCharacter::ReadBYTE(UINT_PTR dwAddr)
{
	return ReadMemoryValue<BYTE>(dwAddr);
}

float CCharacter::ReadFloat(UINT_PTR dwAddr)
{
	return ReadMemoryValue<float>(dwAddr);
}

double CCharacter::ReadDouble(UINT_PTR dwAddr)
{
	return ReadMemoryValue<double>(dwAddr);
}

BOOL CCharacter::WriteDWORD(UINT_PTR dwAddr, DWORD dwValue)
{
	__try
	{
		if (s_ReadMemoryType == Read_Pointer)
		{
			if ((dwAddr == 0x0 || IsBadCodePtr(FARPROC(dwAddr))))
				return NULL;

			DWORD dwOldProtect = NULL;
			::VirtualProtect((LPVOID)dwAddr, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
			*(UINT_PTR*)dwAddr = dwValue;

			if (dwOldProtect != NULL)
				::VirtualProtect((LPVOID)dwAddr, 4, dwOldProtect, &dwOldProtect);
		}
		else if (s_ReadMemoryType == Read_API)
		{
			DWORD dwOldProtect = NULL;
			::VirtualProtect((LPVOID)dwAddr, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
			::WriteProcessMemory(::GetCurrentProcess(), (LPVOID)dwAddr, (LPCVOID)&dwValue, sizeof(DWORD), NULL);

			if (dwOldProtect != NULL)
				::VirtualProtect((LPVOID)dwAddr, 4, dwOldProtect, &dwOldProtect);
		}

		return TRUE;
	}
	_except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"WriteDWORD出现异常");
	}
	return FALSE;
}

BOOL CCharacter::WriteFloat(UINT_PTR dwAddr, float fValue)
{
	__try
	{
		if (s_ReadMemoryType == Read_Pointer)
		{
			if ((dwAddr == 0x0 || IsBadCodePtr(FARPROC(dwAddr))))
				return NULL;

			DWORD dwOldProtect = NULL;
			::VirtualProtect((LPVOID)dwAddr, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
			*(float*)dwAddr = fValue;

			if (dwOldProtect != NULL)
				::VirtualProtect((LPVOID)dwAddr, 4, dwOldProtect, &dwOldProtect);
		}
		else if (s_ReadMemoryType == Read_API)
		{
			::WriteProcessMemory(::GetCurrentProcess(), (LPVOID)dwAddr, (LPCVOID)&fValue, sizeof(fValue), NULL);
		}

		return TRUE;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"WriteFloat出现异常");
	}
	return FALSE;
}

BOOL CCharacter::WriteBYTE(UINT_PTR dwAddr, BYTE bValue)
{
	__try
	{
		if (s_ReadMemoryType == Read_Pointer)
		{
			if ((dwAddr == 0x0 || IsBadCodePtr(FARPROC(dwAddr))))
				return NULL;

			DWORD dwOldProtect = NULL;
			::VirtualProtect((LPVOID)dwAddr, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
			*(float*)dwAddr = bValue;

			if (dwOldProtect != NULL)
				::VirtualProtect((LPVOID)dwAddr, 4, dwOldProtect, &dwOldProtect);
		}
		else if (s_ReadMemoryType == Read_API)
		{
			::WriteProcessMemory(::GetCurrentProcess(), (LPVOID)dwAddr, (LPCVOID)&bValue, sizeof(bValue), NULL);
		}

		return TRUE;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"WriteFloat出现异常");
	}
	return FALSE;
}


LPSTR CCharacter::UnicodeToASCII(LPCWSTR lpszWText,LPSTR lpszText)
{
	__try
	{
		int ansiLen = ::WideCharToMultiByte(CP_ACP, NULL, lpszWText, static_cast<int>(wcslen(lpszWText)), NULL, 0, NULL, NULL);
		char* szAnsi = new char[ansiLen + 1];
		::WideCharToMultiByte(CP_ACP, NULL, lpszWText, static_cast<int>(wcslen(lpszWText)), szAnsi, ansiLen, NULL, NULL);
		szAnsi[ansiLen] = '\0';
		CCharacter::strcpy_my(lpszText, szAnsi, ansiLen + 1);
		delete[] szAnsi;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"UnicodeToASCII发生了异常");
	}
	return lpszText;
}

std::string CCharacter::UnicodeToASCII(CONST std::wstring& wstr)
{
	CHAR* pszText = new CHAR[wstr.length() * 2 + 2];
	if (pszText == NULL)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"Alloc Memory in UnicodeToASCII");
		return std::string();
	}

	UnicodeToASCII(wstr.c_str(), pszText);

	std::string str = pszText;
	delete[] pszText;

	return str;
}

LPWSTR CCharacter::ASCIIToUnicode(LPCSTR lpszText,LPWSTR lpszWText)
{
	__try
	{
		int cchWideChar = MultiByteToWideChar(CP_ACP, 0, lpszText, -1, NULL, 0);
		wchar_t * ptszText = NULL;
		ptszText = new wchar_t[cchWideChar];
		MultiByteToWideChar(CP_ACP, 0, lpszText, -1, ptszText, cchWideChar);
		CCharacter::wstrcpy_my(lpszWText, ptszText, cchWideChar + 1);
		delete[] ptszText;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"ASCIIToUnicode发生了异常");
	}
	return lpszWText;
}

std::wstring CCharacter::ASCIIToUnicode(const std::string& str)
{
	WCHAR* pwszText = new WCHAR[str.length() * 2 + 2];
	if (pwszText == NULL)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"Alloc Memory in ASCIIToUnicode");
		return std::wstring();
	}

	ASCIIToUnicode(str.c_str(), pwszText);

	std::wstring wstr = pwszText;
	delete [] pwszText;

	return wstr;
}

LPWSTR CCharacter::UTF8ToUnicode(LPWSTR pszText, LPCSTR pUTF8)
{
	__try
	{
		DWORD dwUnicodeLen = MultiByteToWideChar(CP_UTF8, 0, pUTF8, -1, NULL, 0);
		wchar_t* pwText = new wchar_t[dwUnicodeLen];
		if (pwText == nullptr)
			return nullptr;

		//转为Unicode
		MultiByteToWideChar(CP_UTF8,0,pUTF8,-1,pwText,dwUnicodeLen);
		//转为
		CCharacter::wstrcpy_my(pszText,pwText);
		//清除内存
		delete []pwText;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"UTF8ToUnicode出现异常");
	}

	return pszText;
}

std::wstring CCharacter::UTF8ToUnicode(_In_ CONST std::string& csText)
{
	DWORD dwUnicodeLen = MultiByteToWideChar(CP_UTF8, 0, csText.c_str(), -1, NULL, 0);

	wchar_t* pwText = new wchar_t[dwUnicodeLen];
	if (pwText == nullptr)
		return std::wstring();

	//转为Unicode
	MultiByteToWideChar(CP_UTF8, 0, csText.c_str(), -1, pwText, dwUnicodeLen);

	return std::wstring(pwText);
}

LPSTR CCharacter::UnicodeToUTF8(LPCWSTR pszText, LPSTR pUTF8)
{
	__try
	{
		DWORD dwLen = ::WideCharToMultiByte(CP_UTF8, NULL, pszText, static_cast<int>(wcslen(pszText)), NULL, NULL, NULL, NULL);
		char* pszUTF8 = new char[dwLen + 1];
		if (pszUTF8 == nullptr)
			return nullptr;

		::WideCharToMultiByte(CP_UTF8, NULL, pszText, static_cast<int>(wcslen(pszText)), pszUTF8, dwLen, NULL, NULL);
		pszUTF8[dwLen] = '\0';
		CCharacter::strcpy_my(pUTF8, pszUTF8, dwLen + 1);
		delete[] pszUTF8;

		return pUTF8;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"UnicodeToUTF8出现异常");
	}

	return pUTF8;
}

std::string CCharacter::UnicodeToUTF8(_In_ CONST std::wstring& wsText)
{
	DWORD dwLen = ::WideCharToMultiByte(CP_UTF8, NULL, wsText.c_str(), static_cast<int>(wsText.length()), NULL, NULL, NULL, NULL);
	char* pszUTF8 = new char[dwLen + 1];
	if (pszUTF8 == nullptr)
		return std::string();


	::WideCharToMultiByte(CP_UTF8, NULL, wsText.c_str(), static_cast<int>(wsText.length()), pszUTF8, dwLen, NULL, NULL);
	pszUTF8[dwLen] = '\0';
	
	return std::string(pszUTF8);
}

LPSTR CCharacter::UnicodeToGB2312(LPCWSTR pszText, LPSTR pGB2312)
{
	__try
	{
		DWORD dwLen = ::WideCharToMultiByte(CP_ACP, NULL, pszText, -1, NULL, NULL, NULL, NULL);
		char* pszGB2312 = new char[dwLen];
		if (pszGB2312 == nullptr)
			return nullptr;

		::WideCharToMultiByte(CP_ACP, NULL, pszText, static_cast<int>(wcslen(pszText)), pszGB2312, dwLen, NULL, NULL);
		pszGB2312[dwLen] = '\0';
		CCharacter::strcpy_my(pGB2312, pszGB2312, dwLen + 1);
		delete[] pszGB2312;

		return pGB2312;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"UnicodeToGB2312出现异常");
	}
	return pGB2312;
}

LPWSTR CCharacter::GB2312ToUnicode(LPCSTR pGB2312, LPWSTR pszText)
{
	__try
	{
		int nUnicodeLen = ::MultiByteToWideChar(CP_ACP, NULL, pGB2312, -1, NULL, 0);
		wchar_t* szUnicode = new wchar_t[nUnicodeLen + 1];
		if (szUnicode == nullptr)
			return nullptr;

		::MultiByteToWideChar(CP_ACP, NULL, pGB2312, -1, szUnicode, nUnicodeLen);
		szUnicode[nUnicodeLen] = '\0';
		CCharacter::wstrcpy_my(pszText, szUnicode, nUnicodeLen + 1);
		delete[] szUnicode;

		return pszText;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"GB2312ToUnicode出现异常");
	}
	return pszText;
}

int CCharacter::GetRand(int nMax, int nMin)
{
	static DWORD dwSeed = ::GetTickCount();
	srand(dwSeed);
	dwSeed = rand();
	return (nMin + rand() % (nMax - nMin + 1));
}

DWORD CCharacter::GetRand_For_DWORD()
{
	static DWORD dwSeed = ::GetTickCount();
	srand(dwSeed);
	dwSeed = rand() << 15 | rand();
	return dwSeed;
}

DWORD CCharacter::GetRand()
{
	// 个位数
	int nABit = GetRand(0xF + 1);

	// 十位数
	int nTenBit = GetRand(0xF + 1);
	nTenBit = nTenBit << 4;

	// 百位数
	int nHundredBit = GetRand(0xF + 1);
	nHundredBit = nHundredBit << 8;

	// 千位数
	int nThousandBit = GetRand(0xF + 1);
	nThousandBit = nThousandBit << 12;

	// 十万位
	int nTenThousandBit = GetRand(0xF + 1);
	nTenThousandBit = nTenThousandBit << 16;

	// 百万位
	int nHunThouBit = GetRand(0xF + 1);
	nHunThouBit = nHunThouBit << 20;

	// 千万位
	int nThunThunBit = GetRand(0xF + 1);
	nThunThunBit = nThunThunBit << 24;

	// 亿位
	int nHunMillBit = GetRand(0xF + 1);
	nHunMillBit = nHunMillBit << 28;

	return nHunMillBit + nThunThunBit + nHunThouBit + nTenThousandBit + nThousandBit + nHundredBit + nTenBit + nABit;
}

UINT CCharacter::Split(_In_ CONST std::wstring& wsText, _In_ CONST std::wstring& wsFormat, _Out_opt_ std::vector<std::wstring>& vlst, _In_opt_ int emOption)
{
	try
	{
		std::wstring wsTmpText = wsText;
		size_t nIndex = wsTmpText.find(wsFormat);
		while (nIndex != -1)
		{
			std::wstring wstr = wsTmpText.substr(0, nIndex);
			if (emOption & Split_Option_RemoveEmptyEntries)
			{
				if (!CCharacter::Trim_W(wstr).empty())
					vlst.push_back(wstr);
			}
			else if (emOption & Split_Option_None)
			{
				vlst.push_back(wstr);
			}

			wsTmpText = wsTmpText.substr(nIndex + wsFormat.length());
			nIndex = wsTmpText.find(wsFormat);
		}
		if (!wsTmpText.empty() || vlst.size() > 0 || (emOption & Split_Option_KeepOnly))
			vlst.push_back(wsTmpText);
	}
	catch(...)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"Split出现异常");
	}

	return static_cast<UINT>(vlst.size());
}

UINT CCharacter::SplitA(_In_ CONST std::string& sText, _In_ CONST std::string& sFormat, _Out_opt_ std::vector<std::string>& vlst, _In_opt_ int emOption)
{
	try
	{
		std::string sTmpText = sText;
		size_t nIndex = sTmpText.find(sFormat);
		while (nIndex != -1)
		{
			std::string str = sTmpText.substr(0, nIndex);
			if (emOption | Split_Option_RemoveEmptyEntries)
			{
				if (!CCharacter::Trim_A(str).empty())
					vlst.push_back(str);
			}
			else if (emOption | Split_Option_None)
			{
				vlst.push_back(str);
			}

			sTmpText = sTmpText.substr(nIndex + sFormat.length());
			nIndex = sTmpText.find(sFormat);
		}
		if (!sTmpText.empty() && (vlst.size() > 0 || emOption | Split_Option_KeepOnly))
			vlst.push_back(sTmpText);
	}
	catch (...)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"Split出现异常");
	}

	return static_cast<UINT>(vlst.size());
}

BOOL CCharacter::GetSplit_By_List(_In_ std::wstring& wsText, _In_ CONST std::vector<std::wstring>& vSplit, _Out_opt_ std::vector<std::wstring>& vSplitContent, _In_opt_ int emOption)
{
	if (vSplit.size() != 2)
		return FALSE;


	try
	{
		size_t nLeftIndex = wsText.find(vSplit.at(0));
		size_t nRightIndex = wsText.find(vSplit.at(1));
		if (nLeftIndex == -1 || nRightIndex == -1 || nRightIndex <= nLeftIndex)
			return FALSE;

		std::wstring wsSplit = wsText.substr(nLeftIndex + 1, nRightIndex - nLeftIndex - 1);
		CCharacter::Trim_W(wsSplit);

		if (emOption | Split_Option_RemoveEmptyEntries)
		{
			if (wsSplit != L"")
				vSplitContent.push_back(wsSplit);
		}
		else if (emOption | Split_Option_None)
		{
			vSplitContent.push_back(wsSplit);
		}

		wsText = wsText.substr(0, nLeftIndex) + wsText.substr(nRightIndex + 1);
		GetSplit_By_List(wsText, vSplit, vSplitContent, emOption);
	}
	catch(...)
	{
		LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,L"GetSplit_By_List出现异常");
	}


	return vSplitContent.size() > 0;
}

BOOL CCharacter::GetTime_By_Text(_In_ CONST std::wstring& wsText, _Out_opt_ SYSTEMTIME& SysTime)
{
	// 格式: 2015-1-1 2:3:4
	std::vector<std::wstring> vContent;

	ZeroMemory(&SysTime, sizeof(SysTime));
	CCharacter::Split(wsText, L" ", vContent, Split_Option_RemoveEmptyEntries);
	if (vContent.size() != 2)
		return FALSE;

	std::vector<std::wstring> vTime;
	CCharacter::Split(vContent.at(0), L"-", vTime, Split_Option_RemoveEmptyEntries);
	if (vTime.size() != 3)
		return FALSE;

	SysTime.wYear = static_cast<WORD>(_wtoi(vTime.at(0).c_str()));
	SysTime.wMonth = static_cast<WORD>(_wtoi(vTime.at(1).c_str()));
	SysTime.wDay = static_cast<WORD>(_wtoi(vTime.at(2).c_str()));

	vTime.clear();
	CCharacter::Split(vContent.at(1), L":", vTime, Split_Option_RemoveEmptyEntries);
	if (vTime.size() != 3)
		return FALSE;

	SysTime.wHour = static_cast<WORD>(_wtoi(vTime.at(0).c_str()));
	SysTime.wMinute = static_cast<WORD>(_wtoi(vTime.at(1).c_str()));
	SysTime.wSecond = static_cast<WORD>(_wtoi(vTime.at(2).c_str()));
	return TRUE;
}

BOOL CCharacter::ReadUTF8Text(_In_ UINT_PTR dwAddr, _Out_ std::wstring& wsText)
{
	CHAR szText[64] = { 0 };
	WCHAR wszText[64] = { 0 };
	if (ReadDWORD(dwAddr + 0x14) > 0xF)
		strcpy_my(szText, reinterpret_cast<CONST CHAR*>(ReadDWORD(dwAddr)), _countof(szText));
	else
		strcpy_my(szText, reinterpret_cast<CONST CHAR*>(dwAddr), _countof(szText));

	UTF8ToUnicode(wszText, szText);
	wsText = wszText;
	return TRUE;
}

LPCWSTR CCharacter::FormatText(_In_ LPCWSTR pwszFormat, ...)
{
	va_list		args;
	static wchar_t		szBuff[1024] = { 0 };

	va_start(args, pwszFormat);
	_vsnwprintf_s(szBuff, _countof(szBuff) - 1, _TRUNCATE, pwszFormat, args);
	va_end(args);

	return szBuff;
}

std::wstring& CCharacter::FormatText(_Out_ std::wstring& ResultText, _In_ LPCWSTR pwszFormat, ...)
{
	va_list		args;
	wchar_t		szBuff[1024] = { 0 };

	va_start(args, pwszFormat);
	_vsnwprintf_s(szBuff, _countof(szBuff) - 1, _TRUNCATE, pwszFormat, args);
	va_end(args);

	ResultText = szBuff;
	return ResultText;
}

std::wstring CCharacter::MakeTextToLower(_In_ CONST std::wstring& wsText)
{
	return MakeTextTo(wsText, tolower);
}

std::wstring CCharacter::MakeTextToUpper(_In_ CONST std::wstring& wsText)
{
	return MakeTextTo(wsText, toupper);
}

VOID CCharacter::GetVecByParm_RemoveQuotes(_In_ CONST std::wstring& wsText, _In_ WCHAR wchKeyword, _Out_opt_ std::vector<std::wstring>& VecParm)
{
	//"1",2,"3",4,asd,"eqrwer"
	std::wstring wsParm;
	BOOL bQuotes = FALSE;
	for (auto& itm : wsText)
	{
		if (!bQuotes && itm == wchKeyword)
		{
			VecParm.push_back(wsParm);
			wsParm.clear();
			continue;
		}

		if (itm == '\"')
		{
			bQuotes = !bQuotes;
			continue;
		}

		wsParm.push_back(itm);
	}

	if (!wsParm.empty())
		VecParm.push_back(wsParm);
}
