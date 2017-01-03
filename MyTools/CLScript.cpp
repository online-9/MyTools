#include "stdafx.h"
#include "CLScript.h"
#include <algorithm>
#include <memory>
#include <array>
#include "CLFile.h"
#include "Character.h"
#include "CLLog.h"
#include "CLStackTrace.h"
#include "CLPublic.h"
#define _SELF L"CLScript.cpp"

CLScript::CLScript()
{
	bExit = FALSE;
	lpCustomeFunParm = this;
	bShowDebugMsg = FALSE;
}

CLScript::~CLScript()
{
	//Log(LOG_LEVEL_NORMAL,L"CLScript::~CLScript");
	Release();
}

BOOL CLScript::ReadScriptFile(_In_ cwstring& wsPath)
{
	try
	{
		std::wstring wsContent;
		if (!CLFile::ReadUnicodeFile(wsPath, wsContent))
		{
			Log(LOG_LEVEL_EXCEPTION, L"ReadScriptFile Fiald! Path:%s", wsPath.c_str());
			return FALSE;
		}

		CCharacter::Split(wsContent, L"\r\n", SourceCodeList, Split_Option_RemoveEmptyEntries);

		CLPublic::Vec_erase_if(SourceCodeList, [](CONST std::wstring& wsText){ return wsText.find(L"//") != -1; });
		return SourceCodeList.size() != 0;
	}
	catch(...)
	{
		Log(LOG_LEVEL_EXCEPTION,L"ReadScriptFile出现异常");
	}
	return FALSE;
}

BOOL CLScript::AnalysisSourceCode()
{
	for (auto& SourceCode : SourceCodeList)
	{
		em_CL_Script_Code_Type emCodeType = GetSourceType(SourceCode);
		if (emCodeType == em_CL_Script_Code_Type_None || em_CL_Script_Code_Type_Note == emCodeType)
			continue;

		CL_Script_TranCode		TranCode;
		TranCode.emCodeType = emCodeType;

		BOOL bRetCode = FALSE;
		switch (emCodeType)
		{
		case em_CL_Script_Code_Type_DefineFun:
			bRetCode = AnalysisCode_DefineFun(SourceCode, TranCode.pStruct);
			break;
		case em_CL_Script_Code_Type_Function:
			bRetCode = AnalysisCode_ExcuteFun(SourceCode, TranCode.pStruct);
			break;
		case em_CL_Script_Code_Type_If:
			//bRetCode = AnalsisCode_If(SourceCode.wszCode, TranCode.pStruct);
			break;
		case em_CL_Script_Code_Type_ElseIf:
			//bRetCode = AnalsisCode_If(SourceCode.wszCode, TranCode.pStruct);
			break;
		case em_CL_Script_Code_Type_While:
			//bRetCode = AnalsisCode_If(SourceCode.wszCode, TranCode.pStruct);
			break;
		default:
			bRetCode = TRUE;
			TranCode.pStruct = NULL;
			break;
		}

		if (!bRetCode)
			return FALSE;
		
		ScriptCodeList.push_back(TranCode);
	}

	SourceCodeList.clear();
	return TRUE;
}

em_CL_Script_Code_Type CLScript::GetSourceType(_In_ cwstring wsText)
{
	if (wsText.find(L"function ") != -1)
		return em_CL_Script_Code_Type_DefineFun;

	if (wsText.find(L"{") != -1)
		return em_CL_Script_Code_Type_Left_Brace;

	if (wsText.find(L"}") != -1)
		return em_CL_Script_Code_Type_Right_Brace;

	if (wsText.find(L")") != -1)
		return em_CL_Script_Code_Type_Function;

	if (wsText.find(L"//") != -1)
		return em_CL_Script_Code_Type_Note;

	/*if (CCharacter::wstrstr_my(pwszText, L"if(") || CCharacter::wstrstr_my(pwszText, L"if ("))
		return em_CL_Script_Code_Type_If;

	if (CCharacter::wstrstr_my(pwszText, L"else if(") || CCharacter::wstrstr_my(pwszText,L"else if ("))
		return em_CL_Script_Code_Type_ElseIf;

	if (CCharacter::wstrstr_my(pwszText, L"else"))
		return em_CL_Script_Code_Type_Else;

	if (CCharacter::wstrstr_my(pwszText, L"while(") || CCharacter::wstrstr_my(pwszText,L"while ("))
		return em_CL_Script_Code_Type_While;
	
	if (CCharacter::wstrstr_my(pwszText, L"continue;"))
		return em_CL_Script_Code_Type_Contiune;

	if (CCharacter::wstrstr_my(pwszText, L"break;"))
		return em_CL_Script_Code_Type_Break;*/

	return em_CL_Script_Code_Type_None;
}

