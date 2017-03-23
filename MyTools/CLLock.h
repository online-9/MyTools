#ifndef __CLLOCK_H__
#define __CLLOCK_H__

#include "ToolsPublic.h"

class CLLock
{
public:
	CLLock(_In_ std::wstring wsLockName_);
	~CLLock();

	BOOL Access(std::function<void(void)> MethodPtr) CONST;

	void Lock() CONST;
	void UnLock() CONST;

private:
	//HANDLE hMutex;
	mutable CRITICAL_SECTION LockSection;
	std::wstring wsLockName;
};



#endif