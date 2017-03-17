#include "stdafx.h"
#include "Log.h"
#include <stack>
#include <thread>
#include "CLThread.h"
#include "Character.h"
#include "CLStackTrace.h"
#include "CLPublic.h"
#include "CLFile.h"
#include "CLExpressionCalc.h"
#include "CLResManager.h"
#include <Shlwapi.h>

#define _SELF L"Log.cpp"

CLog::CLog() : wsClientName(L"Empty"), bRun(FALSE), m_bOverWrite(TRUE), Lock_LogContentQueue(L"Lock_LogContentQueue"), Lock_SaveLogContentQueue(L"Lock_SaveLogContentQueue")
{
	ZeroMemory(&CurrentSysTime, sizeof(CurrentSysTime));
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

	if (bMsgBox)
	{
		::MessageBoxW(NULL, szBuff, wsClientName.c_str(), NULL);
	}

	LogContent LogContent_;
	LogContent_.emLogType = emLogType;
	LogContent_.uLine = nLine;
	CCharacter::wstrcpy_my(LogContent_.wszClientName, wsClientName.c_str(), _countof(LogContent_.wszClientName) - 1);
	CCharacter::wstrcpy_my(LogContent_.wszFileName, pwszFileName, _countof(LogContent_.wszFileName) - 1);
	CCharacter::wstrcpy_my(LogContent_.wszFunName, pwszFunName, _countof(LogContent_.wszFunName) - 1);
	CCharacter::wstrcpy_my(LogContent_.wszLogContent, szBuff, _countof(LogContent_.wszLogContent));

	std::stack<CLStackGroup::FunctionStackContent> vStackTracer;
	CLStackGroup::GetInstance().CopyThreadStackContent(::GetCurrentThreadId(), vStackTracer);

	LogContent_.uRepeatCount = 0;
	LogContent_.uStackCount = vStackTracer.size() >= 10 ? 10 : vStackTracer.size();
	for (INT i = 0; !vStackTracer.empty() && i < 10; ++i)
	{
		auto& itm = vStackTracer.top();
		CCharacter::wstrcpy_my(LogContent_.wszStack[i], itm.wsFunName.c_str(), _countof(LogContent_.wszStack[i]) - 1);
		vStackTracer.pop();
	}

	if (nLogOutputType & LOG_TYPE_FILE)
	{
		AddSaveLogToQueue(LogContent_);
		//SaveLog(LogContent_);
	}
	if (nLogOutputType & LOG_TYPE_CONSOLE)
	{
		AddLogContentToQueue(LogContent_);
		//static CLLock Lock(L"Log.Print.Lock");
		//Lock.Access([LogContent_, this]{ PrintTo(LogContent_); });
	}
}


