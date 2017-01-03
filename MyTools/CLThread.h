#ifndef __CLTHREAD_H__
#define __CLTHREAD_H__ TRUE

#include "ToolsPublic.h"

typedef enum _CL_THREAD_STATUS
{
	CL_THREAD_STATUS_RUN,
	CL_THREAD_STATUS_SUSPEND,
	CL_THREAD_STATUS_RESUME,
	CL_THREAD_STATUS_EXIT,
	CL_THREAD_STATUS_TERMINATE
}CL_THREAD_STATUS;

typedef struct _CL_THREAD_INFO
{
	DWORD	dwThreadId;
	WCHAR	wchThreadName[32];
	CL_THREAD_STATUS Thread_Status;
}CL_THREAD_INFO;

typedef struct _CL_PROCESS_THREADINFO
{
	LPVOID pvStartAddr;				// 线程的起始地址
	DWORD dwTid;					// 线程Id
	WCHAR wszModuleName[MAX_PATH];	// 所属的模块路径
	_CL_PROCESS_THREADINFO()
	{
		pvStartAddr = NULL;
		dwTid = NULL;
		ZeroMemory(wszModuleName, sizeof(wszModuleName));
	}
}CL_PROCESS_THREADINFO;


class CLThread
{
public:
	CLThread();
	~CLThread();

public:
	static DWORD	GetMainThreadId();									//获取当前进程的主线程ID,失败返回0
	static DWORD	SetThreadId(DWORD dwThreadId);						//修改当前线程的线程ID,返回旧线程的ID,失败返回0
	static BOOL		QueryThreadInfo_By_Pid(DWORD dwPid, std::vector<CL_PROCESS_THREADINFO> & vlst);
	static BOOL		TryTriminateThread(HANDLE& hThread, DWORD dwTimeOut = 3000);
public:
};



#endif