#ifndef __MYTOOLS_LOG_CLLOG_H__
#define __MYTOOLS_LOG_CLLOG_H__

#include "ToolsPublic.h"
#include <deque>

#define CLLOG_MUTEX	L"CLLOG_MUTEX"
#define CLLOG_READY_EVENT L"CLLOG_READY_EVENT"
#define CLLOG_BUFFER_EVENT L"CLLOG_BUFFER_EVENT"
#define CLLOG_SHAREMEM L"CLLOG_SHAREMEM"

#define Log(MsgLevel,pwszFormat,...) CLLog::Print(__FUNCTIONW__, _SELF,__LINE__,MsgLevel,pwszFormat,__VA_ARGS__ )
#define LogMsgBox(MsgLevel,pwszFormat,...) CLLog::MsgBox(__FUNCTIONW__, _SELF,__LINE__,MsgLevel,pwszFormat,__VA_ARGS__ )

#define LOG_LEVEL_EXCEPTION							L"异常"
#define LOG_LEVEL_NORMAL							L"普通"
#define LOG_LEVEL_FUNCTION							L"函数"
#define LOG_LEVEL_PARM								L"参数"

class CLLog
{
public:
#pragma pack(push)
#pragma pack(4)
	struct _LogContent
	{
		// 日志等级
		WCHAR wszLogLevel[32];
		// 函数名
		WCHAR wszFunName[64];
		// 文件名称+行数
		WCHAR wszFileName[64];
		// 昵称
		WCHAR wszClientName[32];
		// 内容
		UINT  uRepeatCount;
		WCHAR wszLogContent[1024];
		// 时间
		SYSTEMTIME SysTime;
		// 堆栈追踪
		WCHAR wszStack[10][32];
		UINT  uStackCount;
	};
#pragma pack(pop)
public:
	CLLog() = delete;
	~CLLog() = delete;

	static BOOL Print(_In_ LPCWSTR pwszFunName,_In_ LPCWSTR pwszFileName, _In_ int nLine, _In_ LPCWSTR pwszMsgLevel, _In_ LPCWSTR pwszFormat,...);
	static BOOL MsgBox(_In_ LPCWSTR pwszFunName, _In_ LPCWSTR pwszFileName, _In_ int nLine, _In_ LPCWSTR pwszMsgLevel, _In_ LPCWSTR pwszFormat, ...);

	static BOOL SetClientName(_In_ CONST std::wstring& wsClientName);

	static BOOL PrintExceptionCode(_In_ LPEXCEPTION_POINTERS ExceptionPtr);
private:
	static BOOL PrintLogTo(_In_ _LogContent& LogContent);

	static BOOL SaveLog(_In_ _LogContent& LogContent);
private:
	static std::wstring wsProjectName;
	static std::wstring wsLocalLogPath;
};



#endif