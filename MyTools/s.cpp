#include <iostream>
#include <Shlwapi.h>
#include <thread>
#include "CLScript.h"
#include "CLStackTrace.h"
#include "CLLog.h"
#include "ClassInstance.h"
#include "Character.h"
#include "CLFile.h"
#include "CLPublic.h"
#include "CLThread.h"
#include "CLExpression.h"
#include "Log.h"
#include "CLScript.h"
#define _SELF L"s.cpp"
using namespace std;

class TestScript : public CLScript
{
public:
	TestScript() = default;
	~TestScript() = default;

	virtual VOID SetSciptPtr()
	{

	}
private:

};



int main()
{
	TestScript Script;
	Script.Read(L"C:\\1.inf");

	std::wstring wsErrText;
	if (Script.Check(wsErrText))
	{
	}
	return 0;
}
