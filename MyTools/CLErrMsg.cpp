#include "stdafx.h"
#include "CLErrMsg.h"
#include <algorithm>

vector<CLErrMsg::ErrMsg> CLErrMsg::vlst;
CLErrMsg::CLErrMsg(_In_ LPCWSTR pwszFunName, int nLine, LPCWSTR pwszFormat, ...)
{
	va_list		args;
	wchar_t		szBuff[1024] = { 0 };
	wchar_t		szBuffer[1024] = { 0 };

	va_start(args, pwszFormat);
	_vsnwprintf_s(szBuff, _countof(szBuff) - 1, _TRUNCATE, pwszFormat, args);
	va_end(args);

	swprintf_s(szBuff,_countof(szBuff) - 1, L"%s %d:%s", szBuffer, nLine, pwszFunName);

	ErrMsg ErrMsg_;
	ErrMsg_.dwThreadId = ::GetCurrentThreadId();
	ErrMsg_.wsMsg = szBuff;
	vlst.push_back(ErrMsg_);
}

CLErrMsg::~CLErrMsg()
{
}

BOOL CLErrMsg::ExistErrMsg()
{
	DWORD dwThreadId = ::GetCurrentThreadId();
	return find_if(vlst.begin(), vlst.end(), [&dwThreadId](CONST ErrMsg& ErrMsg_){
		return dwThreadId == ErrMsg_.dwThreadId;
	}) != vlst.end();
}

BOOL CLErrMsg::GetCurThreadErrMsg(_Out_ vector<wstring>& vErrMsg)
{
	DWORD dwThreadId = ::GetCurrentThreadId();
	for (CONST auto& itm : vlst)
	{
		if (itm.dwThreadId == dwThreadId)
			vErrMsg.push_back(itm.wsMsg);
	}
	return vErrMsg.size() != NULL;
}