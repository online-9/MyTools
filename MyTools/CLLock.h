#ifndef __CLLOCK_H__
#define __CLLOCK_H__

#include <mutex>
#include "ToolsPublic.h"

class CLLock
{
public:
	CLLock(_In_ std::wstring wsLockName_);
	~CLLock();

	BOOL Access(std::function<void(void)> MethodPtr) CONST;

private:
	void Lock() CONST;
	void UnLock() CONST;

private:
	//HANDLE hMutex;
	mutable std::mutex _Mutex;
	std::wstring _wsLockName;
};



#endif