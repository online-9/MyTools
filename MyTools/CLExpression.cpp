#include "stdafx.h"
#include "CLExpression.h"
#include "Log.h"
#include "Character.h"
#include "CLPublic.h"
#define _SELF L"CLExpression.cpp"

UINT CLExpression::Push(_In_ std::function<VOID(CONST std::vector<std::wstring>&)> fnPtr, _In_ CONST std::wstring& wsFunName) throw()
{
	ExpressionFunPtr FunPtrCustome_ = { fnPtr, wsFunName };
	VecFunPtr.push_back(std::move(FunPtrCustome_));
	return static_cast<UINT>(VecFunPtr.size());
}

VOID CLExpression::SetVecExprFunPtr(_In_ CONST std::vector<ExpressionFunPtr>& ParmVecFunPtr) throw()
{
	VecFunPtr = ParmVecFunPtr;
}

BOOL CLExpression::Run(_In_ CONST std::wstring& wsText) throw()
{
	if (wsText.find(L")") == -1 || wsText.find(L")") == -1)
	{
		LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"'%s' UnExist '(' or ')'", wsText.c_str());
		return FALSE;
	}
	if (CCharacter::GetCount_By_CharacterW(wsText,L"\"") % 2 != 0)
	{
		LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"'\"' have to make a pair!");
		return FALSE;
	}

	std::wstring wsFunName;
	std::wstring wsExpText = wsText;
	CCharacter::GetRemoveLeft(wsExpText, L"(", wsFunName);
	CCharacter::GetRemoveRight(wsExpText, L"(", wsExpText);
	CCharacter::GetRemoveLeft(wsExpText, L")", wsExpText);

	std::vector<std::wstring> VecParm;
	CCharacter::GetVecByParm_RemoveQuotes(wsExpText, ',', VecParm);

	auto p = CLPublic::Vec_find_if(VecFunPtr, [wsFunName](ExpressionFunPtr& ExprFunPtr){ return ExprFunPtr.wsFunName == wsFunName; });
	if (p != nullptr)
	{
		p->fnPtr(VecParm);
		return TRUE;
	}

	LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"UnExist Function Name '%s'", wsFunName.c_str());
	auto pHelp = CLPublic::Vec_find_if(VecFunPtr, [](ExpressionFunPtr& ExprFunPtr){ return ExprFunPtr.wsFunName == L"Help"; });
	if (pHelp != nullptr)
	{
		pHelp->fnPtr(VecParm);
		return FALSE;
	}

	
	return FALSE;
}