BOOL CLScript::AnalysisCode_DefineFun(_In_ cwstring& wsText, __out LPVOID& pAddr)
{
	std::wstring wsTmpText;

	CL_Script_TranCode_DefineFun* pTranCode = new CL_Script_TranCode_DefineFun;
	if (pTranCode == NULL)
		return FALSE;

	CCharacter::GetRemoveRight(wsText, L"function", wsTmpText); // Task1()
	CCharacter::GetRemoveLeft(CCharacter::Trim_W(wsTmpText), L"(", pTranCode->wsFunName);

	pAddr = pTranCode;
	return TRUE;
}

BOOL CLScript::AnalysisCode_ExcuteFun(_In_ cwstring& wsText, __out LPVOID& pAddr)
{
	std::wstring wsTmpText;

	CL_Script_TranCode_Function* pTranCode = new CL_Script_TranCode_Function;
	if (pTranCode == NULL)
		return FALSE;

	// Get Fun Name
	CCharacter::GetRemoveLeft(wsText, L"(", pTranCode->wsFunName);
	CCharacter::Trim_W(pTranCode->wsFunName);

	// Remove Fun Name && ( && )
	CCharacter::GetRemoveRight(wsText, L"(", wsTmpText);
	CCharacter::GetRemoveLeft(wsTmpText, L")", wsTmpText);
	CCharacter::Trim_W(wsTmpText);

	// Set Fun Parm
	std::vector<std::wstring> vlst;
	std::vector<std::wstring> vParm;

	// Task,[13E,2],[2,0] -> Task,,
	CCharacter::GetSplit_By_List(wsTmpText, { L"[", L"]" }, vlst, Split_Option_None);

	// Task,, -> "Task","",""
	if (wsTmpText.find(L",") != -1)
		CCharacter::Split(wsTmpText, L",", vParm, Split_Option_None);
	else if (CCharacter::Trim_W(wsTmpText) != L"")
		vParm.push_back(wsTmpText);
	else if (vlst.size() == 1 && vParm.size() == 0)
		vParm.push_back(L"");

	for (auto& itr = vParm.begin(); itr != vParm.end() && vlst.size() != 0; ++itr)
	{
		if (*itr == L"")
		{
			*itr = *vlst.begin();
			vlst.erase(vlst.begin());
		}
	}

	std::for_each(vParm.begin(), vParm.end(), [&pTranCode](std::wstring& wsParm){
		pTranCode->ParmList.push_back(_CL_Script_TranCode_FunParm(CCharacter::Trim_W(wsParm)));
	});

	pAddr = pTranCode;
	return TRUE;
}

BOOL CLScript::ExcuteFunction(_In_ cwstring& wsName, _In_ std::vector<CL_Script_TranCode_FunParm>& vlst)
{
	for (auto itr = ScriptCodeList.begin(); itr != ScriptCodeList.end() && !bExit; ++itr)
	{
		if (itr->emCodeType != em_CL_Script_Code_Type_DefineFun)
			continue;

		if (((CL_Script_TranCode_DefineFun *)itr->pStruct)->wsFunName != wsName)
			continue;

		Log(LOG_LEVEL_NORMAL, L"执行脚本函数:%s", wsName.c_str());

		auto EndItr = GetItr_By_CodeType(itr, em_CL_Script_Code_Type_Right_Brace);
		if (EndItr == ScriptCodeList.end())
		{
#if Script_MsgBox_Exception
			LogMsgBox(LOG_LEVEL_EXCEPTION, L"Can't Find '}' in %s", wsName.c_str());
#endif
			bExit = TRUE;
			return FALSE;
		}
		return ExcuteContent(itr, EndItr);
	}

	CL_Script_CustomeFunction CL_Script_CustomeFunction_;
	if (!IsExistCustomeFunAddrList(wsName, CL_Script_CustomeFunction_))
	{
		LogMsgBox(LOG_LEVEL_EXCEPTION, L"Can't Find Function:%s", wsName.c_str());
		return FALSE;
	}

	auto fnExcuteCustomeFun = [&CL_Script_CustomeFunction_, this, &vlst]()
	{
		__try
		{
			return CL_Script_CustomeFunction_.pFunAddr(lpCustomeFunParm, vlst);
		}
		__except (CLLog::PrintExceptionCode(GetExceptionInformation()))
		{
			LogMsgBox(LOG_LEVEL_EXCEPTION, L"fnExcuteCustomeFun发生了异常");
		}
		return FALSE;
	};

	Log(LOG_LEVEL_NORMAL, L"执行自定义函数:%s", wsName.c_str());
	return fnExcuteCustomeFun();
}

