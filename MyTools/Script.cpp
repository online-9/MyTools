#include "stdafx.h"
#include "Script.h"
#include "CLFile.h"
#include "CLPublic.h"
#include "CLLog.h"

#define _SELF L"Script.cpp"
MyTools::CScript::CScript()
{
	_fnLogPtr = nullptr;
	_fnExceptionPtr = nullptr;
	_fnIfPtr = nullptr;
	_fnWhilePtr = nullptr;
	_pCurrentMethodContent = nullptr;
	_pCurrentScriptCode = nullptr;
}

MyTools::CScript::~CScript()
{
	Release();
}

BOOL MyTools::CScript::Read(_In_ em_ReadScript_Type emReadScriptType, _In_ CONST std::wstring& wsContent)
{
	std::vector<std::wstring> VecFileContent;
	switch (emReadScriptType)
	{
	case MyTools::CScript::em_ReadScript_Type_FilePath:
		if (!ReadScriptFile(wsContent, VecFileContent))
			return FALSE;
		break;
	case MyTools::CScript::em_ReadScript_Type_Text:
		if (!ReadScriptContent(wsContent, VecFileContent))
			return FALSE;
		break;
	default:
		break;
	}

	if (!FileContentToCode(VecFileContent))
	{
		Release();
		return FALSE;
	}

	return TRUE;
}

BOOL MyTools::CScript::AddCustomeFunAddr(_In_ CONST Script_Code_Custome_Method& CustomeMethod)
{
	if (CLPublic::Vec_find_if(_VecCustMethod, static_cast<Script_Code_Custome_Method *>(nullptr), [CustomeMethod](CONST Script_Code_Custome_Method& CustMethod) { return CustMethod.wsMethodName == CustomeMethod.wsMethodName; }))
	{
		PrintLog(__LINE__, L"Custome Method Already Exist:%s", CustomeMethod.wsMethodName.c_str());
		return FALSE;
	}

	_VecCustMethod.push_back(CustomeMethod);
	return TRUE;
}

BOOL MyTools::CScript::Excute(_In_ CONST std::wstring& wsMethodName)
{
	AddExcuteQueue(wsMethodName);
	while (!_QueCurrentPos.empty())
	{
		auto CurrentPos = _QueCurrentPos.top();
		RemoveQueue();

		if (!ExcuteDefMethod(CurrentPos.wsDefMethodName, CurrentPos.ulCodeHash))
			return FALSE;

		
	}
	return TRUE;
}

BOOL MyTools::CScript::ReadScriptFile(_In_ CONST std::wstring& wsFilePath, _Out_ std::vector<std::wstring>& VecFileContent)
{
	std::wstring wsContent;
	if (!CLFile::ReadUnicodeFile(wsFilePath, wsContent))
	{
		PrintLog(__LINE__, L"ReadScriptFile Fiald! Path:%s", wsFilePath.c_str());
		return FALSE;
	}

	return ReadScriptContent(wsContent, VecFileContent);
}

BOOL MyTools::CScript::ReadScriptContent(_In_ CONST std::wstring& wsFileContent, _Out_ std::vector<std::wstring>& VecFileContent)
{
	CCharacter::Split(wsFileContent, L"\r\n", VecFileContent, Split_Option_RemoveEmptyEntries | Split_Option_KeepOnly);

	CLPublic::Vec_erase_if(VecFileContent, [](CONST std::wstring& wsText) { return wsText.find(L"//") != -1; });
	return VecFileContent.size() != 0;
}