VOID CLog::Release()
{
	bRun = FALSE;
	if (hReleaseEvent != NULL)
	{
		::WaitForSingleObject(hReleaseEvent, INFINITE);
		::CloseHandle(hReleaseEvent);
		hReleaseEvent = NULL;
	}
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
	hReleaseEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);
	hWorkExitEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);
	hSendExitEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);
	hSaveLogEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);

	wsClientName = cwsClientName;
	bRun = TRUE;
	auto hWorkThread = cbBEGINTHREADEX(NULL, NULL, _WorkThread, this, NULL, NULL);
	SetResDeleter(hWorkThread, [](HANDLE& hThread){::CloseHandle(hThread); });

	auto hSendThread = cbBEGINTHREADEX(NULL, NULL, _SendThread, this, NULL, NULL);
	SetResDeleter(hSendThread, [](HANDLE& hThread){ ::CloseHandle(hThread); });

	auto hSaveThread = cbBEGINTHREADEX(NULL, NULL, _SaveThread, this, NULL, NULL);
	SetResDeleter(hSaveThread, [](HANDLE& hThread){::CloseHandle(hThread); });

	SYSTEMTIME SysTime;
	::GetLocalTime(&SysTime);

	if (bOverWrite)
		wsLogFilePath = wsSaveLogPath + cwsClientName + L".log";
	else
	{
		WCHAR wszText[64];
		swprintf_s(wszText, _countof(wszText), L"%s_%d-%d-%d.log", cwsClientName.c_str(), SysTime.wYear, SysTime.wMonth, SysTime.wDay);
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

	CurrentSysTime = SysTime;
	m_bOverWrite = bOverWrite;
	LOG_CF(CLog::em_Log_Type::em_Log_Type_Debug, L"------------Run Time=[%d-%d-%d %d:%d:%d] ----------------------------", \
		SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
}

CLExpression& CLog::GetLogExpr() throw()
{
	return Expr;
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

BOOL CLog::SaveLog(_In_ LogContent& LogContent_)
{
	WCHAR wszText[1024];
	std::wstring wsContent;

	SYSTEMTIME SysTime;
	::GetLocalTime(&SysTime);

	if ((!m_bOverWrite && SysTime.wDay != CurrentSysTime.wDay) || !CLPublic::FileExit(wsLogFilePath.c_str()))
	{
		// have to create new log!
		WCHAR wszFileName[64];
		swprintf_s(wszFileName, _countof(wszFileName), L"%s_%d-%d-%d.log", wsClientName.c_str(), SysTime.wYear, SysTime.wMonth, SysTime.wDay);

		CCharacter::wstrcpy_my(wszText, wsLogFilePath.c_str(), _countof(wszText));
		::PathRemoveFileSpec(wszText);
		lstrcatW(wszText, L"\\");
		lstrcatW(wszText, wszFileName);
		wsLogFilePath = wszText;
		CLFile::WriteUnicodeFile(wsLogFilePath, L"");;
	}

	wsContent += L"#Stack:\r\n";
	swprintf_s(wszText, _countof(wszText) - 1, L" #Time:%02d:%02d:%02d ", SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
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

		while (pTestLog->bRun)
		{
			if (!pTestLog->GetLogContentForQueue(LogContent_))
			{
				::Sleep(50);
				continue;
			}

			hMutex = ::OpenMutexW(MUTEX_ALL_ACCESS, FALSE, CL_LOG_MUTEX); // wait for LogServer
			if (hMutex == NULL)
				break;

			::CloseHandle(hMutex);
			hMutex = NULL;

			if (::WaitForSingleObject(hReadyEvent, 1000) != WAIT_TIMEOUT)
			{
				*pLogContent = LogContent_;
				::SetEvent(hBufferEvent);
			}
		}

		::UnmapViewOfFile(pLogContent);
		::CloseHandle(hFileMap);
		::CloseHandle(hBufferEvent);
		::CloseHandle(hReadyEvent);
	}
	::SetEvent(pTestLog->hSendExitEvent);
	return 0;
}

DWORD WINAPI CLog::_WorkThread(LPVOID lpParm)
{
	auto pTestLog = reinterpret_cast<CLog*>(lpParm);

	HANDLE hMutex = ::CreateMutexW(NULL, FALSE, CL_LOG_CMD_MUTEX);
	if (hMutex == NULL)
	{
		MessageBoxW(NULL, L"CreateMutex Fiald!", L"Error", NULL);
		::SetEvent(pTestLog->hReleaseEvent);
		return 0;
	}

	HANDLE hReadyEvent = ::CreateEventW(NULL, FALSE, FALSE, CL_LOG_CMD_READY_EVENT);
	if (hReadyEvent == NULL)
	{
		MessageBoxW(NULL, L"CreateEventW Fiald!", L"Error", NULL);
		::CloseHandle(hMutex);
		::SetEvent(pTestLog->hReleaseEvent);
		return 0;
	}

	HANDLE hBufferEvent = ::CreateEventW(NULL, FALSE, FALSE, CL_LOG_CMD_BUFFER_EVENT);
	if (hBufferEvent == NULL)
	{
		MessageBoxW(NULL, L"CreateEventW Fiald!", L"Error", NULL);
		::CloseHandle(hBufferEvent);
		::CloseHandle(hMutex);
		::SetEvent(pTestLog->hReleaseEvent);
		return 0;
	}

	HANDLE hMapFile = ::CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, NULL, sizeof(CmdLogContent), CL_LOG_CMD_SHAREMEM);
	if (hMapFile == NULL)
	{
		MessageBoxW(NULL, L"CreateFileMappingW Fiald!", L"Error", NULL);
		::CloseHandle(hReadyEvent);
		::CloseHandle(hBufferEvent);
		::CloseHandle(hMutex);
		::SetEvent(pTestLog->hReleaseEvent);
		return 0;
	}

	CmdLogContent* pLogContent = (CmdLogContent *)MapViewOfFile(hMapFile, FILE_MAP_READ, NULL, NULL, NULL);
	if (pLogContent == NULL)
	{
		MessageBoxW(NULL, L"MapViewOfFile Fiald!", L"Error", NULL);
		::CloseHandle(hReadyEvent);
		::CloseHandle(hBufferEvent);
		::CloseHandle(hMutex);
		::CloseHandle(hMapFile);
		::SetEvent(pTestLog->hReleaseEvent);
		return 0;
	}

	
	while (pTestLog->bRun)
	{
		::SetEvent(hReadyEvent);
		if (::WaitForSingleObject(hBufferEvent, 50) == WAIT_TIMEOUT)
			continue;

		auto pCmdLogContent = std::make_shared<CmdLogContent>(*pLogContent);
		pTestLog->ExcuteLogServerCmd(pCmdLogContent);
	}

	::UnmapViewOfFile(pLogContent);
	::CloseHandle(hReadyEvent);
	::CloseHandle(hBufferEvent);
	::CloseHandle(hMutex);
	::CloseHandle(hMapFile);

	::SetEvent(pTestLog->hReleaseEvent);
	return 0;
}

VOID CLog::ExcuteLogServerCmd(_In_ std::shared_ptr<CmdLogContent> CmdLogContent_)
{
	std::thread t([CmdLogContent_, this]
	{
		if (!CCharacter::wstrcmp_my(CmdLogContent_->wszClientName, L"ALL") && !CCharacter::wstrcmp_my(CmdLogContent_->wszClientName, wsClientName.c_str()))
			return;

		LOG_C(em_Log_Type::em_Log_Type_Custome, L"Client:%s Excute Cmd Text:%s", CmdLogContent_->wszClientName, CmdLogContent_->wszCmd);
		CLExpressionCalc ExpAnalysis;

		std::wstring wsExpText = CmdLogContent_->wszCmd;
		if (ExpAnalysis.IsConformToCmdType(wsExpText))
			ExpAnalysis.Analysis(wsExpText);
		else
			Expr.Run(wsExpText);
	});
	t.detach();
}

VOID CLog::AddLogContentToQueue(_In_ CONST LogContent& LogContent_)
{
	Lock_LogContentQueue.Access([this, &LogContent_]
	{
		if (QueueLogContent.size() >= 1000)
			return;

		QueueLogContent.push(std::move(LogContent_));
	});
}

BOOL CLog::GetLogContentForQueue(_Out_ LogContent& LogContent_)
{
	BOOL bExist = FALSE;
	Lock_LogContentQueue.Access([this, &LogContent_, &bExist]
	{
		if (!QueueLogContent.empty())
		{
			LogContent_ = QueueLogContent.front();
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
			LogContent_ = QueueSaveLogContent.front();
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
