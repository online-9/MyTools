#ifndef __MYTOOLS_DEBUG_LOG_CMDLOGSERVER_H__
#define __MYTOOLS_DEBUG_LOG_CMDLOGSERVER_H__

#include "CmdLog.h"

class CCmdLogServer : public CClassInstance<CCmdLogServer>
{
private:
	struct CmdLogClient
	{
		std::wstring wsClientName;
		SOCKET skClient;
	};
public:
	CCmdLogServer();
	~CCmdLogServer();

	BOOL Run();
	VOID Stop();

	VOID SendContent(_In_ CONST std::wstring& wsClientName, _In_ CONST std::wstring& wsContent);

private:
	static DWORD WINAPI _AcceptThread(LPVOID lpParm);

	UINT GetClientSocket_By_ClientName(_In_ CONST std::wstring& wsClientName, _Out_ std::vector<SOCKET>& VecClientSocket) CONST;
private:
	SOCKET _servSocket;
	std::vector<CmdLogClient> _VecClient;
	CLLock _LockCmdLogClient;
	HANDLE _hAcceptThread;
	BOOL _Run;
	CONST int _nSendTimeout;
	CONST int _nRecvTimeout;
};


#endif // !__MYTOOLS_DEBUG_LOG_CMDLOGSERVER_H__
