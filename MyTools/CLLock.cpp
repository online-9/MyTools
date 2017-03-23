#include "stdafx.h"
#include "CLLock.h"
#include "Log.h"
#include <exception>
#include "Character.h"
#define _SELF L"CLLock.cpp"

CLLock::CLLock(_In_ std::wstring wsLockName_) : wsLockName(wsLockName_)
{
	//hMutex = ::CreateMutexW(NULL, FALSE, wsLockName_.c_str());
	::InitializeCriticalSection(&LockSection);
}

CLLock::~CLLock()
{
	//::ReleaseMutex(hMutex);
	//::CloseHandle(hMutex);
	//hMutex = NULL;
	::DeleteCriticalSection(&LockSection);
}

void CLLock::Lock() CONST
{
	//if (WaitForSingleObject(hMutex, 15 * 1000) == WAIT_TIMEOUT)
	//{
	//	LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"Lock超时, LockName=%s", wsLockName.c_str());
	//}
	::EnterCriticalSection(&LockSection);
}

void CLLock::UnLock() CONST
{
	//::ReleaseMutex(hMutex);
	::LeaveCriticalSection(&LockSection);
}

BOOL CLLock::Access(std::function<void(void)> MethodPtr) CONST
{
	BOOL bRetCode = FALSE;
	Lock();

	auto fnAccess = [&MethodPtr,this]
	{
		__try
		{
			MethodPtr();
		}
		__except (1)
		{
			LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"Access发生了异常, LockName=%s", wsLockName.c_str());
		}
	};

	fnAccess();
	bRetCode = TRUE;
	UnLock();
	return bRetCode;
}