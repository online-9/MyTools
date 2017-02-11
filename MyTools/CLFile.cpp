#include "stdafx.h"
#include "CLFile.h"
#include <mutex>
#include "CLLog.h"
#include "Character.h"
#include "CLPublic.h"
#pragma comment(lib, "version.lib")

#define _SELF L"CLFile.cpp"
CLFile::CLFile()
{
}

CLFile::~CLFile()
{
}


BOOL CLFile::CreateMyDirectory(IN LPCWSTR pwchText, BOOL bShowErr /* = FALSE */)
{
	SECURITY_ATTRIBUTES Atribute;
	Atribute.bInheritHandle = FALSE;
	Atribute.lpSecurityDescriptor = NULL;
	Atribute.nLength = sizeof(SECURITY_ATTRIBUTES);

	if (!::CreateDirectoryW(pwchText, &Atribute) && bShowErr){
		MessageBoxW(NULL, L"创建文件夹失败!这不科学啊~", L"错误", NULL);
		return FALSE;
	}
	return TRUE;
}

BOOL CLFile::ReadUnicodeFile(__in CONST std::wstring& wsPath, __out std::wstring& wsContent)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, wsPath.c_str(), L"rb");
	if (pFile == nullptr)
	{
		Log(LOG_LEVEL_EXCEPTION, L"ReadScriptFile Fiald! Path:%s", wsPath.c_str());
		return FALSE;
	}

	fseek(pFile, 0, SEEK_END);
	LONG lLen = ftell(pFile);
	lLen = (lLen + 2) / 2;

	std::shared_ptr<WCHAR> pwstrBuffer(new WCHAR[lLen], [](WCHAR* p){delete[] p; });
	if (pwstrBuffer == nullptr)
	{
		Log(LOG_LEVEL_EXCEPTION, L"Alloc Memory Fiald!");
		return FALSE;
	}

	ZeroMemory(pwstrBuffer.get(), lLen * sizeof(WCHAR));
	fseek(pFile, 0, SEEK_SET);
	fread(pwstrBuffer.get(), sizeof(WCHAR), (size_t)lLen - 1, pFile);
	pwstrBuffer.get()[lLen - 1] = '\0';

	wsContent = pwstrBuffer.get() + ((pwstrBuffer.get()[0] == 0xFEFF) ? 1 : 0);
	fclose(pFile);
	return TRUE;
}

BOOL CLFile::WriteUnicodeFile(__in CONST std::wstring& wsPath, __in CONST std::wstring& wsContent)
{
	// Exist Write File Mutex
	static std::mutex MutexWriteUnicodeFile;
	std::lock_guard<std::mutex> lck(MutexWriteUnicodeFile);
	if (!CLPublic::FileExit(wsPath.c_str()))
		CreateUnicodeTextFile(wsPath);

	FILE* pFile = nullptr;
	_wfopen_s(&pFile, wsPath.c_str(), L"wb+");
	if (pFile == nullptr)
	{
		Log(LOG_LEVEL_EXCEPTION, L"ReadScriptFile Fiald! Path:%s", wsPath.c_str());
		return FALSE;
	}

	std::shared_ptr<WCHAR> pwstrBuffer(new WCHAR[wsContent.length() + 1], [](WCHAR* p){delete[] p; });
	pwstrBuffer.get()[0] = 0xFEFF;
	memcpy(pwstrBuffer.get() + 1, wsContent.c_str(), wsContent.length() * 2);

	fseek(pFile, 0, SEEK_SET);

	fwrite(pwstrBuffer.get(), sizeof(WCHAR), wsContent.length() + 1, pFile);
	fclose(pFile);
	return TRUE;
}

BOOL WINAPI CLFile::WriteASCIIFile(_In_ CONST std::wstring& wsPath, _In_ CONST std::wstring& wsContent)
{
	// Exist Write File Mutex
	static std::mutex MutexWriteASCIIFile;
	std::lock_guard<std::mutex> lck(MutexWriteASCIIFile);

	if (!CLPublic::FileExit(wsPath.c_str()))
		CreateASCIITextFile(wsPath);

	FILE* pFile = nullptr;
	_wfopen_s(&pFile, wsPath.c_str(), L"a+");
	if (pFile == nullptr)
	{
		Log(LOG_LEVEL_EXCEPTION, L"ReadScriptFile Fiald! Path:%s", wsPath.c_str());
		return FALSE;
	}

	std::string str = CCharacter::UnicodeToASCII(wsContent);

	fseek(pFile, 0, SEEK_SET);

	fwrite(str.c_str(), sizeof(CHAR), str.length(), pFile);
	fclose(pFile);
	return TRUE;
}

BOOL WINAPI CLFile::WriteFile(_In_ CONST std::wstring& cwsPath, _In_ CONST BYTE* Buffer, _In_ UINT uSize)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, cwsPath.c_str(), L"wb+");
	if (pFile == nullptr)
	{
		Log(LOG_LEVEL_EXCEPTION, L"WriteFile Fiald! Path:%s", cwsPath.c_str());
		return FALSE;
	}

	fseek(pFile, 0, SEEK_SET);

	fwrite(Buffer, sizeof(BYTE), uSize, pFile);
	fclose(pFile);
	pFile = nullptr;
	return TRUE;
}

