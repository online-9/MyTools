#ifndef __MYTOOLS_ERROR_CLERRMSG_H__
#define __MYTOOLS_ERROR_CLERRMSG_H__

#include "Character.h"

#define _SetErrMsg(Msg,...) CLErrMsg(__FUNCTIONW__, __LINE__,Msg,__VA_ARGS__)
class CLErrMsg
{
public:
	struct ErrMsg { DWORD dwThreadId; std::wstring wsMsg; };
public:
	CLErrMsg(_In_ LPCWSTR pwszFunName, int nLine, LPCWSTR pwszFormat, ...);
	~CLErrMsg();

	static BOOL ExistErrMsg();
	static BOOL GetCurThreadErrMsg(_Out_ std::vector<std::wstring>& vErrMsg);
private:
	static std::vector<ErrMsg> vlst;
};

#endif