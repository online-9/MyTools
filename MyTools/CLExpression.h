#ifndef __MYTOOLS_CHARACTER_CLEXPRESSION_H__
#define __MYTOOLS_CHARACTER_CLEXPRESSION_H__

#include "ToolsPublic.h"

struct ExpressionFunPtr
{
	std::function<VOID(CONST std::vector<std::wstring>&)> fnPtr;
	std::wstring wsFunName;
};

struct ExprStructBase
{
	virtual VOID Print() = NULL;
};

enum em_Cmd_Type;
class CExprFunBase
{
public:
	CExprFunBase() = default;
	virtual ~CExprFunBase() = default;

	virtual std::vector<ExpressionFunPtr>& GetVec() = NULL;

	virtual VOID Release() = NULL;

	virtual VOID Help(CONST std::vector<std::wstring>&) = NULL;
};

class CLExpression
{
public:
	CLExpression() = default;
	~CLExpression() = default;

	UINT Push(_In_ std::function<VOID(CONST std::vector<std::wstring>&)> fnPtr, _In_ CONST std::wstring& wsFunName) throw();

	VOID SetVecExprFunPtr(_In_ CONST std::vector<ExpressionFunPtr>& ParmVecFunPtr) throw();

	BOOL Run(_In_ CONST std::wstring& wsText) throw();
private:
	std::vector<ExpressionFunPtr> VecFunPtr;
};

#endif