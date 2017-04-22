#include "stdafx.h"
#include "RelfexStruct.h"
#include "Character.h"

CRelfexStruct::CRelfexStruct() : _fnErrorPtr(nullptr)
{
	
}

VOID CRelfexStruct::SetErrorPtr(_In_ std::function<VOID(CONST std::wstring&)> fnErrorPtr)
{
	_fnErrorPtr = fnErrorPtr;
}

BOOL CRelfexStruct::Register(_In_ CONST std::wstring& wsText, _In_ CONST LPVOID lpAddr)
{
	if (_MapResPtrText.find(wsText) != _MapResPtrText.end())
	{
		std::wstring wsErrText;
		PrintError(CCharacter::FormatText(wsErrText, L"Repeat Register Struct Text:[%s]", wsText.c_str()));
		return FALSE;
	}

	_MapResPtrText.insert(std::make_pair(wsText, lpAddr));
	return TRUE;
}

LPCVOID CRelfexStruct::GetPtr_By_Text(_In_ CONST std::wstring& wsText)
{
	auto itr = _MapResPtrText.find(wsText);
	if (itr == _MapResPtrText.end())
	{
		std::wstring wsErrText;
		PrintError(CCharacter::FormatText(wsErrText, L"UnExist Struct Text:[%s]", wsText.c_str()));
		return nullptr;
	}
	return itr->second;
}

VOID CRelfexStruct::PrintError(_In_ CONST std::wstring& wsErrText) CONST
{
	if (_fnErrorPtr != nullptr)
		_fnErrorPtr(wsErrText);
}
