#include "stdafx.h"
#include "CLLock.h"
#include "Log.h"
#include <exception>
#include "Character.h"
#define _SELF L"CLLock.cpp"

CLLock::CLLock(_In_ std::wstring wsLockName_) : _wsLockName(wsLockName_)
{
	
}

CLLock::~CLLock()
{
	
	
}

void CLLock::Lock() CONST
{
	_Mutex.lock();
}

void CLLock::UnLock() CONST
{
	_Mutex.unlock();
}

BOOL CLLock::Access(std::function<void(void)> MethodPtr) CONST
{
	auto fnAccess = [&MethodPtr,this]
	{
		__try
		{
			MethodPtr();
		}
		__except (1)
		{
			LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception, L"Access发生了异常, LockName=%s", _wsLockName.c_str());
		}
	};
	Lock();
	fnAccess();
	UnLock();

	return TRUE;
}