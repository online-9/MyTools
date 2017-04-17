#include "stdafx.h"
#include "Log.h"
#include <stack>
#include <thread>
#include "CLThread.h"
#include "Character.h"
#include "CLPublic.h"
#include "CLFile.h"
#include "CLResManager.h"
#include <Shlwapi.h>
#include <mutex>

#define _SELF L"Log.cpp"

CLog::CLog() : wsClientName(L"Empty"), bRun(FALSE), m_bOverWrite(TRUE), Lock_LogContentQueue(L"Lock_LogContentQueue"), Lock_SaveLogContentQueue(L"Lock_SaveLogContentQueue"), _ulMaxFileSize(NULL)
{

}

CLog::~CLog()
{
	Release();
}

VOID CLog::Print(_In_ LPCWSTR pwszFunName, _In_ LPCWSTR pwszFileName, _In_ int nLine, _In_ int nLogOutputType, _In_ em_Log_Type emLogType, _In_ BOOL bMsgBox, _In_ LPCWSTR pwszFormat, ...)
{
	va_list		args;
	WCHAR		szBuff[1024];
	va_start(args, pwszFormat);
	_vsnwprintf_s(szBuff, _countof(szBuff) - 1, _TRUNCATE, pwszFormat, args);
	va_end(args);

	LogContent LogContent_;
	LogContent_.emLogType = emLogType;
	LogContent_.uLine = nLine;
	CCharacter::wstrcpy_my(LogContent_.wszClientName, wsClientName.c_str(), _countof(LogContent_.wszClientName) - 1);
	CCharacter::wstrcpy_my(LogContent_.wszFileName, pwszFileName, _countof(LogContent_.wszFileName) - 1);
	CCharacter::wstrcpy_my(LogContent_.wszFunName, pwszFunName, _countof(LogContent_.wszFunName) - 1);
	CCharacter::wstrcpy_my(LogContent_.wszLogContent, szBuff, _countof(LogContent_.wszLogContent) - 1);

	LogContent_.uRepeatCount = 0;
	LogContent_.uStackCount = 0;

	if (nLogOutputType & LOG_TYPE_FILE)
		AddSaveLogToQueue(LogContent_);
	if (nLogOutputType & LOG_TYPE_CONSOLE)
		AddLogContentToQueue(LogContent_);

	if (bMsgBox)
	{
		::MessageBoxW(NULL, szBuff, wsClientName.c_str(), NULL);
	}
}


VOID CLog::Release()
{
	bRun = FALSE;
	if (hSendExitEvent != NULL)
	{
		::WaitForSingleObject(hSendExitEvent, INFINITE);
		::CloseHandle(hSendExitEvent);
		hSendExitEvent = NULL;
	}
	if (hSaveLogEvent != NULL)
	{
		::WaitForSingleObject(hSaveLogEvent, INFINITE);
		::CloseHandle(hSaveLogEvent);
		hSaveLogEvent = NULL;
	}
}

VOID CLog::SetClientName(_In_ CONST std::wstring& cwsClientName, _In_ CONST std::wstring wsSaveLogPath, _In_ BOOL bOverWrite, _In_ ULONG ulMaxSize)
{
	hSendExitEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);
	hSaveLogEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);

	wsClientName = cwsClientName;

	SYSTEMTIME SysTime;
	::GetLocalTime(&SysTime);

	if (bOverWrite)
		wsLogFilePath = wsSaveLogPath + cwsClientName + L".log";
	else
	{
		WCHAR wszText[64];
		swprintf_s(wszText, _countof(wszText), L"%s_%d-%d-%d.log", cwsClientName.c_str(), static_cast<DWORD>(SysTime.wYear), static_cast<DWORD>(SysTime.wMonth), static_cast<DWORD>(SysTime.wDay));
		wsLogFilePath = wsSaveLogPath + wszText;
	}

	if (!CLPublic::FileExist(wsLogFilePath))
		CLFile::WriteUnicodeFile(wsLogFilePath, L"");
	else if (bOverWrite && CLPublic::FileExist(wsLogFilePath))
	{
		ULONG ulLen = 0;
		CLFile::ReadAsciiFileLen(wsLogFilePath, ulLen);
		if (ulLen >= ulMaxSize)
			CLFile::WriteUnicodeFile(wsLogFilePath, L"");;
	}

	memcpy(&CurrentSysTime, &SysTime, sizeof(SysTime));
	m_bOverWrite = bOverWrite;
	_ulMaxFileSize = ulMaxSize;

	LOG_CF(CLog::em_Log_Type::em_Log_Type_Debug, L"------------Run Time=[%d-%d-%d %d:%d:%d] ----------------------------",							\
		static_cast<DWORD>(SysTime.wYear), static_cast<DWORD>(SysTime.wMonth), static_cast<DWORD>(SysTime.wDay), static_cast<DWORD>(SysTime.wHour), \
		static_cast<DWORD>(SysTime.wMinute), static_cast<DWORD>(SysTime.wSecond));

	bRun = TRUE;

	auto hSendThread = cbBEGINTHREADEX(NULL, NULL, _SendThread, this, NULL, NULL);
	SetResDeleter(hSendThread, [](HANDLE& hThread) { ::CloseHandle(hThread); });

	auto hSaveThread = cbBEGINTHREADEX(NULL, NULL, _SaveThread, this, NULL, NULL);
	SetResDeleter(hSaveThread, [](HANDLE& hThread) {::CloseHandle(hThread); });
}

