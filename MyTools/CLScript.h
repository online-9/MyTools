#ifndef __MYTOOLS_CHARACTER_CLSCRIPT_H__
#define __MYTOOLS_CHARACTER_CLSCRIPT_H__

#include "ToolsPublic.h"

#define CL_SCRIPT_MAXLEN_CODELINE	64

typedef enum _em_CL_Script_Code_Type
{
	em_CL_Script_Code_Type_None,				// 'Empty'
	em_CL_Script_Code_Type_DefineFun,			// function AAA()
	em_CL_Script_Code_Type_Left_Brace,			// {
	em_CL_Script_Code_Type_Right_Brace,			// }
	em_CL_Script_Code_Type_Function,			// AAA()
	em_CL_Script_Code_Type_If,					// if
	em_CL_Script_Code_Type_ElseIf,				// else if
	em_CL_Script_Code_Type_Else,				// else
	em_CL_Script_Code_Type_While,				// while
	em_CL_Script_Code_Type_Contiune,			// contiune
	em_CL_Script_Code_Type_Break,				// break
	em_CL_Script_Code_Type_Note					// Note
}em_CL_Script_Code_Type;

typedef struct _CL_Script_TranCode_FunParm     // (3, AAAA, 5)
{
	std::wstring wsParm;
	DWORD ConvertDWORD()
	{
		return static_cast<DWORD>(wcstol(wsParm.c_str(), NULL, 16));
	}
	DWORD ConvertDWORD_By_Dec()
	{
		return static_cast<DWORD>(_wtoi(wsParm.c_str()));
	}
	float  ConvertFLOAT()
	{
		return static_cast<float>(_wtof(wsParm.c_str()));
	}
	CONST std::wstring& GetString()
	{
		return wsParm;
	}
	_CL_Script_TranCode_FunParm()
	{
		wsParm = L"";
	}
	_CL_Script_TranCode_FunParm(CONST std::wstring& wsParm_)
	{
		wsParm = wsParm_;
	}
}CL_Script_TranCode_FunParm;

typedef struct _CL_Script_TranCode_DefineFun    // function AAA()
{
	std::wstring wsFunName;
}CL_Script_TranCode_DefineFun;

typedef struct _CL_Script_TranCode_Function
{
	std::wstring								wsFunName;
	std::vector<CL_Script_TranCode_FunParm>	ParmList;
	_CL_Script_TranCode_Function()
	{
		wsFunName = L"";
	}
}CL_Script_TranCode_Function;

typedef struct _CL_Script_TranCode
{
	em_CL_Script_Code_Type	emCodeType;
	LPVOID					pStruct;
	_CL_Script_TranCode()
	{
		emCodeType = em_CL_Script_Code_Type_None;
		pStruct = nullptr;
	}
}CL_Script_TranCode;

class CLScript;
typedef BOOL(WINAPI * CLScriptFun)(LPVOID lpCustomeFunParm, std::vector<CL_Script_TranCode_FunParm>& vlst);

typedef struct _CL_Script_CustomeFunction
{
	std::wstring		wsFunName;
	CLScriptFun	pFunAddr;

	BOOL Init(_In_ CONST std::wstring& wsFunName_, _In_ CLScriptFun pFunAddr_)
	{
		wsFunName = wsFunName_;
		pFunAddr = pFunAddr_;
		return TRUE;
	}
	_CL_Script_CustomeFunction(_In_ CONST std::wstring& wsFunName_, _In_ CONST CLScriptFun& pFunAddr_)
	{
		Init(wsFunName_, pFunAddr_);
	}
	_CL_Script_CustomeFunction()
	{
		Init(L"", (CLScriptFun)nullptr);
	}
}CL_Script_CustomeFunction;



class CLScript
{
public:
	CLScript();
	~CLScript();

	// 
	BOOL Read(__in LPCWSTR pwszScriptPath);
	BOOL Read(__in std::shared_ptr<WCHAR>& pScriptStr);
	
	// Add FunAddr to List
	BOOL AddCustomeFunAddr(_In_ CONST std::wstring& wsFunName, __in CLScriptFun pScriptFunAddr);

	// 
	BOOL Excute(_In_ CONST std::wstring& wsName);

	CONST std::vector<CL_Script_CustomeFunction>& GetCustomeFunList() CONST;
	CONST std::vector<CL_Script_TranCode>&	GetSourceList() CONST;

	// Exist FunName in List
	BOOL IsExistScriptFunAddrList(_In_ CONST std::wstring& wsFunname, _Out_opt_ CL_Script_TranCode_DefineFun& CL_Script_TranCode_DefineFun_);
	BOOL IsExistCustomeFunAddrList(_In_ CONST std::wstring& wsFunname, _Out_opt_ CL_Script_CustomeFunction& CL_Script_CustomeFunction_);

	BOOL Check(_Out_opt_ std::wstring& wsErrText);
protected:
	virtual BOOL IsExcuteNext();
protected:
	virtual VOID SetSciptPtr();

	// Read *.inf to list
	BOOL ReadScriptFile(_In_ CONST std::wstring& wsPath);

	// Read Content to list
	BOOL ReadScriptContent(__in LPCSTR pszScriptContent);
	BOOL ReadScriptContent(_In_ CONST std::wstring& wsContent);

	// Analysis Code
	BOOL AnalysisSourceCode();

	// Excute Content
	BOOL ExcuteContent(__in std::vector<CL_Script_TranCode>::iterator itr, __in const std::vector<CL_Script_TranCode>::iterator EndItr);

	em_CL_Script_Code_Type GetSourceType(_In_ CONST std::wstring wsText);

	// -- function AAA()
	BOOL AnalysisCode_DefineFun(_In_ CONST std::wstring& wsText, __out LPVOID& pAddr);

	// -- SetSpeed(10.5, 1, 2)
	BOOL AnalysisCode_ExcuteFun(_In_ CONST std::wstring& wsText, __out LPVOID& pAddr);

	// -- if(IsDead())
	//BOOL AnalsisCode_If(_In_ cwstring& wsText, __out LPVOID& pAddr);

	// 
	BOOL ExcuteCode_If(__in std::vector<CL_Script_TranCode>::iterator CodeItr, __in BOOL bResult);

	// 
	std::vector<CL_Script_TranCode>::iterator GetItr_By_CodeType(__in const std::vector<CL_Script_TranCode>::iterator Startitr, em_CL_Script_Code_Type emCodeType);

	// Release Res
	BOOL Release();

	// Excute Function( Script Function && Cusome Function)
	BOOL ExcuteFunction(_In_ CONST std::wstring& wsName, _In_ std::vector<CL_Script_TranCode_FunParm>& vlst);

public:
	std::vector<std::wstring>						SourceCodeList;
	std::vector<CL_Script_TranCode>					ScriptCodeList;
	std::vector<CL_Script_CustomeFunction>			CustomeFunAddrList;
	BOOL											bShowDebugMsg;
public:
	BOOL											bExit;
	LPVOID											lpCustomeFunParm;
};



#endif