MyTools::CScript::em_Script_CodeType MyTools::CScript::GetScriptCodeType(_In_ CONST std::wstring& wsContent) CONST
{
	struct Script_Code_TextType
	{
		std::wstring		wsValue;
		em_Script_CodeType	emCodeType;
	};

	CONST static std::vector<Script_Code_TextType> VecScriptCodeType =
	{
		{ L"//",		em_Script_CodeType_Comment },
		{ L"function ", em_Script_CodeType_DefMethod },
		{ L"if(",		em_Script_CodeType_If },
		{ L"while(",	em_Script_CodeType_While },
		{ L"{",			em_Script_CodeType_LeftBrace },
		{ L"}",			em_Script_CodeType_RightBrace },
		{ L"(",			em_Script_CodeType_Method },
	};

	auto p = CLPublic::Vec_find_if_Const(VecScriptCodeType, [wsContent](CONST Script_Code_TextType& ScriptCodeType_) { return wsContent.find(ScriptCodeType_.wsValue) != -1; });
	return p == nullptr ? em_Script_CodeType::em_Script_CodeType_None : p->emCodeType;
}

BOOL MyTools::CScript::FillDefMethod(_In_ UINT& Index, _In_ CONST std::vector<std::wstring>& VecFileContent, _Out_ Script_Code_DefMethod& DefMethodContent_) CONST
{
	for (UINT& i = Index; i < VecFileContent.size(); ++i)
	{
		CONST auto& ScriptFileContent = VecFileContent.at(i);
		auto emScriptCodeType = GetScriptCodeType(ScriptFileContent);
		if (emScriptCodeType == em_Script_CodeType_None || emScriptCodeType == em_Script_CodeType_Comment)
			continue;

		switch (emScriptCodeType)
		{
		case MyTools::CScript::em_Script_CodeType_DefMethod:
			PrintLog(__LINE__, L"UnExist '}' in DefMethod:%s", DefMethodContent_.wsMethodName.c_str());
			return FALSE;
		case MyTools::CScript::em_Script_CodeType_Method:
			if (!AnalysisCode_Method(ScriptFileContent, DefMethodContent_))
				return FALSE;
			break;
		case MyTools::CScript::em_Script_CodeType_LeftBrace:
			if (!AnalysisCode_Brace(emScriptCodeType, DefMethodContent_))
				return FALSE;
			break;
		case MyTools::CScript::em_Script_CodeType_RightBrace:
			return AnalysisCode_Brace(emScriptCodeType, DefMethodContent_);
		case MyTools::CScript::em_Script_CodeType_If: case MyTools::CScript::em_Script_CodeType_While:
			if (!AnalysisCode_If(emScriptCodeType, ScriptFileContent, DefMethodContent_))
				return FALSE;
			break;
		default:
			break;
		}
	}

	PrintLog(__LINE__, L"UnExist '}' in Script File End!");
	return FALSE;
}

BOOL MyTools::CScript::FileContentToCode(_In_ CONST std::vector<std::wstring>& VecFileContent)
{
	for (UINT i = 0;i < VecFileContent.size();++i)
	{
		CONST auto& itm = VecFileContent.at(i);
		if (GetScriptCodeType(itm) != em_Script_CodeType_DefMethod)
			continue;

		Script_Code_DefMethod DefMethod_;
		if (!AnalysisCode_DefMethod(itm, DefMethod_))
			return FALSE;

		if (CLPublic::Vec_find_if(_VecCode, static_cast<Script_Code_DefMethod *>(nullptr), [DefMethod_](CONST Script_Code_DefMethod& ScriptCodeDefMethod) { return ScriptCodeDefMethod.wsMethodName == DefMethod_.wsMethodName; }))
		{
			PrintLog(__LINE__, L"Already Exist DefMethod:%s", DefMethod_.wsMethodName.c_str());
			return FALSE;
		}

		if (!FillDefMethod(++i, VecFileContent, DefMethod_))
			return FALSE;

		_VecCode.push_back(std::move(DefMethod_));
	}

	return TRUE;
}

BOOL MyTools::CScript::Release()
{
	for (CONST auto& itm : _VecCode)
	{
		for (CONST auto& Code_ : itm.VecScriptCode)
		{
			if (Code_.pCode == nullptr)
				continue;

			switch (Code_.pCode->emScriptCodeType)
			{
			case em_Script_CodeType_If: case em_Script_CodeType_While:
				delete static_cast<CONST Script_Code_If *>(Code_.pCode);
				break;
			case em_Script_CodeType_Method:
				delete static_cast<CONST Script_Code_Method *>(Code_.pCode);
				break;
			default:
				delete Code_.pCode;
				break;
			}
		}
	}
	return TRUE;
}

