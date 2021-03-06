#include "stdafx.h"
#include "CmdLogServer.h"
#include "Character.h"
#include "CLPublic.h"
#include <thread>

#define _SELF L"CmdLogServer.cpp"
CCmdLogServer::CCmdLogServer() : _LockCmdLogClient(L"LockCmdLogClient"), _Run(FALSE), _hAcceptThread(NULL), _servSocket(INVALID_SOCKET), _nSendTimeout(3 * 1000), _nRecvTimeout(3 * 1000)
{

}

CCmdLogServer::~CCmdLogServer()
{

}

BOOL CCmdLogServer::Run()
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	_servSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_servSocket == INVALID_SOCKET)
		return FALSE;

	setsockopt(_servSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<CONST CHAR*>(&_nSendTimeout), sizeof(int));
	setsockopt(_servSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<CONST CHAR*>(&_nRecvTimeout), sizeof(int));

	// set sock variable
	sockaddr_in addr;
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(CCmdLog::em_CmdLog_Port);

	// bind to port
	if (::bind(_servSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		return FALSE;

	if (listen(_servSocket, CCmdLog::em_CmdLog_MaxClientCount) == SOCKET_ERROR)
		return FALSE;

	_Run = TRUE;
	_hAcceptThread = cbBEGINTHREADEX(NULL, NULL, _AcceptThread, this, NULL, NULL);
	return TRUE;
}

VOID CCmdLogServer::Stop()
{
	_Run = FALSE;

	::shutdown(_servSocket, SD_BOTH);
	::closesocket(_servSocket);
	_servSocket = INVALID_SOCKET;

	if(_hAcceptThread != NULL)
		::WaitForSingleObject(_hAcceptThread, INFINITE);
}

VOID CCmdLogServer::SendContent(_In_ CONST std::wstring& wsClientName, _In_ CONST std::wstring& wsContent)
{
	std::vector<SOCKET> VecClientSocket;
	GetClientSocket_By_ClientName(CCharacter::MakeTextToUpper(wsClientName) == L"ALL" ? L"" : wsClientName, VecClientSocket);

	for (CONST auto& itm : VecClientSocket)
	{
		CCmdLog::RecvText RecvText_;
		RecvText_.dwParm1 = 0x434C536F;
		RecvText_.dwParm2 = 0x69656E74;
		CCharacter::wstrcpy_my(RecvText_.wszText, wsContent.c_str());

		if (::send(itm, reinterpret_cast<CONST CHAR*>(&RecvText_), sizeof(RecvText_), 0) == SOCKET_ERROR)
		{
			RemoveSocket(itm);
			::shutdown(itm, SD_BOTH);
			::closesocket(itm);
		}
	}
}

DWORD WINAPI CCmdLogServer::_AcceptThread(LPVOID lpParm)
{
	CCmdLogServer* pCCmdLogServer = (CCmdLogServer *)lpParm;

	while (pCCmdLogServer->_Run)
	{
		sockaddr_in addr;
		int len = sizeof(addr);
		SOCKET s = ::accept(pCCmdLogServer->_servSocket, (sockaddr*)&addr, &len);
		if (s == SOCKET_ERROR || s == INVALID_SOCKET) // un exist client to server
		{
			Sleep(100);
			continue;
		}

		setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<CONST CHAR*>(&pCCmdLogServer->_nSendTimeout), sizeof(int));
		setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<CONST CHAR*>(&pCCmdLogServer->_nRecvTimeout), sizeof(int));

		SOCKET* pClientSocket = new SOCKET(s);
		std::thread t([pClientSocket, pCCmdLogServer]
		{
			// how to stop? 
			CCmdLog::SendText SendText_;
			int nRetCode = ::recv(*pClientSocket, reinterpret_cast<CHAR*>(&SendText_), sizeof(SendText_), 0);
			if (nRetCode == SOCKET_ERROR || nRetCode != sizeof(SendText_) || SendText_.dwParm1 != 0x434C436C || SendText_.dwParm2 != 0x69656E74)
			{
				::shutdown(*pClientSocket, SD_BOTH);
				::closesocket(*pClientSocket);
				delete pClientSocket;
				return;
			}

			CmdLogClient CmdLogClient_;
			CmdLogClient_.skClient = *pClientSocket;
			CmdLogClient_.wsClientName = SendText_.wszText;
			pCCmdLogServer->_LockCmdLogClient.Access([CmdLogClient_, pCCmdLogServer] { pCCmdLogServer->_VecClient.push_back(CmdLogClient_);  });
			delete pClientSocket;
		});
		t.detach();
	}
	return 0;
}

UINT CCmdLogServer::GetClientSocket_By_ClientName(_In_ CONST std::wstring& wsClientName, _Out_ std::vector<SOCKET>& VecClientSocket) CONST
{
	_LockCmdLogClient.Access([wsClientName, this, &VecClientSocket]
	{
		for (CONST auto& itm : _VecClient)
		{
			if (wsClientName.empty() || CCharacter::MakeTextToUpper(itm.wsClientName) == CCharacter::MakeTextToUpper(wsClientName))
			{
				VecClientSocket.push_back(itm.skClient);
			}
		}
	});
	return VecClientSocket.size();
}

VOID CCmdLogServer::RemoveSocket(_In_ SOCKET skClient)
{
	_LockCmdLogClient.Access([this, skClient]
	{
		CLPublic::Vec_erase_if(_VecClient, [skClient](CONST CmdLogClient& CmdLogClient_){ return CmdLogClient_.skClient == skClient; });
	});
}
