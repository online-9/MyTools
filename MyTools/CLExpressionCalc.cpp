#include "stdafx.h"
#include "CLExpressionCalc.h"
#include <algorithm>
#include "Character.h"
#include "Log.h"
#include "CLPublic.h"

#define _SELF L"CLExpressionCalc.cpp"

BOOL CLExpressionCalc::Analysis(_In_ CONST std::wstring& wsText) throw()
{
	std::vector<std::wstring> VecParm;
	CCharacter::Split(wsText, L" ", VecParm, Split_Option_KeepOnly | Split_Option_RemoveEmptyEntries);

	ExpContent.emCmdType = GetCmdType(VecParm.at(0));
	if (ExpContent.emCmdType == em_Cmd_Type::em_Cmd_Type_Invalid)
	{
		LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"invalid Cmd Type:%s", VecParm.at(0).c_str());
		return FALSE;
	}


	if (VecParm.size() < 2)
	{
		LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"parm < 2");
		return FALSE;
	}

	wsExpText = VecParm.at(1);
	if (!GetExpression())
		return FALSE;

	for (UINT i = 2; i < VecParm.size(); ++i)
	{
		auto itm = CCharacter::MakeTextToLower(VecParm.at(i));
		if (CCharacter::GetCount_By_CharacterW(itm, L"-") == 0)
		{
			LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"Invalid Format!!");
			return FALSE;
		}

		std::wstring wsNumber;
		CCharacter::GetRemoveRight(itm, L"-", wsNumber);
		if (!IsNumber(wsNumber))
		{
			LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"Invalid Format:%s", itm.c_str());
			return FALSE;
		}
		ExpContent.uMemWatchCount = static_cast<UINT>(_wtoi(wsNumber.c_str()));
	}

	if (!GetRpn())
		return FALSE;

	switch (ExpContent.emCmdType)
	{
	case em_Cmd_Type::em_Cmd_Type_Calc:
		return CalcResult_By_Rpn(nullptr);
	case em_Cmd_Type::em_Cmd_Type_dd:
		return ReadMem_By_Rpn();
	default:
		break;
	}

	LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"UnFinish Cmd Type!");
	return FALSE;
}

BOOL CLExpressionCalc::IsConformToCmdType(_In_ CONST std::wstring& wsText) CONST throw()
{
	std::vector<std::wstring> VecParm;
	CCharacter::Split(wsText, L" ", VecParm, Split_Option_KeepOnly | Split_Option_RemoveEmptyEntries);

	return GetCmdType(VecParm.at(0)) != em_Cmd_Type::em_Cmd_Type_Invalid;
}

CLExpressionCalc::em_Cmd_Type CLExpressionCalc::GetCmdType(_In_ CONST std::wstring& wsText) CONST throw()
{
	auto wsLowerText = CCharacter::MakeTextToLower(wsText);
	if (wsLowerText == L"dd")
		return em_Cmd_Type::em_Cmd_Type_dd;
	else if (wsLowerText == L"calc")
		return em_Cmd_Type::em_Cmd_Type_Calc;

	return em_Cmd_Type::em_Cmd_Type_Invalid;
}

CLExpressionCalc::em_Text_Type CLExpressionCalc::GetTextType(_In_ CONST UINT& uIndex) throw()
{
	static CONST WCHAR wszNumberText[] = { L"0123456789xABCDEFabcdef" };
	static CONST WCHAR wchSymbol[] = { L"+-*/%^|~&" };

	if (std::find_if(std::begin(wszNumberText), std::end(wszNumberText), [this, uIndex](CONST WCHAR wch){ return wch == wsExpText.at(uIndex); }) != std::end(wszNumberText))
	{
		return em_Text_Type::em_Text_Type_Number;
	}

	if (std::find_if(std::begin(wchSymbol), std::end(wchSymbol), [this, uIndex](CONST WCHAR wsSymbol){ return wsSymbol == wsExpText.at(uIndex); }) != std::end(wchSymbol))
	{
		return em_Text_Type::em_Text_Type_Symbol;
	}

	switch (wsExpText.at(uIndex))
	{
	case L'(': case L')': case L'[': case L']':
		return em_Text_Type::em_Text_Type_Bracket;
	case L' ': case L'\0':
		return em_Text_Type::em_Text_Type_NULL;
	case L'<': case L'>':
		if (uIndex + 1 >= wsExpText.length()) // two '<<' at least
			em_Text_Type::em_Text_Type_Invalid;

		if (wsExpText.at(uIndex) == L'<' && wsExpText.at(uIndex + 1) != L'<')
			return em_Text_Type::em_Text_Type_Invalid;
		if (wsExpText.at(uIndex) == L'>' && wsExpText.at(uIndex + 1) != L'>')
			return em_Text_Type::em_Text_Type_Invalid;

		return em_Text_Type::em_Text_Type_Symbol;
	default:
		break;
	}

	return em_Text_Type::em_Text_Type_Invalid;
}

