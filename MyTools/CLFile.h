#ifndef __CLFILE_H__
#define __CLFILE_H__

#include "ToolsPublic.h"
#include <memory>

class CLFile
{
public:
	CLFile();
	~CLFile();

public:
	static BOOL WINAPI CreateMyDirectory(IN LPCWSTR pwchText, BOOL bShowErr = FALSE);
	static BOOL WINAPI ReadUnicodeFile(_In_ CONST std::wstring& cwsPath, _Out_ std::wstring& cwsContent);
	static BOOL WINAPI WriteUnicodeFile(_In_ CONST std::wstring& cwsPath, _In_ CONST std::wstring& cwsContent);
	static BOOL WINAPI WriteASCIIFile(_In_ CONST std::wstring& cwsPath, _In_ CONST std::wstring& cwsContent);
	static BOOL WINAPI WriteFile(_In_ CONST std::wstring& cwsPath, _In_ CONST BYTE* Buffer, _In_ UINT uSize);
	static BOOL WINAPI AppendFile(_In_ CONST std::wstring& cwsPath, _In_ CONST BYTE* Buffer, _In_ UINT uSize);
	static BOOL WINAPI ReadAsciiFileLen(_In_ CONST std::wstring& cwsPath, _Out_ ULONG& ulFileLen);
	static BOOL WINAPI ReadAsciiFileContent(_In_ CONST std::wstring& cwsPath, _In_ LONG ulFileLen, _Out_ std::shared_ptr<CHAR>& psContent);
	static BOOL WINAPI AppendUnicodeFile(_In_ CONST std::wstring& cwsPath, _In_ CONST std::wstring& cwsContent);
	static BOOL WINAPI CreateUnicodeTextFile(_In_ CONST std::wstring& cwsPath);
	static BOOL WINAPI CreateASCIITextFile(_In_ CONST std::wstring& cwsPath);
	static BOOL WINAPI ClearFileContent(_In_ CONST std::wstring& cwsPath);
};



#endif//__CLFILE_H__