BOOL WINAPI CLFile::AppendFile(_In_ CONST std::wstring& cwsPath, _In_ CONST BYTE* Buffer, _In_ UINT uSize)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, cwsPath.c_str(), L"ab+");
	if (pFile == nullptr)
	{
		Log(LOG_LEVEL_EXCEPTION, L"AppendFile Fiald! Path:%s", cwsPath.c_str());
		return FALSE;
	}

	fseek(pFile, 0, SEEK_SET);

	fwrite(Buffer, sizeof(BYTE), uSize, pFile);
	fclose(pFile);
	pFile = nullptr;
	return TRUE;
}

BOOL WINAPI CLFile::ReadAsciiFileLen(_In_ CONST std::wstring& cwsPath, _Out_ ULONG& ulFileLen)
{
	FILE* pFile = nullptr;
	fopen_s(&pFile, CCharacter::UnicodeToASCII(cwsPath).c_str(), "rb");
	if (pFile == nullptr)
	{
		Log(LOG_LEVEL_EXCEPTION, L"ReadScriptFile Fiald! Path:%s", cwsPath.c_str());
		return FALSE;
	}

	fseek(pFile, 0, SEEK_END);
	LONG lLen = ftell(pFile);
	fclose(pFile);

	ulFileLen = lLen;
	return TRUE;
}

BOOL WINAPI CLFile::ReadAsciiFileContent(_In_ cwstring& cwsPath, _In_ LONG ulFileLen, _Out_ std::shared_ptr<CHAR>& psContent)
{
	FILE* pFile = nullptr;
	fopen_s(&pFile, CCharacter::UnicodeToASCII(cwsPath).c_str(), "rb");
	if (pFile == nullptr)
	{
		Log(LOG_LEVEL_EXCEPTION, L"ReadScriptFile Fiald! Path:%s", cwsPath.c_str());
		return FALSE;
	}

	ZeroMemory(psContent.get(), ulFileLen);
	fseek(pFile, 0, SEEK_SET);
	fread(psContent.get(), sizeof(CHAR), (size_t)ulFileLen, pFile);
	fclose(pFile);
	psContent.get()[ulFileLen] = '\0';
	return TRUE;
}


BOOL WINAPI CLFile::AppendUnicodeFile(__in cwstring& wsPath, __in cwstring& cwsContent)
{
	// Exist Write File Mutex
	static std::mutex MutexAppendUnicodeFile;
	std::lock_guard<std::mutex> lck(MutexAppendUnicodeFile);

	if (!CLPublic::FileExit(wsPath.c_str()))
		CreateUnicodeTextFile(wsPath);

	FILE* pFile = nullptr;
	_wfopen_s(&pFile, wsPath.c_str(), L"ab+");
	if (pFile == nullptr)
	{
		Log(LOG_LEVEL_EXCEPTION, L"AppendUnicodeFile Fiald! Path:%s", wsPath.c_str());
		return FALSE;
	}

	fseek(pFile, 0, SEEK_END);

	std::wstring wsContent = cwsContent;
	if (cwsContent[cwsContent.length() - 1] != '\r\n')
		wsContent.append(L"\r\n");

	fwrite(wsContent.c_str(), sizeof(WCHAR), wsContent.length(), pFile);
	fclose(pFile);
	return TRUE;
}

BOOL WINAPI CLFile::CreateUnicodeTextFile(_In_ CONST std::wstring& cwsPath)
{
	Log(LOG_LEVEL_NORMAL, L"CreateUnicodeTextFile:%s", cwsPath.c_str());
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, cwsPath.c_str(), L"a");
	if (pFile == NULL)
	{
		LogMsgBox(LOG_LEVEL_EXCEPTION, L"创建文件:%s 失败!", cwsPath.c_str());
		return FALSE;
	}

	fseek(pFile, 0, SEEK_SET);
	WCHAR wszFlag = 0xFEFF;
	fwrite(&wszFlag, sizeof(WCHAR), 1, pFile);
	fclose(pFile);
	return TRUE;
}

BOOL WINAPI CLFile::CreateASCIITextFile(_In_ CONST std::wstring& cwsPath)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, cwsPath.c_str(), L"a");
	if (pFile == NULL)
	{
		LogMsgBox(LOG_LEVEL_EXCEPTION, L"创建文件:%s 失败!", cwsPath.c_str());
		return FALSE;
	}

	fclose(pFile);
	return TRUE;
}

BOOL WINAPI CLFile::ClearFileContent(_In_ CONST std::wstring& cwsPath)
{
	FILE* pFile = nullptr;
	_wfopen_s(&pFile, cwsPath.c_str(), L"w+");
	if (pFile != NULL)
		fclose(pFile);

	pFile = nullptr;
	return TRUE;
}

