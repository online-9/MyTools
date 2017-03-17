#include "stdafx.h"
#include "CLStackTrace.h"
#include "CLFile.h"
#include "CLLock.h"
#include <deque>
#include "CLLog.h"

#define _SELF L"CLStackTrace.cpp"


VOID CLStackGroup::InstallStackTracer(_In_ CONST std::wstring& wsStackTracerName)
{
	DWORD dwThreadId = ::GetCurrentThreadId();
	if (ThreadStackMap.find(dwThreadId) == ThreadStackMap.end())
	{
		auto ThreadStackContent_ = std::make_shared<ThreadStackContent>();
		ThreadStackContent_->wsStackTracerName = wsStackTracerName;
		ThreadStackMap.insert(std::map<DWORD, std::shared_ptr<ThreadStackContent>>::value_type(dwThreadId, ThreadStackContent_));
	}
}

VOID CLStackGroup::UnInstallStackTracer()
{
	auto itr = ThreadStackMap.find(::GetCurrentThreadId());
	if (itr != ThreadStackMap.end())
		ThreadStackMap.erase(itr);
}

VOID CLStackGroup::Push(_In_ DWORD dwThreadId, _In_ CONST std::wstring& wsFunName, _In_ CONST std::wstring& wsFileName, _In_ int nLine)
{
	auto itm = ThreadStackMap.find(dwThreadId);
	if (itm == ThreadStackMap.end())
		return;

	std::lock_guard<std::mutex> lck(itm->second->ThreadMutex);
	itm->second->StackList.push(FunctionStackContent{ wsFunName, wsFileName, nLine });
}

VOID CLStackGroup::Pop(_In_ DWORD dwThreadId)
{
	auto itm = ThreadStackMap.find(dwThreadId);
	if (itm == ThreadStackMap.end())
		return;

	std::lock_guard<std::mutex> lck(itm->second->ThreadMutex);
	itm->second->StackList.pop();
}

UINT CLStackGroup::CopyThreadStackContent(_In_ DWORD dwThreadId, _Out_ std::stack<FunctionStackContent>& FunStaackContentStack)
{
	auto itr = ThreadStackMap.find(dwThreadId);
	if (itr == ThreadStackMap.end())
		return 0;

	auto& itm = itr->second;
	std::lock_guard<std::mutex> lck(itm->ThreadMutex);
	FunStaackContentStack = itm->StackList;
	return FunStaackContentStack.size();
}

CLStackTracer::CLStackTracer(_In_ CONST std::wstring& wsFunName, _In_ CONST std::wstring& wsFileName, _In_ int nLine)
{
	CLStackGroup::GetInstance().Push(::GetCurrentThreadId(), wsFunName, wsFileName, nLine);
}

CLStackTracer::~CLStackTracer()
{
	CLStackGroup::GetInstance().Pop(::GetCurrentThreadId());
}