VOID MyTools::CScript::PrintLog(_In_ int nLine, _In_ LPCWSTR pwszFormat, ...) CONST
{
	if (_fnLogPtr != nullptr)
	{
		va_list		args;
		wchar_t		szBuff[1024] = { 0 };

		va_start(args, pwszFormat);
		_vsnwprintf_s(szBuff, _countof(szBuff) - 1, _TRUNCATE, pwszFormat, args);
		va_end(args);

		_fnLogPtr(L"Script.cpp", nLine, std::wstring(szBuff));
	}
}

BOOL MyTools::CScript::ExcuteDefMethod(_In_ CONST std::wstring& wsMethodName, _In_ ULONG ulHash)
{
	CONST auto p = ExistDefMethod(wsMethodName);
	if (p == nullptr)
	{
		PrintLog(__LINE__, L"UnExist Method:%s", wsMethodName.c_str());
		return FALSE;
	}

	if (ulHash == NULL)
	{
		// Excute '{'
		//if (!ExcuteScriptCode(wsMethodName, p->VecScriptCode.at(0))) 
		//	return FALSE;
		if (p->VecScriptCode.size() < 2)
		{
			PrintLog(__LINE__, L"VecScriptCode.size()=%d and < 2, MethodName=%s", p->VecScriptCode.size(), wsMethodName.c_str());
			return FALSE;
		}

		// Excute Next Code
		AddExcuteQueue(wsMethodName, p->VecScriptCode.at(1).ulCodeHash);
		return TRUE;
	}

	for (UINT i = 0;i < p->VecScriptCode.size() - 1;++i)
	{
		if (p->VecScriptCode.at(i).ulCodeHash == ulHash)
		{
			// Excute Next Code
			AddExcuteQueue(wsMethodName, p->VecScriptCode.at(i + 1).ulCodeHash);
			_pCurrentScriptCode = &p->VecScriptCode.at(i);
			return ExcuteScriptCode(wsMethodName, p->VecScriptCode.at(i));
		}
	}

	// Excute Code is '}', Pass!
	return TRUE;
}

BOOL MyTools::CScript::ExcuteScriptCode(_In_ CONST std::wstring& wsMethodName, _In_ CONST Script_Code& ScriptCode_)
{
	switch (ScriptCode_.pCode->emScriptCodeType)
	{
	case em_Script_CodeType_If:
		if (_fnIfPtr == nullptr)
		{
			PrintLog(__LINE__, L"Not Declare Method if Ptr!!!");
			return FALSE;
		}
		else if (_fnIfPtr(*static_cast<CONST Script_Code_If*>(ScriptCode_.pCode)))
		{
			AddExcuteQueue(static_cast<CONST Script_Code_If*>(ScriptCode_.pCode)->wsMethodName, NULL);
			return TRUE;
		}
		break;
	case em_Script_CodeType_While:
		if (_fnWhilePtr == nullptr)
		{
			PrintLog(__LINE__, L"Not Declare Method While Ptr!!!");
			return FALSE;
		}
		else if (_fnWhilePtr(*static_cast<CONST Script_Code_If*>(ScriptCode_.pCode)))
		{
			ExcuteLoop(static_cast<CONST Script_Code_If*>(ScriptCode_.pCode)->wsMethodName, ScriptCode_);
			return TRUE;
		}
		break;
	case em_Script_CodeType_Method:
		if (ExistDefMethod(static_cast<CONST Script_Code_Method *>(ScriptCode_.pCode)->wsMethodName) != nullptr)
		{
			AddExcuteQueue(static_cast<CONST Script_Code_Method *>(ScriptCode_.pCode)->wsMethodName, NULL);
			return TRUE;
		}
		if (ExistCustMethod(static_cast<CONST Script_Code_Method *>(ScriptCode_.pCode)->wsMethodName) != nullptr)
		{
			return ExcuteCustMethod(wsMethodName, static_cast<CONST Script_Code_Method *>(ScriptCode_.pCode));
		}

		PrintLog(__LINE__, L"UnExist Method:%s", wsMethodName.c_str());
		return FALSE;
	default:
		break;
	}
	return TRUE;
}

