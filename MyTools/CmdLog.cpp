#include "stdafx.h"
#include "CmdLog.h"
#include <thread>
#include <WS2tcpip.h>
#include "Log.h"
#include "CLExpressionCalc.h"
#include "Character.h"
#pragma  comment (lib,"wsock32.lib") 

#define _SELF L"CmdLog.cpp"
CCmdLog::CCmdLog() : _Run(FALSE), _skClient(INVALID_SOCKET), _hRecvThread(NULL)
{
	
}

CCmdLog::~CCmdLog()
{

}

BOOL CCmdLog::Run(_In_ CONST std::wstring& wsClientName, _In_ CONST std::vector<ExpressionFunPtr>& ParmVecFunPtr)
{
	WSAData wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	_Run = TRUE;
	_wsClientName = wsClientName;
	_CLExpression.SetVecExprFunPtr(ParmVecFunPtr);
	_hRecvThread = cbBEGINTHREADEX(NULL, NULL, _RecvThread, this, NULL, NULL);
	return TRUE;
}

VOID CCmdLog::Stop()
{
	_Run = FALSE; 
	BreakLogConnect();
	if (::WaitForSingleObject(_hRecvThread, 3 * 1000) == WAIT_TIMEOUT)
		::TerminateThread(_hRecvThread, 0);

	::CloseHandle(_hRecvThread);
	_hRecvThread = NULL;
}

DWORD WINAPI CCmdLog::_RecvThread(LPVOID lpParm)
{
	CCmdLog* pCCmdLog = (CCmdLog *)lpParm;

	while (pCCmdLog->_Run)
	{
		if (!pCCmdLog->ConnectLogServer())
		{
			::Sleep(100);
			continue;
		}

		if (!pCCmdLog->SendClientName())
		{
			pCCmdLog->BreakLogConnect();
			continue;
		}

		while (pCCmdLog->_Run)
		{
			std::wstring wsCmdText;
			if(!pCCmdLog->Recv(wsCmdText))
				break;


			pCCmdLog->ExcuteLogServerCmd(wsCmdText);
		}

	}
	return 0;
}

VOID CCmdLog::ExcuteLogServerCmd(_In_ CONST std::wstring& wsCmdText)
{
	WCHAR* pwszCmdText = new WCHAR[1024];
	CCharacter::wstrcpy_my(pwszCmdText, wsCmdText.c_str());

	std::thread t([pwszCmdText, this]
	{
		CONST std::wstring wsCmdText = pwszCmdText;
		LOG_C(CLog::em_Log_Type::em_Log_Type_Custome, L"Client:%s Excute Cmd Text:%s", _wsClientName.c_str(), wsCmdText.c_str());
		CLExpressionCalc ExpAnalysis;

		if (ExpAnalysis.IsConformToCmdType(wsCmdText))
			ExpAnalysis.Analysis(wsCmdText);
		else
			_CLExpression.Run(wsCmdText);

		delete[] pwszCmdText;
	});
	t.detach();
}

BOOL CCmdLog::ConnectLogServer()
{
	if (_skClient == INVALID_SOCKET)
	{
		_skClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_skClient == INVALID_SOCKET)
			return FALSE;

		CONST static int nSendTimeout = 3 * 1000;
		setsockopt(_skClient, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<CONST CHAR*>(&nSendTimeout), sizeof(int));
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(em_CmdLog_Port);

	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	return connect(_skClient, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR ? FALSE : TRUE;
}

VOID CCmdLog::BreakLogConnect()
{
	::shutdown(_skClient, SD_BOTH);
	::closesocket(_skClient);
	_skClient = INVALID_SOCKET;
}

BOOL CCmdLog::SendClientName()
{
	SendText SendText_ = { 0 };
	SendText_.dwParm1 = 0x434C436C;
	SendText_.dwParm2 = 0x69656E74;
	SendText_.dwMsg = static_cast<DWORD>(em_CmdLog_Msg_SendClientInfo);
	CCharacter::wstrcpy_my(SendText_.wszText, _wsClientName.c_str());


	return Send(reinterpret_cast<CONST CHAR*>(&SendText_), sizeof(SendText_));
}

int CCmdLog::Send(_In_ CONST CHAR* Buffer, _In_ INT nLen)
{
	return ::send(_skClient, reinterpret_cast<CONST CHAR*>(Buffer), nLen, 0);
}

BOOL CCmdLog::Recv(_Out_ std::wstring& wsCmdText)
{
	RecvText RecvText_ = { 0 };
	int nRetCode = ::recv(_skClient, reinterpret_cast<CHAR*>(&RecvText_), sizeof(RecvText_), 0);
	if (nRetCode == -1 || nRetCode != sizeof(RecvText_))
	{
		BreakLogConnect();
		return FALSE;
	}
	else if (RecvText_.dwParm1 != 0x434C536F || RecvText_.dwParm2 != 0x69656E74)
	{
		BreakLogConnect();
		return FALSE;
	}

	wsCmdText = RecvText_.wszText;
	return TRUE;
}