BOOL CLog::PrintTo(_In_ CONST LogContent& LogContent_)
{
	HANDLE hMutex = ::OpenMutexW(MUTEX_ALL_ACCESS, FALSE, CL_LOG_MUTEX); // wait for LogServer
	if (hMutex == NULL)
		return FALSE;

	::CloseHandle(hMutex);
	hMutex = NULL;

	HANDLE hReadyEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, CL_LOG_READY_EVENT);
	if (hReadyEvent == NULL)
		return FALSE;

	HANDLE hBufferEvent = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, CL_LOG_BUFFER_EVENT);
	if (hBufferEvent == NULL)
	{
		::CloseHandle(hBufferEvent);
		return FALSE;
	}

	HANDLE hFileMap = ::OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, CL_LOG_SHAREMEM);
	if (hFileMap == NULL)
	{
		::CloseHandle(hBufferEvent);
		::CloseHandle(hReadyEvent);
		return FALSE;
	}

	LogContent* pLogContent = (LogContent*)MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, NULL, NULL, sizeof(LogContent));
	if (pLogContent == nullptr)
	{
		::CloseHandle(hBufferEvent);
		::CloseHandle(hReadyEvent);
		::CloseHandle(hFileMap);
		return FALSE;
	}

	if (::WaitForSingleObject(hReadyEvent, 1000) != WAIT_TIMEOUT)
	{
		*pLogContent = LogContent_;
		::SetEvent(hBufferEvent);
	}

	::UnmapViewOfFile(pLogContent);
	::CloseHandle(hFileMap);
	::CloseHandle(hBufferEvent);
	::CloseHandle(hReadyEvent);
	return TRUE;
}

BOOL CLog::SaveLog(_In_ CONST LogContent& LogContent_) CONST
{
	WCHAR wszText[1024] = { 0 };
	std::wstring wsContent;

	SYSTEMTIME SysTime;
	::GetLocalTime(&SysTime);

	if ((!m_bOverWrite && SysTime.wDay != CurrentSysTime.wDay) || !CLPublic::FileExist(wsLogFilePath))
	{
		// have to create new log!
		WCHAR wszFileName[64];
		swprintf_s(wszFileName, _countof(wszFileName), L"%s_%d-%d-%d.log", wsClientName.c_str(), static_cast<DWORD>(SysTime.wYear), static_cast<DWORD>(SysTime.wMonth), static_cast<DWORD>(SysTime.wDay));

		CCharacter::wstrcpy_my(wszText, wsLogFilePath.c_str(), _countof(wszText));
		::PathRemoveFileSpec(wszText);
		lstrcatW(wszText, L"\\");
		lstrcatW(wszText, wszFileName);
		wsLogFilePath = wszText;
		CLFile::WriteUnicodeFile(wsLogFilePath, L"");;
	}

	static int Count = 0;
	if (++Count % 100 == 0)
	{
		ULONG ulLen = 0;
		CLFile::ReadAsciiFileLen(wsLogFilePath, ulLen);
		if (ulLen >= _ulMaxFileSize)
			CLFile::WriteUnicodeFile(wsLogFilePath, L"");;
	}
	

	wsContent += L"#Stack:\r\n";
	swprintf_s(wszText, _countof(wszText) - 1, L" #Time:%02d:%02d:%02d ", static_cast<DWORD>(SysTime.wHour), static_cast<DWORD>(SysTime.wMinute), static_cast<DWORD>(SysTime.wSecond));
	wsContent += wszText;
	wsContent += L" #Client:";
	wsContent += LogContent_.wszClientName;
	wsContent += L"	#Level:";

	swprintf_s(wszText, _countof(wszText) - 1, L"%d", LogContent_.emLogType);
	wsContent += wszText;

	wsContent += L"	#File:";
	wsContent += LogContent_.wszFileName;

	wsContent += L"	#FunName:";
	wsContent += LogContent_.wszFunName;

	swprintf_s(wszText, _countof(wszText) - 1, L" Line:%d", LogContent_.uLine);
	wsContent += wszText;

	wsContent += L"	#Content:";
	wsContent += LogContent_.wszLogContent;
	wsContent += L"\r\n";

	return CLFile::AppendUnicodeFile(wsLogFilePath, wsContent);
}

