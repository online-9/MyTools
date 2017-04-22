#ifndef __MYTOOLS_RELFEX_RELFEXSTRUCT_H__
#define __MYTOOLS_RELFEX_RELFEXSTRUCT_H__

#include "ToolsPublic.h"
#include <map>

class CRelfexStruct
{
public:
	CRelfexStruct();
	virtual ~CRelfexStruct() = default;

	VOID SetErrorPtr(_In_ std::function<VOID(CONST std::wstring&)> fnErrorPtr);

	BOOL Register(_In_ CONST std::wstring& wsText, _In_ LPCVOID lpAddr);

	template<typename T>
	T GetPtr(_In_ CONST std::wstring& wsText)
	{
		return reinterpret_cast<T>(GetPtr_By_Text(wsText));
	}
private:
	LPCVOID GetPtr_By_Text(_In_ CONST std::wstring& wsText);

	VOID PrintError(_In_ CONST std::wstring& wsErrText) CONST;
private:
	std::function<VOID(CONST std::wstring&)> _fnErrorPtr;
	std::map<std::wstring, LPCVOID> _MapResPtrText;
};



#endif // !__MYTOOLS_RELFEX_RELFEXSTRUCT_H__