BOOL CLScript::ExcuteContent(__in std::vector<CL_Script_TranCode>::iterator itr, __in const std::vector<CL_Script_TranCode>::iterator EndItr)
{
	auto FindCodeType = [this](__in em_CL_Script_Code_Type emType)
	{
		return std::find_if(ScriptCodeList.begin(), ScriptCodeList.end(), [&emType](CL_Script_TranCode& Cf){ return Cf.emCodeType == emType; });
	};

	while (itr != EndItr && !bExit)
	{
		BOOL bRetCode = TRUE;
		switch (itr->emCodeType)
		{
		case em_CL_Script_Code_Type_Function:
			bRetCode = ExcuteFunction(((CL_Script_TranCode_Function *)itr->pStruct)->wsFunName, ((CL_Script_TranCode_Function *)itr->pStruct)->ParmList);
			break;
			
		/*case em_CL_Script_Code_Type_If:
			bRetCode = ExcuteFunction(((CL_Script_TranCode_DefineFun *)itr->pStruct)->wszFunName, EmptyParmList);
			if (!bRetCode) // Jmp to '}' + 1
			{
				//itr = FindCodeType(em_CL_Script_Code_Type_Right_Brace);
			}
			else // Jmp to 'else'
			{

			}


			if (itr == ScriptCodeList.end())
			{
				LogMsgBox(LOG_LEVEL_EXCEPTION, L"Find 'if' -> '}' Faild! ");
				bExit = TRUE;
			}
			break;*/

		default:
			break;
		}

		if (IsExcuteNext())
			++itr;

		if (!bRetCode)
			return FALSE;
		
	}
	return TRUE;
}

BOOL CLScript::AddCustomeFunAddr(_In_ cwstring& wsFunName, __in CLScriptFun pScriptFunAddr)
{
	CL_Script_CustomeFunction CL_Script_CustomeFunction_;
	if (IsExistCustomeFunAddrList(wsFunName, CL_Script_CustomeFunction_))
		return TRUE;

	CustomeFunAddrList.push_back(CL_Script_CustomeFunction(wsFunName, pScriptFunAddr));
	return TRUE;
}

BOOL CLScript::IsExistScriptFunAddrList(_In_ cwstring& wsFunname, _Out_opt_ CL_Script_TranCode_DefineFun& CL_Script_TranCode_DefineFun_)
{
	BOOL bRetCode = FALSE;

	auto& itr = std::find_if(ScriptCodeList.begin(), ScriptCodeList.end(), [&wsFunname](CONST CL_Script_TranCode& CL_Script_TranCode_){
		return (CL_Script_TranCode_.emCodeType == em_CL_Script_Code_Type_DefineFun && ((CL_Script_TranCode_DefineFun *)CL_Script_TranCode_.pStruct)->wsFunName == wsFunname);
	});

	if (itr != ScriptCodeList.end())
	{
		bRetCode = TRUE;
		CL_Script_TranCode_DefineFun_ = *(CL_Script_TranCode_DefineFun *)itr->pStruct;
	}

	return bRetCode;
}

BOOL CLScript::IsExistCustomeFunAddrList(_In_ cwstring& wsFunname, _Out_opt_ CL_Script_CustomeFunction& CL_Script_CustomeFunction_)
{
	BOOL bRetCode = FALSE;

	auto& itr = std::find_if(CustomeFunAddrList.begin(), CustomeFunAddrList.end(), [&wsFunname](CL_Script_CustomeFunction& Cf){
		return Cf.wsFunName == wsFunname;
	});

	if (itr != CustomeFunAddrList.end())
	{
		CL_Script_CustomeFunction_ = *itr;
		bRetCode = TRUE;
	}

	return bRetCode;
}

BOOL CLScript::ExcuteCode_If(__in std::vector<CL_Script_TranCode>::iterator CodeItr, __in BOOL bResult)
{
	/*for (auto itr = CodeItr + 1; itr != ScriptCodeList.end(); ++itr)
	{
		
	}*/
	return TRUE;
}

std::vector<CL_Script_TranCode>::iterator CLScript::GetItr_By_CodeType(__in const std::vector<CL_Script_TranCode>::iterator Startitr, em_CL_Script_Code_Type emCodeType)
{
	return std::find_if(Startitr, ScriptCodeList.end(), [&emCodeType](CL_Script_TranCode& TranCode){ return TranCode.emCodeType == emCodeType; });
}

BOOL CLScript::Read(__in LPCWSTR pwszScriptPath)
{
	_CLStackTrace(L"CLScript::Read");
	if (!ReadScriptFile(pwszScriptPath))
		return FALSE;

	if (!AnalysisSourceCode())
	{
		Release();
		return FALSE;
	}

	return TRUE;
}

BOOL CLScript::Read(__in std::shared_ptr<WCHAR>& pScriptStr)
{
	try
	{
		if (!ReadScriptContent(pScriptStr.get()))
			return FALSE;

		if (!AnalysisSourceCode())
		{
			Release();
			return FALSE;
		}

		return TRUE;
	}
	catch (...)
	{
		Log(LOG_LEVEL_EXCEPTION,L"Read发生了异常");
	}
	return FALSE;
}