VOID MyTools::CScript::ExcuteLoop(_In_ CONST std::wstring& wsExcuteMethodName, _In_ CONST Script_Code& CurrentScriptCode_)
{
	// Remove Last One!
	RemoveQueue();

	// Excute Again 'while'
	AddExcuteQueue(static_cast<CONST Script_Code_Method *>(CurrentScriptCode_.pCode)->wsMethodName, CurrentScriptCode_.ulCodeHash);

	// Excute 'while(..CALLBACK)' -> CALLBACK();
	AddExcuteQueue(wsExcuteMethodName, NULL);
}

BOOL MyTools::CScript::ExcuteCustMethod(_In_ CONST std::wstring&, _In_ CONST Script_Code_Method* pCodeMethod)
{
	auto p = ExistCustMethod(pCodeMethod->wsMethodName);
	auto fnExcuteCustomeFun = [p, pCodeMethod, this]()
	{
		__try
		{
			Log(LOG_LEVEL_NORMAL, L"执行函数:%s", p->wsMethodName.c_str());
			_pCurrentMethodContent = pCodeMethod;
			return p->MethodPtr();
		}
		__except (CLLog::PrintExceptionCode(GetExceptionInformation()))
		{
			PrintLog(__LINE__, L"fnExcuteCustomeFun发生了异常");
		}

		return _fnExceptionPtr != nullptr ? _fnExceptionPtr() : FALSE;
	};	
	return fnExcuteCustomeFun();
}

VOID MyTools::CScript::AddExcuteQueue(_In_ CONST std::wstring& wsMethodName, _In_ ULONG ulHash /*= NULL*/)
{
	Script_Current_Content CurrentPos;
	CurrentPos.wsDefMethodName = wsMethodName;
	CurrentPos.ulCodeHash = ulHash;
	_QueCurrentPos.push(std::move(CurrentPos));
}

VOID MyTools::CScript::RemoveQueue()
{
	_QueCurrentPos.pop();
}

CONST MyTools::CScript::Script_Code_DefMethod* MyTools::CScript::ExistDefMethod(_In_ CONST std::wstring& wsMethodName) CONST
{
	return CLPublic::Vec_find_if_Const(_VecCode, [wsMethodName](CONST Script_Code_DefMethod& DefMethod) { return DefMethod.wsMethodName == wsMethodName; });
}

CONST MyTools::CScript::Script_Code_Custome_Method* MyTools::CScript::ExistCustMethod(_In_ CONST std::wstring& wsMethodName) CONST
{
	return CLPublic::Vec_find_if_Const(_VecCustMethod, [wsMethodName](CONST Script_Code_Custome_Method& CustMethod) { return CustMethod.wsMethodName == wsMethodName; });
}

BOOL MyTools::CScript::AnalysisCode_DefMethod(_In_ CONST std::wstring& wsContent, _Out_ Script_Code_DefMethod& DefMethodContent_) CONST
{
	if (wsContent.find(L"(") == -1)
	{
		PrintLog(__LINE__, L"Method UnExist '(' when Content=%s", wsContent.c_str());
		return FALSE;
	}

	CCharacter::GetRemoveRight(wsContent, L"function ", DefMethodContent_.wsMethodName);
	CCharacter::GetRemoveLeft(DefMethodContent_.wsMethodName, L"(", DefMethodContent_.wsMethodName);
	CCharacter::Trim_W(DefMethodContent_.wsMethodName);
	return TRUE;
}

