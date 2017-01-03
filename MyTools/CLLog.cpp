#include "stdafx.h"
#include "CLLog.h"
#include "CLStackTrace.h"
#include "Character.h"
#include "CLLock.h"
#include "CLFile.h"
#include "CLPublic.h"

#define _SELF L"CLLog.cpp"

std::wstring CLLog::wsProjectName = L"UnInit";
std::wstring CLLog::wsLocalLogPath;

BOOL CLLog::Print(_In_ LPCWSTR pwszFunName, _In_ LPCWSTR pwszFileName, _In_ int nLine, _In_ LPCWSTR pwszMsgLevel, _In_ LPCWSTR pwszFormat, ...)
{
#ifdef DISABLELOG
	return TRUE;
#endif // DISABLELOG

	va_list		args;
	WCHAR		szBuff[1024];
	va_start(args, pwszFormat);
	_vsnwprintf_s(szBuff, _countof(szBuff) - 1, _TRUNCATE, pwszFormat, args);
	va_end(args);

	// 添加结构
	static DWORD dwLastHash = 0;
	static ULONGLONG ulTick = 0;

	_LogContent LogContent;
	CCharacter::wstrcpy_my(LogContent.wszClientName, wsProjectName.c_str(), wsProjectName.length());
	swprintf_s(LogContent.wszFileName, _countof(LogContent.wszFileName) - 1, L"%s:%d", pwszFileName, nLine);
	CCharacter::wstrcpy_my(LogContent.wszFunName, pwszFunName, _countof(LogContent.wszFunName) - 1);
	CCharacter::wstrcpy_my(LogContent.wszLogLevel, pwszMsgLevel, _countof(LogContent.wszLogLevel) - 1);
	CCharacter::wstrcpy_my(LogContent.wszLogContent, szBuff, _countof(LogContent.wszLogContent) - 1);
	::GetLocalTime(&LogContent.SysTime);

	std::stack<CLStackGroup::FunctionStackContent> vStackTracer;
	CLStackGroup::GetInstance().CopyThreadStackContent(::GetCurrentThreadId(), vStackTracer);

	LogContent.uRepeatCount = 0;
	LogContent.uStackCount = vStackTracer.size() >= 10 ? 10 : vStackTracer.size();
	for (INT i = 0; !vStackTracer.empty() && i < 10; ++i)
	{
		auto& itm = vStackTracer.top();
		CCharacter::wstrcpy_my(LogContent.wszStack[i], itm.wsFunName.c_str(), _countof(LogContent.wszStack[i]) - 1);
		vStackTracer.pop();
	}

	static CLLock Lock(L"CLLog::Print");
	return Lock.Access([&LogContent]{
		PrintLogTo(LogContent);
	});
}


BOOL CLLog::MsgBox(_In_ LPCWSTR pwszFunName, _In_ LPCWSTR pwszFileName, _In_ int nLine, _In_ LPCWSTR pwszMsgLevel, _In_ LPCWSTR pwszFormat, ...)
{
	va_list		args;
	WCHAR		szBuff[1024];
	va_start(args, pwszFormat);
	_vsnwprintf_s(szBuff, _countof(szBuff) - 1, _TRUNCATE, pwszFormat, args);
	va_end(args);

	Print(pwszFunName, pwszFileName, nLine, pwszMsgLevel, szBuff);
	return MessageBoxW(NULL, szBuff, wsProjectName == L"" ? L"Prompt" : wsProjectName.c_str(), NULL);
}


BOOL CLLog::SetClientName(_In_ CONST std::wstring& wsClientName)
{
	wsProjectName = wsClientName;

	WCHAR wszPath[MAX_PATH];
	::GetCurrentDirectoryW(MAX_PATH, wszPath);
	lstrcatW(wszPath, L"\\Log\\");
	if (!CLPublic::FileExit(wszPath))
		CLFile::CreateMyDirectory(wszPath);

	lstrcatW(wszPath, wsProjectName.c_str());
	lstrcatW(wszPath, L".CLLog");
	wsLocalLogPath = wszPath;

	if (!CLPublic::FileExit(wszPath))
		return CLFile::WriteUnicodeFile(wsLocalLogPath, L"#    Build in Process!\r\n");

	ULONG ulLen = 0;
	CLFile::ReadAsciiFileLen(wszPath, ulLen);
	if (ulLen >= 1024 * 1024 * 20)
		return CLFile::WriteUnicodeFile(wsLocalLogPath, L"#    Build in Process!\r\n");

	return TRUE;
}

