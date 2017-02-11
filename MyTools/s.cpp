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
#include "CLResManager.h"
#include "CLPic.h"
#define _SELF L"s.cpp"
using namespace std;



int main()
{
	CLog::GetInstance().SetClientName(L"MyTools", L"C:\\TempLog", FALSE, 20 * 1024 * 1024);
	HWND hWnd = ::FindWindowW(L"LaunchUnrealUWindowsClient", NULL);
	if (hWnd != NULL)
	{
		CLPic::GetInstance().ScreenShot(hWnd, L"C:\\1.bmp");
	}
	return 0;
}