CLExpressionCalc::em_Content_Type CLExpressionCalc::GetContentType(_In_ CONST std::wstring& wsSymbolText, _In_ em_Text_Type emTextType) throw()
{
	struct TextContent
	{
		em_Content_Type emContentType;
		std::wstring    wsText;
	};

	if (emTextType == em_Text_Type::em_Text_Type_Symbol)
	{
		CONST static std::vector<TextContent> Vec =
		{
			{ em_Content_Type::em_Content_Type_Symbol_Add, L"+" },
			{ em_Content_Type::em_Content_Type_Symbol_Sub, L"-" },
			{ em_Content_Type::em_Content_Type_Symbol_Mul, L"*" },
			{ em_Content_Type::em_Content_Type_Symbol_Div, L"/" },
			{ em_Content_Type::em_Content_Type_Symbol_Mod, L"%" },
			{ em_Content_Type::em_Content_Type_Symbol_ExOr, L"^" },
			{ em_Content_Type::em_Content_Type_Symbol_InOr, L"|" },
			{ em_Content_Type::em_Content_Type_Symbol_Comp, L"~" },
			{ em_Content_Type::em_Content_Type_Symbol_And, L"&" },
			{ em_Content_Type::em_Content_Type_Symbol_LeftShift, L"<<" },
			{ em_Content_Type::em_Content_Type_Symbol_RightShift, L">>" },
		};

		auto p = CLPublic::Vec_find_if_Const(Vec, [wsSymbolText](CONST TextContent& TC){ return TC.wsText == wsSymbolText; });
		return p != nullptr ? p->emContentType : em_Content_Type::em_Content_Type_None;
	}
	else if (emTextType == em_Text_Type::em_Text_Type_Bracket)
	{
		if (wsSymbolText == L"(")
			return em_Content_Type::em_Content_Type_LeftBracket;
		else if (wsSymbolText == L")")
			return em_Content_Type::em_Content_Type_RightBracket;
		else if (wsSymbolText == L"[")
			return em_Content_Type::em_Content_Type_LeftAddress;
		else if (wsSymbolText == L"]")
			return em_Content_Type::em_Content_Type_RightAddress;

	}

	return em_Content_Type::em_Content_Type_None;
}

BOOL CLExpressionCalc::GetSymbolText(_In_ _Out_ UINT& uIndex, _Out_ std::wstring& wsSymbolText) throw()
{
	if (wsExpText.at(uIndex) == L'<')
	{
		if (wsExpText.at(uIndex + 1) != L'<')
			return FALSE;
		wsSymbolText = L"<<";
		uIndex += 1;
	}
	else if (wsExpText.at(uIndex) == L'>')
	{
		if (wsExpText.at(uIndex + 1) != L'>')
			return FALSE;
		wsSymbolText = L">>";
		uIndex += 1;
	}
	else
	{
		wsSymbolText.push_back(wsExpText.at(uIndex));
	}
	return TRUE;
}

BOOL CLExpressionCalc::GetExpression() throw()
{
	std::wstring wsNumber;
	em_Text_Type emLastTextType = em_Text_Type::em_Text_Type_NULL;
	BOOL bNegative = FALSE;
	UINT uIndex = 0;

	while (uIndex < wsExpText.length())
	{
		auto emTextType = GetTextType(uIndex);
		switch (emTextType)
		{
		case em_Text_Type_Invalid:
			LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"Invalid Character '%c'", wsExpText.at(uIndex));
			return FALSE;
		case em_Text_Type_Number:
			wsNumber.push_back(wsExpText.at(uIndex));
			break;
		case em_Text_Type_Symbol: case em_Text_Type_Bracket:
		{
			if (!wsNumber.empty())
			{
				if (bNegative)
					wsNumber = L"-" + wsNumber;

				ExpContent.ExpressionVec.push_back(ExpressionInfo{ em_Content_Type_Number, wsNumber });
				wsNumber.clear();
				bNegative = FALSE;
			}

			// 2次连续出现符号, 这次的符号必须是-的才行
			if (emTextType != em_Text_Type_Bracket && emLastTextType == em_Text_Type_Symbol)
			{
				if (wsExpText.at(uIndex) != L'-')
				{
					LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"Prohibition of the continuous emergence of symbols: '%c'", wsExpText.at(uIndex));
					return FALSE;
				}
				bNegative = TRUE;
				break;
			}


			std::wstring wsSymbolText;
			if (!GetSymbolText(uIndex, wsSymbolText))
			{
				LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"Invalid Symbol '%c'", wsExpText.at(uIndex));
				return FALSE;
			}

			auto emContentType = GetContentType(wsSymbolText, emTextType);
			if (emContentType == em_Content_Type::em_Content_Type_None)
			{
				LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"Invalid Symbol '%c'", wsExpText.at(uIndex));
				return FALSE;
			}

			ExpContent.ExpressionVec.push_back(ExpressionInfo{ emContentType, wsSymbolText });
		}
		break;
		case em_Text_Type_NULL:
			break;
		default:
			break;
		}

		uIndex += 1;
		emLastTextType = emTextType;
	}

	if (!wsNumber.empty())
	{
		if (bNegative)
			wsNumber = L"-" + wsNumber;

		ExpContent.ExpressionVec.push_back(ExpressionInfo{ em_Content_Type_Number, wsNumber });
		wsNumber.clear();
	}
	return TRUE;
}

