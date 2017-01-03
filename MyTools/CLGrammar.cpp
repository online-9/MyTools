#include "stdafx.h"
#include "CLGrammar.h"
#include <algorithm>
#include <iostream>
#include "CLLog.h"
#include "Character.h"
#define _SELF L"CLGrammar.cpp"

#define CopyErrMsg(x)								\
	if (pwszErrMsg)									\
	{												\
		CCharacter::wstrcpy_my(pwszErrMsg, x);		\
	}

CLGrammar::CLGrammar()
{
}

CLGrammar::~CLGrammar()
{
}

BOOL CLGrammar::AnalysisGrammar(__in LPCWSTR pwszText, __out LPWSTR pwszErrMsg)
{
	/////AnalysisGrammar/////////////////////////////////////////////////////////////////////
	if (wcslen(pwszText) >= 1024 / 2)
	{
		CopyErrMsg(L"Command was too long, len must < 512\r\n");
		return FALSE;
	}

	// Copy Text
	static WCHAR wszText[1024];
	CCharacter::wstrcpy_my(wszText, pwszText, _countof(wszText) - 1);
	CCharacter::Trim_W(wszText);

	auto fnGetNextCmd = [](LPWSTR pwszCmd)
	{
		if (CCharacter::wstrstr_my(wszText, L"-"))
		{
			CCharacter::GetRemoveLeft(wszText, L"-", pwszCmd);
			CCharacter::GetRemoveRight(wszText, L"-", wszText);
		}
		else
		{
			CCharacter::wstrcpy_my(pwszCmd, wszText);
			wszText[0] = '\0';
		}

		CCharacter::Trim_W(pwszCmd);
	};

	GetGrammarList().clear();
	while (wszText[0] != '\0')
	{
		GrammarContext Gc;

		fnGetNextCmd(Gc.szCmd);
		GetGrammarList().push_back(Gc);
	}

	if (GetGrammarList().size() == 0)
	{
		CopyErrMsg(L"Can't find function name");
		return FALSE;
	}

	LPVOID lpFnAddr = GetFunAddr(GetGrammarList().begin()->szCmd);
	if (lpFnAddr == NULL)
	{
		CopyErrMsg(L"Err function name\r\n");
		return FALSE;
	}

	GrammarFun pfn = (GrammarFun)lpFnAddr;
	pfn(GetGrammarList(), pwszErrMsg);

	return TRUE;
}

std::vector<GrammarTran>& CLGrammar::GetTranList()
{
	static std::vector<GrammarTran> vlst;
	return vlst;
}

VOID CLGrammar::AddTranList(__in LPCWSTR pwszCmd, __in GrammarFun lpfun)
{
	std::vector<GrammarTran>& vlst = GetTranList();

	auto fnExistCmd = [&vlst](LPCWSTR pwszText)
	{
		return std::find_if(vlst.begin(), vlst.end(), [&pwszText](GrammarTran& GT){ return CCharacter::wstrcmp_my(pwszText, GT.wszCmd); }) != vlst.end() ? TRUE : FALSE;
	};

	if (fnExistCmd(pwszCmd))
		return;

	static GrammarTran GT;
	CCharacter::wstrcpy_my(GT.wszCmd, pwszCmd);
	GT.pFunAddr = lpfun;
	vlst.push_back(GT);
}

LPVOID CLGrammar::GetFunAddr(__in LPCWSTR pwszCmd)
{
	std::vector<GrammarTran>& vlst = GetTranList();

	auto itr = std::find_if(vlst.begin(), vlst.end(), [&pwszCmd](GrammarTran& Gt){ return CCharacter::wstrcmp_my(pwszCmd, Gt.wszCmd); });
	return itr == vlst.end() ? NULL : itr->pFunAddr;
}

std::vector<GrammarContext>& CLGrammar::GetGrammarList()
{
	static std::vector<GrammarContext> vlst;
	return vlst;
}

DWORD CLGrammar::Query_DWORDParm_By_GrammarList(__in UINT uIndex, __in std::vector<GrammarContext>& vlst)
{
	if (uIndex >= vlst.size())
		return FALSE;

	return wcstol(vlst.at(uIndex).szCmd, NULL, 16);
}

float CLGrammar::Query_FLOATParm_By_GrammarList(__in UINT uIndex, __in std::vector<GrammarContext>& vlst)
{
	if (uIndex >= vlst.size())
		return FALSE;

	return (float)_wtof(vlst.at(uIndex).szCmd);
}

LPCWSTR CLGrammar::Query_LPWSTRParm_By_GrammarList(__in UINT uIndex, __in std::vector<GrammarContext>& vlst)
{
	if (uIndex >= vlst.size())
		return nullptr;
	
	return vlst.at(uIndex).szCmd;
}

BOOL CLGrammar::Check_ParmCount_By_GrammarList(__in UINT uCount, __in std::vector<GrammarContext>& vlst)
{
	return vlst.size() - 1 >= uCount;
}