BOOL CLScript::Release()
{
	std::for_each(ScriptCodeList.begin(), ScriptCodeList.end(), [](CL_Script_TranCode& TC){
		switch (TC.emCodeType)
		{
		case em_CL_Script_Code_Type_DefineFun:
			delete reinterpret_cast<CL_Script_TranCode_DefineFun*>(TC.pStruct);
			break;
		case em_CL_Script_Code_Type_Function:
			delete reinterpret_cast<CL_Script_TranCode_Function*>(TC.pStruct);
			break;
		case em_CL_Script_Code_Type_If:
			break;
		case em_CL_Script_Code_Type_ElseIf:
			break;
		case em_CL_Script_Code_Type_While:
			break;
		default:
			break;
		}
	});

	ScriptCodeList.clear();
	return TRUE;
}

BOOL CLScript::Excute(_In_ cwstring& wsName)
{
	std::vector<CL_Script_TranCode_FunParm> EmptyParmList;

	//bExit = FALSE;
	return ExcuteFunction(wsName, EmptyParmList);
}

BOOL CLScript::ReadScriptContent(__in LPCSTR pszScriptContent)
{
	try
	{
		std::wstring wsContent = CCharacter::ASCIIToUnicode(pszScriptContent);

		CCharacter::Split(wsContent, L"\r\n", SourceCodeList, Split_Option_RemoveEmptyEntries);

		CLPublic::Vec_erase_if(SourceCodeList, [](CONST std::wstring& wsText){ return wsText.find(L"//") != -1; });
		return SourceCodeList.size() != 0;
	}
	catch (...)
	{
		Log(LOG_LEVEL_EXCEPTION, L"ReadScriptFile出现异常");
	}
	return FALSE;
}

BOOL CLScript::ReadScriptContent(_In_ cwstring& wsContent)
{
	try
	{
		CCharacter::Split(wsContent, L"\r\n", SourceCodeList, Split_Option_RemoveEmptyEntries);

		CLPublic::Vec_erase_if(SourceCodeList, [](CONST std::wstring& wsText){ return wsText.find(L"//") != -1; });
		return SourceCodeList.size() != 0;
	}
	catch (...)
	{
		Log(LOG_LEVEL_EXCEPTION, L"ReadScriptFile出现异常");
	}
	return FALSE;
}

CONST std::vector<CL_Script_CustomeFunction>& CLScript::GetCustomeFunList() CONST
{
	return CustomeFunAddrList;
}

CONST std::vector<CL_Script_TranCode>& CLScript::GetSourceList() CONST
{
	return ScriptCodeList;
}

BOOL CLScript::IsExcuteNext()
{
	return TRUE;
}

BOOL CLScript::Check(_Out_opt_ std::wstring& wsErrText)
{
#define Check_Function_Name			0x2
#define Check_Function_Left_Brace	0x4
#define Check_Function_Right_Brace	0x8

	DWORD dwFlag = 0x0;
	CL_Script_TranCode_DefineFun* pScriptFunction = nullptr;

	auto fnCheckBrace = [&pScriptFunction, &wsErrText](DWORD dwFlag)
	{
		if (pScriptFunction != nullptr)
		{
			if (!(dwFlag & Check_Function_Left_Brace))
			{
				wsErrText = CCharacter::FormatText(L"函数:%s 不存在'{'", pScriptFunction->wsFunName.c_str());
				return FALSE;
			}
			else if (!(dwFlag & Check_Function_Right_Brace))
			{
				wsErrText = CCharacter::FormatText(L"函数:%s 不存在'}'", pScriptFunction->wsFunName.c_str());
				return FALSE;
			}
		}
		return TRUE;
	};

	for (CONST auto& itm : ScriptCodeList)
	{
		if (itm.emCodeType == em_CL_Script_Code_Type::em_CL_Script_Code_Type_Left_Brace)
		{
			dwFlag |= Check_Function_Left_Brace;
			continue;
		}
		else if (itm.emCodeType == em_CL_Script_Code_Type::em_CL_Script_Code_Type_Right_Brace)
		{
			dwFlag |= Check_Function_Right_Brace;
			continue;
		}

		if (itm.emCodeType == em_CL_Script_Code_Type::em_CL_Script_Code_Type_DefineFun)
		{
			if (!fnCheckBrace(dwFlag))
				return FALSE;
	
			dwFlag = Check_Function_Name;
			pScriptFunction = reinterpret_cast<CL_Script_TranCode_DefineFun *>(itm.pStruct);
			continue;
		}
	}

	if (!fnCheckBrace(dwFlag))
		return FALSE;

	return TRUE;
}

VOID CLScript::SetSciptPtr()
{

}