int CLExpressionCalc::GetPriority(_In_ em_Content_Type emContentType) throw()
{
	return (emContentType == em_Content_Type::em_Content_Type_Symbol_Add || emContentType == em_Content_Type::em_Content_Type_Symbol_Sub) ? 0x1 : 0x2;
}

BOOL CLExpressionCalc::CompPrioity(_In_ em_Content_Type emTextType1, _In_ em_Content_Type emTextType2) throw()
{
	return GetPriority(emTextType1) > GetPriority(emTextType2);
}

BOOL CLExpressionCalc::GetRpn() throw()
{
	std::stack<ExpressionInfo> StackSymbol;
	std::stack<ExpressionInfo> tmpRpn;
	for (CONST auto& itm : ExpContent.ExpressionVec)
	{
		if (itm.emContentType == em_Content_Type::em_Content_Type_Number)
		{
			tmpRpn.push(itm);
			continue;
		}

		if (itm.emContentType == em_Content_Type::em_Content_Type_LeftBracket || itm.emContentType == em_Content_Type::em_Content_Type_LeftAddress)
		{
			StackSymbol.push(itm);
			continue;
		}

		if (itm.emContentType == em_Content_Type::em_Content_Type_RightBracket)
		{
			while (StackSymbol.top().emContentType != em_Content_Type::em_Content_Type_LeftBracket)
			{
				tmpRpn.push(std::move(StackSymbol.top()));
				StackSymbol.pop();
			}
			StackSymbol.pop();
			continue;
		}

		if (itm.emContentType == em_Content_Type::em_Content_Type_RightAddress)
		{
			BOOL bExistAddress = FALSE;
			do
			{
				if (StackSymbol.top().emContentType == em_Content_Type::em_Content_Type_LeftAddress)
					bExistAddress = TRUE;
				tmpRpn.push(std::move(StackSymbol.top()));
				StackSymbol.pop();
			} while (StackSymbol.top().emContentType != em_Content_Type::em_Content_Type_LeftAddress);

			if (!bExistAddress)
			{
				tmpRpn.push(std::move(StackSymbol.top()));
				StackSymbol.pop();
			}


			// Keep '['
			continue;
		}

		if (StackSymbol.empty() || StackSymbol.top().emContentType == em_Content_Type::em_Content_Type_LeftBracket || StackSymbol.top().emContentType == em_Content_Type::em_Content_Type_LeftAddress)
		{
			StackSymbol.push(itm);
			continue;
		}

		if (CompPrioity(itm.emContentType, StackSymbol.top().emContentType))
		{
			StackSymbol.push(itm);
			continue;
		}

		tmpRpn.push(std::move(StackSymbol.top()));
		StackSymbol.pop();
		StackSymbol.push(itm);
	}

	while (!StackSymbol.empty())
	{
		tmpRpn.push(std::move(StackSymbol.top()));
		StackSymbol.pop();
	}

	while (!tmpRpn.empty())
	{
		Rpn.push(std::move(tmpRpn.top()));
		tmpRpn.pop();
	}

	return TRUE;
}