BOOL MyTools::CScript::AnalysisCode_Brace(_In_ em_Script_CodeType emScriptCodeType_, _Out_ Script_Code_DefMethod& DefMethodContent_) CONST
{
	if (emScriptCodeType_ == MyTools::CScript::em_Script_CodeType_LeftBrace)
	{
		// Already Exist '{' ?
		if (CLPublic::Vec_find_if(DefMethodContent_.VecScriptCode, static_cast<Script_Code *>(nullptr), [](CONST Script_Code& ScriptCode_) { return ScriptCode_.pCode->emScriptCodeType == em_Script_CodeType_LeftBrace; }))
		{
			PrintLog(__LINE__, L"Already Exist '{' in Method:%s", DefMethodContent_.wsMethodName.c_str());
			return FALSE;
		}
	}
	else
	{
		// Right Brace -> Is Exist '{'
		// Already Exist '{' ?
		if (!CLPublic::Vec_find_if(DefMethodContent_.VecScriptCode, static_cast<Script_Code *>(nullptr), [](CONST Script_Code& ScriptCode_) { return ScriptCode_.pCode->emScriptCodeType == em_Script_CodeType_LeftBrace; }))
		{
			PrintLog(__LINE__, L"UnExist '{' in Method:%s", DefMethodContent_.wsMethodName.c_str());
			return FALSE;
		}
	}

	Script_Code_Type* pCode = new Script_Code_Type(emScriptCodeType_);
	if (pCode == nullptr)
	{
		PrintLog(__LINE__, L"Alloc Memory Faild! pCode = NULL! Method=%s", DefMethodContent_.wsMethodName.c_str());
		return FALSE;
	}

	Script_Code ScriptCode;
	ScriptCode.pCode = pCode;
	DefMethodContent_.VecScriptCode.push_back(std::move(ScriptCode));
	return TRUE;
}

BOOL MyTools::CScript::AnalysisCode_If(_In_ em_Script_CodeType emScriptCodeType_, _In_ CONST std::wstring& wsContent, _Out_ Script_Code_DefMethod& DefMethodContent_) CONST
{
	// if(ExistMonster,[a,b,c,d],CALLBACK)
	std::wstring Text;
	CCharacter::GetRemoveRight(wsContent, L"(", Text);
	CCharacter::GetRemoveLeft(Text, L")", Text);

	// ExistMonster,[a,b,c,d],CALLBACK
	if (wsContent.find(L",") == -1)
	{
		PrintLog(__LINE__, L"UnExist ',' in LeftValue on Method:%s, CodeText=%s", DefMethodContent_.wsMethodName.c_str(), wsContent.c_str());
		return FALSE;
	}

	// Expression Left Value
	Script_Code_If* pScriptCodeIf = new Script_Code_If;
	if (pScriptCodeIf == nullptr)
	{
		PrintLog(__LINE__, L"Alloc Memory Faild! pCode = NULL! Method=%s", DefMethodContent_.wsMethodName.c_str());
		return FALSE;
	}

	pScriptCodeIf->emScriptCodeType = emScriptCodeType_;

	CCharacter::GetRemoveLeft(Text, L",", pScriptCodeIf->wsCondition);
	CCharacter::Trim_W(pScriptCodeIf->wsCondition);
	CCharacter::GetRemoveRight(Text, L",", Text);

	// [a,b,c,d],CALLBACK
	if (Text.find(L",") == -1)
	{
		PrintLog(__LINE__, L"UnExist ',' in RightValue on Method:%s, CodeText=%s", DefMethodContent_.wsMethodName.c_str(), wsContent.c_str());
		delete pScriptCodeIf;
		return FALSE;
	}

	// Expression Right Value
	if (Text.find(L"[") != -1)
	{
		if (Text.find(L"]") == -1)
		{
			PrintLog(__LINE__, L"UnExist ']' in Script_if, MethodName=%s, Text=%s", DefMethodContent_.wsMethodName.c_str(), wsContent.c_str());
			delete pScriptCodeIf;
			return FALSE;
		}

		// Exist Multiple Parameter
		CONST static std::vector<std::wstring> VecSplitText = { L"[",L"]" };

		std::vector<std::wstring> VecText;
		CCharacter::GetSplit_By_List(Text, VecSplitText, VecText, Split_Option_RemoveEmptyEntries);
		if (VecText.size() == 0)
		{
			PrintLog(__LINE__, L"Analysis '[]' in Script_If Faild, MethodName=%s, Text=%s", DefMethodContent_.wsMethodName.c_str(), wsContent.c_str());
			delete pScriptCodeIf;
			return FALSE;
		}

		std::vector<std::wstring> VecValue;
		CCharacter::Split(VecText.at(0), L",", VecValue, Split_Option_RemoveEmptyEntries | Split_Option_KeepOnly);
		for (auto& itm : VecValue)
			pScriptCodeIf->VecValue.push_back(Script_Code_MethodParameter(CCharacter::Trim_W(itm)));

		CCharacter::GetRemoveRight(Text, L",", Text);
	}
	else // a,CALLBACK
	{
		std::wstring wsValue;
		CCharacter::GetRemoveLeft(Text, L",", wsValue);
		CCharacter::GetRemoveRight(Text, L",", Text);

		pScriptCodeIf->VecValue.push_back(std::move(CCharacter::Trim_W(wsValue)));
	}

	pScriptCodeIf->wsMethodName = std::move(CCharacter::Trim_W(Text));
	
	Script_Code ScriptCode;
	ScriptCode.pCode = pScriptCodeIf;
	DefMethodContent_.VecScriptCode.push_back(std::move(ScriptCode));
	return TRUE;
}

