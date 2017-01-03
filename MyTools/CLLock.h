#ifndef __CLLOCK_H__
#define __CLLOCK_H__

#include "ToolsPublic.h"

class CLLock
{
public:
	CLLock(_In_ std::wstring wsLockName_);
	~CLLock();

	BOOL Access(std::function<void(void)> f) CONST;

	void Lock() CONST;
	void UnLock() CONST;

private:
	HANDLE hMutex;
	std::wstring wsLockName;
};



#endif