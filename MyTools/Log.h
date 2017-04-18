#ifndef _LOG_H__
#define _LOG_H__

#include <memory>
#include <queue>
#include "ClassInstance.h"
#include "CLLock.h"

#define LOG_TYPE_CONSOLE 0x2
#define LOG_TYPE_FILE	 0x4

#define LOG_C(Type,FormatText,...)	CLog::GetInstance().Print(__FUNCTIONW__, _SELF, __LINE__, LOG_TYPE_CONSOLE, Type, FALSE, FormatText, __VA_ARGS__)
#define LOG_F(Type,FormatText,...)	CLog::GetInstance().Print(__FUNCTIONW__, _SELF, __LINE__, LOG_TYPE_FILE, Type, FALSE, FormatText, __VA_ARGS__)
#define LOG_CF(Type,FormatText,...) CLog::GetInstance().Print(__FUNCTIONW__, _SELF, __LINE__, LOG_TYPE_CONSOLE | LOG_TYPE_FILE, Type, FALSE, FormatText, __VA_ARGS__)
#define LOG_FC(Type,FormatText,...) LOG_CF(Type, FormatText, __VA_ARGS__)
#define LOG_MSG_CF(Type,FormatText,...) CLog::GetInstance().Print(__FUNCTIONW__, _SELF, __LINE__, LOG_TYPE_CONSOLE, Type, TRUE, FormatText, __VA_ARGS__)
#define LOG_CF_D(FormatText,...)	LOG_CF(CLog::em_Log_Type::em_Log_Type_Debug,FormatText, __VA_ARGS__)
#define LOG_CF_E(FormatText,...)	LOG_CF(CLog::em_Log_Type::em_Log_Type_Exception,FormatText, __VA_ARGS__)
#define LOG_C_D(FormatText,...)		LOG_C(CLog::em_Log_Type::em_Log_Type_Debug,FormatText, __VA_ARGS__)

class CLog : public virtual CClassInstance<CLog>
{
public:
	enum em_Log_Type
	{
		em_Log_Type_Invalid,
		em_Log_Type_Debug,
		em_Log_Type_Warning,
		em_Log_Type_Custome,
		em_Log_Type_Exception,
	};
private:
#define CL_LOG_MUTEX	L"CL_LOG_MUTEX"
#define CL_LOG_READY_EVENT L"CL_LOG_READY_EVENT"
#define CL_LOG_BUFFER_EVENT L"CL_LOG_BUFFER_EVENT"
#define CL_LOG_READMEMORY_EVENT L"CL_LOG_READMEMORY_EVENT"
#define CL_LOG_SHAREMEM L"CL_LOG_SHAREMEM"

#define CL_LOG_CMD_MUTEX	L"CL_LOG_CMD_MUTEX"
#define CL_LOG_CMD_READY_EVENT L"CL_LOG_CMD_READY_EVENT"
#define CL_LOG_CMD_BUFFER_EVENT L"CL_LOG_CMD_BUFFER_EVENT"
#define CL_LOG_CMD_SHAREMEM L"CL_LOG_CMD_SHAREMEM"

#pragma pack(push)
#pragma pack(4)

	struct CmdLogContent
	{
		WCHAR wszClientName[32];
		WCHAR wszCmd[1024];
		DWORD dwHash;
	};

	struct LogContent
	{
		em_Log_Type emLogType;

		// FunctionName
		WCHAR wszFunName[32];

		// FileName
		WCHAR wszFileName[32];

		// Log Line in Code
		UINT uLine;

		// Client Name
		WCHAR wszClientName[32];

		// ÄÚÈÝ
		UINT  uRepeatCount;
		WCHAR wszLogContent[1024];

		// ¶ÑÕ»×·×Ù
		WCHAR wszStack[10][32];
		UINT  uStackCount;
	};
#pragma pack(pop)
public:
	CLog();
	~CLog();

	CLog& operator = (CONST CLog&) = delete;
	CLog(CONST CLog&) = delete;
public:
	VOID Print(_In_ LPCWSTR pwszFunName, _In_ LPCWSTR pwszFileName, _In_ int nLine, _In_ int nLogOutputType, _In_ em_Log_Type emLogType, _In_ BOOL bMsgBox, _In_ LPCWSTR pwszFormat, ...);

	VOID MsgBox(_In_ LPCWSTR pwszFunName, _In_ LPCWSTR pwszFileName, _In_ int nLine, _In_ int nLogOutputType, _In_ em_Log_Type emLogType, _In_ LPCWSTR pwszFormat, ...);

	// SaveLogPath have to like 'C:\\'
	VOID SetClientName(_In_ CONST std::wstring& cwsClientName, _In_ CONST std::wstring wsSaveLogPath, _In_ BOOL bOverWrite, _In_ ULONG ulMaxSize);

	// Mutiple Thread UnSafe, be care use!
	VOID SetClientName(_In_ CONST std::wstring& cwsClientName);

	VOID Release();

private:
	BOOL PrintTo(_In_ CONST LogContent& LogContent_);

	BOOL SaveLog(_In_ CONST LogContent& LogContent_) CONST;

	static DWORD WINAPI _SendThread(LPVOID lpParm);
	static DWORD WINAPI _SaveThread(LPVOID lpParm);

	VOID AddLogContentToQueue(_In_ CONST LogContent& LogContent_);

	BOOL GetLogContentForQueue(_Out_ LogContent& LogContent_);

	VOID AddSaveLogToQueue(_In_ CONST LogContent& LogContent_);

	BOOL GetSaveLogContentForQueue(_Out_ LogContent& LogContent_);
private:
	std::queue<LogContent> QueueLogContent;
	std::queue<LogContent> QueueSaveLogContent;

	SYSTEMTIME CurrentSysTime;
	mutable std::wstring wsLogFilePath;
	std::wstring wsClientName;
	HANDLE hSaveLogEvent;
	HANDLE hSendExitEvent;
	BOOL bRun;
	BOOL m_bOverWrite;
	ULONGLONG _ulMaxFileSize;
	CLLock Lock_LogContentQueue;
	CLLock Lock_SaveLogContentQueue;
};

#endif