DWORD CLExpressionCalc::CalcResult_By_Parm(_In_ DWORD dwNumberLeft, _In_ DWORD dwNumberRight, _In_ em_Content_Type emSymbolType) throw()
{
	switch (emSymbolType)
	{
	case em_Content_Type_LeftAddress: case em_Content_Type_RightAddress:
		return static_cast<DWORD>(CCharacter::ReadDWORD(static_cast<UINT_PTR>(dwNumberLeft)));
	case em_Content_Type_Symbol_Add:
		return dwNumberLeft + dwNumberRight;
	case em_Content_Type_Symbol_Sub:
		return dwNumberLeft - dwNumberRight;
	case em_Content_Type_Symbol_Mul:
		return dwNumberLeft * dwNumberRight;
	case em_Content_Type_Symbol_Div:
		return dwNumberLeft / dwNumberRight;
	case em_Content_Type_Symbol_Mod:
		return dwNumberLeft % dwNumberRight;
	case em_Content_Type_Symbol_ExOr:
		return dwNumberLeft ^ dwNumberRight;
	case em_Content_Type_Symbol_InOr:
		return dwNumberLeft | dwNumberRight;
	case em_Content_Type_Symbol_Comp:
		return ~dwNumberLeft;
	case em_Content_Type_Symbol_And:
		return dwNumberLeft & dwNumberRight;
	case em_Content_Type_Symbol_LeftShift:
		return dwNumberLeft << dwNumberRight;
	case em_Content_Type_Symbol_RightShift:
		return dwNumberLeft >> dwNumberRight;
	default:
		break;
	}
	return 0;
}

BOOL CLExpressionCalc::CalcResult_By_Rpn(_Out_opt_ DWORD* pdwResult) throw()
{
	std::stack<DWORD> StackResult;
	while (!Rpn.empty())
	{
		auto& itm = Rpn.top();
		if (itm.emContentType == em_Content_Type::em_Content_Type_Number)
		{
			StackResult.push(itm.GetHex());
			Rpn.pop();
			continue;
		}

		if (StackResult.size() == 0)
		{
			LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"calc Number UnExist Left Value in Symbol '%s'", itm.wsText.c_str());
			return FALSE;
		}

		DWORD NumberRight = StackResult.top();
		StackResult.pop();


		DWORD nResult = NULL;
		DWORD NumberLeft = NULL;
		if (itm.emContentType == em_Content_Type::em_Content_Type_LeftAddress)
		{
			nResult = CalcResult_By_Parm(NumberRight, NULL, itm.emContentType);
			LOG_C(CLog::em_Log_Type::em_Log_Type_Custome, L"[0x%08X]=0x%X", NumberRight, nResult);
		}
		else if (itm.emContentType == em_Content_Type::em_Content_Type_Symbol_Comp)
		{
			nResult = CalcResult_By_Parm(NumberRight, NULL, itm.emContentType);
			LOG_C(CLog::em_Log_Type::em_Log_Type_Custome, L"~0x%08X=%08X", NumberRight, nResult);
		}
		else
		{
			if (StackResult.size() == 0)
			{
				LOG_C(CLog::em_Log_Type::em_Log_Type_Custome, L"calc Number UnExist Right Value in Symbol '%s' ", itm.wsText.c_str());
				return FALSE;
			}

			NumberLeft = StackResult.top();
			StackResult.pop();

			if (itm.emContentType == em_Content_Type::em_Content_Type_Symbol_Div && NumberRight == 0)
			{
				LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"are u kidding me? did u want to div zero?");
				return FALSE;
			}

			nResult = CalcResult_By_Parm(NumberLeft, NumberRight, itm.emContentType);
			LOG_C(CLog::em_Log_Type::em_Log_Type_Custome, L"0x%08X %s 0x%08X = 0x%08X", NumberLeft, itm.wsText.c_str(), NumberRight, nResult);
		}

		StackResult.push(nResult);
		Rpn.pop();
	}

	if (StackResult.size() == 0)
	{
		LOG_C(CLog::em_Log_Type::em_Log_Type_Exception, L"UnExist Result!");
		return FALSE;
	}
	CLPublic::SetPtr(pdwResult, StackResult.top());
	return TRUE;
}

BOOL CLExpressionCalc::ReadMem_By_Rpn() throw()
{
	DWORD dwResult = 0;
	if (!CalcResult_By_Rpn(&dwResult))
		return FALSE;

	for (UINT i = 0; i < ExpContent.uMemWatchCount; ++i)
		LOG_C(CLog::em_Log_Type::em_Log_Type_Custome, L"0x%08X -> 0x%X = 0x%08X", dwResult + i * 0x4, i * 4, CCharacter::ReadDWORD(dwResult + i * 4));

	return TRUE;
}

BOOL CLExpressionCalc::IsNumber(_In_ CONST std::wstring& wsText) CONST throw()
{
	for (CONST auto& itm : wsText)
	{
		if (!isdigit(itm))
			return FALSE;
	}
	return TRUE;
}