BOOL MyTools::CScript::AnalysisCode_Method(_In_ CONST std::wstring& wsContent, _Out_ Script_Code_DefMethod& DefMethodContent_) CONST
{
	if (wsContent.find(L")") == -1)
	{
		PrintLog(__LINE__, L"UnExist ')' in Method=%s, Text=%s", DefMethodContent_.wsMethodName.c_str(), wsContent.c_str());
		return FALSE;
	}

	// AAA(a,[b,c,d],e)
	Script_Code_Method* pScriptCodeMethod = new Script_Code_Method;
	if (pScriptCodeMethod == nullptr)
	{
		PrintLog(__LINE__, L"Alloc Memory Faild! pCode = NULL! Method=%s, Text=%s", DefMethodContent_.wsMethodName.c_str(), wsContent.c_str());
		return FALSE;
	}


	CCharacter::GetRemoveLeft(wsContent, L"(", pScriptCodeMethod->wsMethodName);
	CCharacter::Trim_W(pScriptCodeMethod->wsMethodName);


	std::wstring Text;
	CCharacter::GetRemoveRight(wsContent, L"(", Text);
	CCharacter::GetRemoveLeft(Text, L")", Text);


	static CONST std::vector<std::wstring> VecSplit = { L"[",L"]" };
	std::vector<std::wstring> VecSplitText;
	CCharacter::GetSplit_By_List(Text, VecSplit, VecSplitText, Split_Option_None);

	std::vector<std::wstring> VecText;
	if (Text.find(L",") != -1)
		CCharacter::Split(Text, L",", VecText, Split_Option_None);
	else if (!CCharacter::Trim_W(Text).empty())
		VecText.push_back(std::move(Text));
	else if (VecSplitText.size() == 1 && VecText.size() == 0)
		VecText.push_back(L"");

	for (auto itr = VecText.begin(); itr != VecText.end() && VecSplitText.size() != 0; ++itr)
	{
		if (!itr->empty())
			continue;
		*itr = std::move(VecSplitText.at(0));
		VecSplitText.erase(VecSplitText.begin());
	}

	for (auto& itm : VecText)
	{
		Script_Code_MethodParameter MethodParameter(CCharacter::Trim_W(itm));
		pScriptCodeMethod->VecParm.push_back(std::move(MethodParameter));
	}

	Script_Code ScriptCode;
	ScriptCode.pCode = pScriptCodeMethod;
	DefMethodContent_.VecScriptCode.push_back(std::move(ScriptCode));
	return TRUE;
}
