#include "stdafx.h"
#include "CLLock.h"
#include "CLLog.h"
#include <exception>
#include "Character.h"
#include "CLStackTrace.h"
#define _SELF L"CLLock.cpp"

CLLock::CLLock(_In_ std::wstring wsLockName_) : wsLockName(wsLockName_)
{
	hMutex = ::CreateMutexW(NULL, FALSE, wsLockName_.c_str());
	//InitializeCriticalSection(&cs);
}

CLLock::~CLLock()
{
	::ReleaseMutex(hMutex);
	::CloseHandle(hMutex);
	hMutex = NULL;
}

void CLLock::Lock() CONST
{
	if (WaitForSingleObject(hMutex, 15 * 1000) == WAIT_TIMEOUT)
	{
		Log(LOG_LEVEL_EXCEPTION, L"Lock超时, LockName=%s", wsLockName.c_str());
	}
}

void CLLock::UnLock() CONST
{
	::ReleaseMutex(hMutex);
}

BOOL CLLock::Access(std::function<void(void)> f) CONST
{
	BOOL bRetCode = FALSE;
	Lock();

	auto fnAccess = [&f,this]
	{
		__try
		{
			f();
		}
		__except (CLLog::PrintExceptionCode(GetExceptionInformation()))
		{
			Log(LOG_LEVEL_EXCEPTION, L"Access发生了异常, LockName=%s", wsLockName.c_str());
		}
	};

	fnAccess();
	bRetCode = TRUE;
	UnLock();
	return bRetCode;
}