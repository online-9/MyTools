#ifndef __MYTOOLS_PUBLIC_CLASSINSTANCE_H__
#define __MYTOOLS_PUBLIC_CLASSINSTANCE_H__

#include <map>
#include "CLLock.h"

template<class T = DWORD>
class CClassInstance
{
public:
	CClassInstance() = default;
	~CClassInstance() = default;

	CClassInstance(CONST CClassInstance&) = delete;

	static T& GetInstance()
	{
		return GetStaticVariable<T>();
	}

	void operator = (CONST CClassInstance&) = delete;

	template<typename Var>
	static Var& GetStaticVariable()
	{
		static Var Var_;
		return Var_;
	}


	

private:
	
};

template<typename ValueType>
class CVariable
{
public:
	CVariable() = default;
	virtual ~CVariable() = default;

	CVariable(CONST CVariable&) = delete;
	void operator = (CONST CVariable&) = delete;

	// Get Ref Value
	template<class em_Variable_Id>
	ValueType& GetRefValue_By_Id(_In_ em_Variable_Id emVarId) CONST throw()
	{
		static std::map<em_Variable_Id, ValueType> VarMap;
		static CLLock Lock(L"CVariable::GetRefValue_By_Id");

		ValueType* pValue = nullptr;
		Lock.Access([&emVarId, &pValue]
		{
			auto& itr = VarMap.find(emVarId);
			if (itr != VarMap.end())
				pValue = &itr->second;
			else
				VarMap.insert(std::map<em_Variable_Id, ValueType>::value_type(emVarId, NULL));
		});

		return pValue != nullptr ? *pValue : GetRefValue_By_Id(emVarId);
	}

	// Set New Value and Return Old Value
	template<class em_Variable_Id>
	ValueType SetValueAndGetOldValue_By_Id(_In_ em_Variable_Id emVarId, _In_ ValueType NewValue) CONST throw()
	{
		auto& OldValue = GetRefValue_By_Id(emVarId);
		::swap(OldValue, NewValue);
		// New Value = OldValue, because swap...
		return NewValue;
	}

	// Worker when Value Equal
	template<class em_Variable_Id>
	BOOL Action_For_EqualValue_By_Id(_In_ em_Variable_Id emVarId, _In_ ValueType EqualValue, _In_ std::function<VOID(VOID)> Worker) CONST throw()
	{
		if (GetRefValue_By_Id(emVarId) == EqualValue)
		{
			Worker();
			return TRUE;
		}

		return FALSE;
	}

private:

};




#endif // !__MYTOOLS_PUBLIC_CLASSINSTANCE_H__
