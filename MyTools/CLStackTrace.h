#ifndef __MYTOOLS_STACKTRACE_STACKTRACE_H__
#define __MYTOOLS_STACKTRACE_STACKTRACE_H__

#include "ClassInstance.h"
#include <stack>
#include <mutex>

#define _CLStackTrace(FunName)  //CLStackTracer CLStackTracer_(FunName, _SELF, __LINE__);

class CLStackGroup : public CClassInstance<CLStackGroup>
{
public:
	struct FunctionStackContent
	{
		std::wstring wsFunName;
		std::wstring wsFileName;
		int			 nLine;
	};
	struct ThreadStackContent : public enable_shared_from_this<ThreadStackContent>
	{
		std::stack<FunctionStackContent>	StackList;
		std::wstring						wsStackTracerName;
		std::mutex							ThreadMutex;
	};
public:
	CLStackGroup() = default;
	~CLStackGroup() = default;

public:

	VOID InstallStackTracer(_In_ CONST std::wstring& wsStackTracerName);
	
	VOID UnInstallStackTracer();

	VOID Push(_In_ DWORD dwThreadId, _In_ CONST std::wstring& wsFunName, _In_ CONST std::wstring& wsFileName, _In_ int nLine);

	VOID Pop(_In_ DWORD dwThreadId);

	UINT CopyThreadStackContent(_In_ DWORD dwThreadId, _Out_ std::stack<FunctionStackContent>& FunStaackContentStack);
private:
	std::map<DWORD, std::shared_ptr<ThreadStackContent>> ThreadStackMap;
};

class CLStackTracer
{
public:
	CLStackTracer(_In_ CONST std::wstring& wsFunName, _In_ CONST std::wstring& wsFileName, _In_ int nLine);
	~CLStackTracer();

private:

};





#endif