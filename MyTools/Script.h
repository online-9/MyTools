#ifndef __MYTOOLS_CHARACTER_SCRIPT_SCRIPT_H__
#define __MYTOOLS_CHARACTER_SCRIPT_SCRIPT_H__

#include "Character.h"
#include <stack>

namespace MyTools
{
	class CScript
	{
	protected:
		enum em_Script_CodeType
		{
			em_Script_CodeType_None,			// Empty
			em_Script_CodeType_DefMethod,		// function AAA()
			em_Script_CodeType_Method,			// SetSpeed(3.0)
			em_Script_CodeType_LeftBrace,		// {
			em_Script_CodeType_RightBrace,		// }
			em_Script_CodeType_If,				// if(Condition,1==1,CALLBACK)
			em_Script_CodeType_While,			// while(Condition,1==1,CALLBACK)
			em_Script_CodeType_Comment			// // 
		};

		struct Script_Code_Type
		{
			em_Script_CodeType emScriptCodeType;

			Script_Code_Type() : emScriptCodeType(em_Script_CodeType_None)
			{
				
			}

			Script_Code_Type(_In_ em_Script_CodeType emScriptCodeType_) : emScriptCodeType(emScriptCodeType_)
			{

			}
		};

		struct Script_Code_MethodParameter
		{
			std::wstring					wsValue;

			DWORD ConvertDWORD() CONST
			{
				return static_cast<DWORD>(wcstol(wsValue.c_str(), NULL, 16));
			}
			DWORD ConvertDWORD_By_Dec() CONST
			{
				return static_cast<DWORD>(_wtoi(wsValue.c_str()));
			}
			float  ConvertFLOAT() CONST
			{
				return static_cast<float>(_wtof(wsValue.c_str()));
			}
			CONST std::wstring& GetString() CONST
			{
				return wsValue;
			}
			Script_Code_MethodParameter()
			{
				wsValue = L"";
			}
			Script_Code_MethodParameter(CONST std::wstring& wsValue_)
			{
				wsValue = wsValue_;
			}
		};

		struct Script_Code_Method : public Script_Code_Type
		{
			std::wstring				wsMethodName;
			std::vector<Script_Code_MethodParameter> VecParm;

			Script_Code_Method()
			{
				emScriptCodeType = em_Script_CodeType::em_Script_CodeType_Method;
			}
		};

		struct Script_Code_If : public Script_Code_Type
		{
			std::wstring				wsCondition;
			std::vector<Script_Code_MethodParameter>	VecValue;
			std::wstring				wsMethodName;

			Script_Code_If()
			{
				emScriptCodeType = em_Script_CodeType::em_Script_CodeType_If;
			}
		};

		struct Script_Code
		{
			Script_Code_Type*	pCode;
			ULONG				ulCodeHash;
			Script_Code()
			{
				pCode = nullptr;
				ulCodeHash = CCharacter::GetRand_For_DWORD();
			}
		};

		struct Script_Code_DefMethod
		{
			std::vector<Script_Code>		VecScriptCode;
			std::wstring					wsMethodName;
		};

		typedef std::function<BOOL()> DefScriptMethod;
		struct Script_Code_Custome_Method
		{
			std::wstring				wsMethodName;
			DefScriptMethod				MethodPtr;

			Script_Code_Custome_Method(_In_ CONST std::wstring& wsMethodName_, _In_ CONST DefScriptMethod& pMethodPtr) : wsMethodName(wsMethodName_), MethodPtr(pMethodPtr)
			{

			}
		};

		struct Script_Current_Content
		{
			std::wstring	wsDefMethodName;
			ULONG			ulCodeHash;
		};

	public:
		CScript();
		~CScript();

		// Set Script Content
		enum em_ReadScript_Type { em_ReadScript_Type_FilePath, em_ReadScript_Type_Text };
		BOOL Read(_In_ em_ReadScript_Type emReadScriptType, _In_ CONST std::wstring& wsContent);

		// 
		BOOL AddCustomeFunAddr(_In_ CONST Script_Code_Custome_Method& CustomeMethod);

		// 
		BOOL Excute(_In_ CONST std::wstring& wsMethodName);

	protected:
		BOOL ReadScriptFile(_In_ CONST std::wstring& wsFilePath, _Out_ std::vector<std::wstring>& VecFileContent);

		BOOL ReadScriptContent(_In_ CONST std::wstring& wsFileContent, _Out_ std::vector<std::wstring>& VecFileContent);

		em_Script_CodeType GetScriptCodeType(_In_ CONST std::wstring& wsContent) CONST;

		BOOL FillDefMethod(_In_ UINT& Index, _In_ CONST std::vector<std::wstring>& VecFileContent, _Out_ Script_Code_DefMethod& DefMethodContent_) CONST;

		BOOL FileContentToCode(_In_ CONST std::vector<std::wstring>& VecFileContent);

		BOOL Release();

		VOID PrintLog(_In_ int nLine, _In_ LPCWSTR pwszFormat, ...) CONST;

		BOOL ExcuteDefMethod(_In_ CONST std::wstring& wsMethodName, _In_ ULONG ulHash);

		BOOL ExcuteScriptCode(_In_ CONST std::wstring& wsMethodName, _In_ CONST Script_Code& ScriptCode_);

		BOOL ExcuteCustMethod(_In_ CONST std::wstring&, _In_ CONST Script_Code_Method* pCodeMethod);

		VOID AddExcuteQueue(_In_ CONST std::wstring& wsMethodName, _In_ ULONG ulHash = NULL);

		VOID RemoveQueue();

		CONST Script_Code_DefMethod* ExistDefMethod(_In_ CONST std::wstring& wsMethodName) CONST;

		CONST Script_Code_Custome_Method* ExistCustMethod(_In_ CONST std::wstring& wsMethodName) CONST;
	private:
		BOOL AnalysisCode_DefMethod(_In_ CONST std::wstring& wsContent, _Out_ Script_Code_DefMethod& DefMethodContent_) CONST;

		BOOL AnalysisCode_Brace(_In_ em_Script_CodeType emScriptCodeType_, _Out_ Script_Code_DefMethod& DefMethodContent_) CONST;

		BOOL AnalysisCode_If(_In_ em_Script_CodeType emScriptCodeType_, _In_ CONST std::wstring& wsContent, _Out_ Script_Code_DefMethod& DefMethodContent_) CONST;

		BOOL AnalysisCode_Method(_In_ CONST std::wstring& wsContent, _Out_ Script_Code_DefMethod& DefMethodContent_) CONST;
	protected:
		std::vector<Script_Code_DefMethod> _VecCode;
		std::vector<Script_Code_Custome_Method> _VecCustMethod;
		std::stack<Script_Current_Content> _QueCurrentPos;
		std::function<BOOL(VOID)> _fnExceptionPtr;
		std::function<VOID(CONST std::wstring&, int nLine, CONST std::wstring&)> _fnLogPtr;
		std::function<BOOL(CONST Script_Code_If&)> _fnIfPtr;
		std::function<BOOL(CONST Script_Code_If&)> _fnWhilePtr;
		CONST Script_Code_Method* _pCurrentMethodContent;
		CONST Script_Code* _pCurrentScriptCode;
	};

}




#endif // !__MYTOOLS_CHARACTER_SCRIPT_SCRIPT_H__