DWORD WINAPI CLog::_SendThread(LPVOID lpParm)
{
	auto pTestLog = reinterpret_cast<CLog*>(lpParm);
	LogContent LogContent_;
	while (pTestLog->bRun)
	{
		HANDLE hMutex = ::OpenMutexW(MUTEX_ALL_ACCESS, FALSE, CL_LOG_MUTEX); // wait for LogServer
		if (hMutex == NULL)
		{
			pTestLog->GetLogContentForQueue(LogContent_);
			::Sleep(50);
			continue;
		}

		::ReleaseMutex(hMutex);

		HANDLE hReadyEvent = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, CL_LOG_READY_EVENT);
		if (hReadyEvent == NULL)
			return FALSE;

		HANDLE hBufferEvent = ::OpenEventW(EVENT_ALL_ACCESS, FALSE, CL_LOG_BUFFER_EVENT);
		if (hBufferEvent == NULL)
			return FALSE;

		HANDLE hFileMap = ::OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, CL_LOG_SHAREMEM);
		if (hFileMap == NULL)
		{
			::CloseHandle(hBufferEvent);
			::CloseHandle(hReadyEvent);
			return FALSE;
		}

		LogContent* pLogContent = (LogContent*)MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, NULL, NULL, sizeof(LogContent));
		if (pLogContent == nullptr)
		{
			::CloseHandle(hBufferEvent);
			::CloseHandle(hReadyEvent);
			::CloseHandle(hFileMap);
			return FALSE;
		}

		while (pTestLog->bRun)
		{
			if (!pTestLog->GetLogContentForQueue(LogContent_))
			{
				::Sleep(50);
				continue;
			}

			if (::WaitForSingleObject(hReadyEvent, 1000) != WAIT_TIMEOUT)
			{
				*pLogContent = LogContent_;
				::SetEvent(hBufferEvent);
			}

			hMutex = ::OpenMutexW(MUTEX_ALL_ACCESS, FALSE, CL_LOG_MUTEX); // wait for LogServer
			if (hMutex == NULL)
				break;

			::ReleaseMutex(hMutex);
			hMutex = NULL;
		}

		::UnmapViewOfFile(pLogContent);
		::CloseHandle(hFileMap);
		::CloseHandle(hBufferEvent);
		::CloseHandle(hReadyEvent);
	}
	::SetEvent(pTestLog->hSendExitEvent);
	return 0;
}

VOID CLog::AddLogContentToQueue(_In_ CONST LogContent& LogContent_)
{
	Lock_LogContentQueue.Access([this, &LogContent_]
	{
		if (QueueLogContent.size() >= 1000)
			return;

		QueueLogContent.push(LogContent_);
	});
}

BOOL CLog::GetLogContentForQueue(_Out_ LogContent& LogContent_)
{
	BOOL bExist = FALSE;
	Lock_LogContentQueue.Access([this, &LogContent_, &bExist]
	{
		if (!QueueLogContent.empty())
		{
			LogContent_ = std::move(QueueLogContent.front());
			QueueLogContent.pop();
			bExist = TRUE;
		}
	});
	return bExist;
}

VOID CLog::AddSaveLogToQueue(_In_ CONST LogContent& LogContent_)
{
	Lock_SaveLogContentQueue.Access([this, &LogContent_]
	{
		if (QueueSaveLogContent.size() >= 1000)
			return;

		QueueSaveLogContent.push(std::move(LogContent_));
	});
}

BOOL CLog::GetSaveLogContentForQueue(_Out_ LogContent& LogContent_)
{
	BOOL bExist = FALSE;
	Lock_SaveLogContentQueue.Access([this, &LogContent_, &bExist]
	{
		if (!QueueSaveLogContent.empty())
		{
			LogContent_ = std::move(QueueSaveLogContent.front());
			QueueSaveLogContent.pop();
			bExist = TRUE;
		}
	});
	return bExist;
}

DWORD WINAPI CLog::_SaveThread(LPVOID lpParm)
{
	auto pTestLog = reinterpret_cast<CLog*>(lpParm);
	LogContent LogContent_;
	while (pTestLog->bRun)
	{
		if (!pTestLog->GetSaveLogContentForQueue(LogContent_))
		{
			::Sleep(100);
			continue;
		}
		pTestLog->SaveLog(LogContent_);
	}
	::SetEvent(pTestLog->hSaveLogEvent);
	return 0;
}