BOOL CLLog::PrintLogTo(_In_ _LogContent& LogContent)
{
	HANDLE hMutex = ::OpenMutexW(MUTEX_ALL_ACCESS, FALSE, CLLOG_MUTEX); // wait for LogServer
	if (hMutex == NULL)
		return SaveLog(LogContent);

	::CloseHandle(hMutex);
	hMutex = NULL;

	HANDLE hReadyEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, CLLOG_READY_EVENT);
	if (hReadyEvent == NULL)
		return FALSE;

	HANDLE hBufferEvent = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, CLLOG_BUFFER_EVENT);
	if (hBufferEvent == NULL)
	{
		::CloseHandle(hBufferEvent);
		return FALSE;
	}

	HANDLE hFileMap = ::OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, CLLOG_SHAREMEM);
	if (hFileMap == NULL)
	{
		::CloseHandle(hBufferEvent);
		::CloseHandle(hReadyEvent);
		return FALSE;
	}

	_LogContent* pLogContent = (_LogContent*)MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, NULL, NULL, sizeof(_LogContent));
	if (pLogContent == nullptr)
	{
		::CloseHandle(hBufferEvent);
		::CloseHandle(hReadyEvent);
		::CloseHandle(hFileMap);
		return FALSE;
	}

	if (::WaitForSingleObject(hReadyEvent, 1000) != WAIT_TIMEOUT)
	{
		*pLogContent = LogContent;
		::SetEvent(hBufferEvent);
	}

	::UnmapViewOfFile(pLogContent);
	::CloseHandle(hFileMap);
	::CloseHandle(hBufferEvent);
	::CloseHandle(hReadyEvent);
	return TRUE;
}

BOOL CLLog::SaveLog(_In_ _LogContent& LogContent)
{
	if (!CLPublic::FileExit(wsLocalLogPath.c_str()))
	{
		return FALSE;
	}

	static WCHAR wszText[1024];
	std::wstring wsContent;

	wsContent += L"#Stack:\r\n";
	swprintf_s(wszText, _countof(wszText) - 1, L" #Time:%02d:%02d:%02d ", LogContent.SysTime.wHour, LogContent.SysTime.wMinute, LogContent.SysTime.wSecond);
	wsContent += wszText;
	wsContent += L" #Client:";
	wsContent += LogContent.wszClientName;
	wsContent += L"	#Level:";
	wsContent += LogContent.wszLogLevel;
	wsContent += L"	#File:";
	wsContent += LogContent.wszFileName;
	wsContent += L"	#FunName:";
	wsContent += LogContent.wszFunName;
	wsContent += L"	#Content:";
	wsContent += LogContent.wszLogContent;
	wsContent += L"\r\n";

	return CLFile::AppendUnicodeFile(wsLocalLogPath, wsContent);
}

BOOL CLLog::PrintExceptionCode(_In_ LPEXCEPTION_POINTERS ExceptionPtr)
{
	Log(LOG_LEVEL_EXCEPTION, L"ExceptionCode=%X, EIP=%X, Addr=%X", ExceptionPtr->ExceptionRecord->ExceptionCode, ExceptionPtr->ContextRecord->Eip, ExceptionPtr->ExceptionRecord->ExceptionAddress);
	switch (ExceptionPtr->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		Log(LOG_LEVEL_EXCEPTION, L"内存地址非法访问异常!");
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		Log(LOG_LEVEL_EXCEPTION, L"数据类型未对齐异常");
		break;
	case EXCEPTION_BREAKPOINT:
		Log(LOG_LEVEL_EXCEPTION, L"中断异常!");
		break;
	case EXCEPTION_SINGLE_STEP:
		Log(LOG_LEVEL_EXCEPTION, L"单步中断异常");
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		Log(LOG_LEVEL_EXCEPTION, L"数组越界");
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO: case EXCEPTION_INT_DIVIDE_BY_ZERO:
		Log(LOG_LEVEL_EXCEPTION, L"除以0异常!");
		break;
	case EXCEPTION_INT_OVERFLOW: case EXCEPTION_FLT_OVERFLOW:
		Log(LOG_LEVEL_EXCEPTION, L"数据溢出异常");
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		Log(LOG_LEVEL_EXCEPTION, L"浮点数计算异常");
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		Log(LOG_LEVEL_EXCEPTION, L"页错误异常");
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		Log(LOG_LEVEL_EXCEPTION, L"非法指令异常");
		break;
	case EXCEPTION_STACK_OVERFLOW:
		Log(LOG_LEVEL_EXCEPTION, L"堆栈溢出!");
		break;
	case EXCEPTION_INVALID_HANDLE:
		Log(LOG_LEVEL_EXCEPTION, L"无效句柄异常!");
		break;
	default:
		break;
	}
	return TRUE; // return TRUE 表示异常被处理, 否则继续往上一层抛!
}
