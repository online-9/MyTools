#include "stdafx.h"
#include "CLPublic.h"
#include <Shlwapi.h>
#include "Character.h"
#include "CLThread.h"
#define _SELF L"CLPublic.cpp"
#pragma comment(lib,"Shlwapi.lib")
#pragma warning(disable: 4996) 
CLPublic::CLPublic()
{
}

CLPublic::~CLPublic()
{
}
/************************************************************************/
/* Global variables                                                                     */
/************************************************************************/
ULONG CLPublic::s_Crc32Table[256] = {0};

//////////////////////////////////////////////////////////////////////////
VOID CLPublic::GetCRC32Table()
{
	static BOOL s_Init = FALSE;
	//if (s_Init)
		//return;

	unsigned long Crc;
	for (int i = 0; i < 256; ++i)
	{
		Crc = i;
		for (int j = 0; j < 8; ++j)
		{
			if (Crc & 1)
			{
				Crc = (Crc >> 1) ^ 0xEDB88320;
			}
			else
			{
				Crc >>= 1;
			}
		}
		s_Crc32Table[i] = Crc;
	}
	s_Init = TRUE;
}

VOID CLPublic::GetCRC32(IN LPSTR inStr, IN UINT uSize, OUT LPSTR OutStr)
{
	GetCRC32Table();
	ULONG crc = 0xFFFFFFFF;
	unsigned char* buffer;
	buffer = (unsigned char*)inStr;
	//int len = strlen(inStr);
	for (int i = 0; i < (int)uSize; ++i)
	{
		crc = (crc >> 8) ^ s_Crc32Table[(crc & 0xFF) ^ *buffer++];
	}
	char value[10] = { 0 };
	//_itoa(crc ^ 0xFFFFFFFF, value, 16);
	sprintf_s(value, _countof(value) - 1, "%X", crc ^ 0xFFFFFFFF);
	for (int i = 0; i < 10; ++i)
	{
		value[i] = static_cast<char>(toupper(value[i]));
	}
	//strcpy(inStr, value);
	CCharacter::strcpy_my(OutStr, value, strlen(value));
}

VOID CLPublic::GetCRC32_DWORD(IN LPCSTR inStr, IN UINT uSize, OUT DWORD& OutValue)
{
	CHAR szCrc[9];

	GetCRC32Table();
	ULONG crc = 0xFFFFFFFF;
	unsigned char* buffer;
	buffer = (unsigned char*)inStr;
	//int len = strlen(inStr);
	for (int i = 0; i < (int)uSize; ++i)
	{
		crc = (crc >> 8) ^ s_Crc32Table[(crc & 0xFF) ^ *buffer++];
	}
	char value[10] = { 0 };
	//_itoa(crc ^ 0xFFFFFFFF, value, 16);
	sprintf_s(value, _countof(value) - 1, "%X", crc ^ 0xFFFFFFFF);
	for (int i = 0; i < 10; ++i)
	{
		value[i] = static_cast<char>(toupper(value[i]));
	}
	//strcpy(inStr, value);
	CCharacter::strcpy_my(szCrc, value, strlen(value));
	szCrc[8] = '\0';
	OutValue = (DWORD)strtoll(szCrc, NULL, 16);
}

DWORD CLPublic::GetCRC32_DWORD(IN LPCSTR inStr, IN UINT uSize)
{
	//if (strlen(inStr) == 0)
		//return 0;

	DWORD dwValue = 0;
	GetCRC32_DWORD(inStr, uSize, dwValue);
	return dwValue;
}

DWORD CLPublic::GetCRC32_DWORD(IN LPCWSTR inStr)
{
	if (wcslen(inStr) == 0)
		return 0x0;

	std::string str = CCharacter::UnicodeToASCII(std::wstring(inStr));

	DWORD dwCrc = 0;
	GetCRC32_DWORD(str.c_str(), static_cast<UINT>(str.length()), dwCrc);
	return dwCrc;
}

VOID CLPublic::GetCRC32_Long(IN LPSTR inStr, IN LONG lgSize, OUT LPSTR OutStr)
{
	GetCRC32Table();
	ULONG crc = 0xFFFFFFFF;
	unsigned char* buffer;
	buffer = (unsigned char*)inStr;
	//int len = strlen(inStr);
	for (LONG i = 0; i < lgSize; ++i)
	{
		crc = (crc >> 8) ^ s_Crc32Table[(crc & 0xFF) ^ *buffer++];
	}
	char value[10] = { 0 };
	//_itoa(crc ^ 0xFFFFFFFF, value, 16);
	sprintf_s(value, _countof(value) - 1, "%X", crc ^ 0xFFFFFFFF);
	for (int i = 0; i < 10; ++i)
	{
		value[i] = static_cast<char>(toupper(value[i]));
	}
	//strcpy(inStr, value);
	CCharacter::strcpy_my(OutStr, value, strlen(value));
}

BOOL CLPublic::SimulationKey(DWORD dwASCII, HWND hWnd, DWORD dwTime)
{
	char ch = (char)dwASCII;
	SHORT tmp = VkKeyScanW(ch);//扫描虚拟码
	WPARAM wParam = tmp & 0xFF;
	LPARAM lParam = 1;
	lParam += static_cast<UINT_PTR>((wParam, MAPVK_VK_TO_VSC) << 16);//获取扫描码并且赋值Lparm

	PostMessageW(hWnd, WM_KEYDOWN, wParam, lParam);
	Sleep(dwTime);
	lParam += 1 << 30;
	lParam += 1 << 31;
	PostMessageW(hWnd, WM_KEYUP, wParam, lParam);

	return TRUE;
}

DWORD CLPublic::GetVirKey(DWORD dwASCII)
{
	char ch = (char)dwASCII;
	SHORT tmp = VkKeyScanW(ch);//扫描虚拟码
	DWORD wParam = tmp & 0xFF;
	DWORD lParam = 1;
	lParam += MapVirtualKey(wParam, MAPVK_VK_TO_VSC) << 16;//获取扫描码并且赋值Lparm
	return lParam;
}

BOOL CLPublic::SimulationMouse(int x, int y)
{
	for (int i = 0; i < 1; ++i)
	{
		// 设置鼠标
		::SetCursorPos(x, y);
		mouse_event(MOUSEEVENTF_LEFTDOWN, x, y, NULL, NULL);
		mouse_event(MOUSEEVENTF_LEFTUP, x, y, NULL, NULL);
	}
	return TRUE;
}

BOOL CLPublic::SimulationRightMouse(int x, int y)
{
	for (int i = 0; i < 1; ++i)
	{
		// 设置鼠标
		::SetCursorPos(x, y);
		mouse_event(MOUSEEVENTF_RIGHTDOWN, x, y, NULL, NULL);
		mouse_event(MOUSEEVENTF_RIGHTUP, x, y, NULL, NULL);
	}
	return TRUE;
}

BOOL CLPublic::SendAscii(wchar_t data, BOOL shift)
{
	INPUT input[2];
	memset(input, 0, 2 * sizeof(INPUT));

	if (shift)
	{
		input[0].type = INPUT_KEYBOARD;
		input[0].ki.wVk = VK_SHIFT;
		SendInput(1, input, sizeof(INPUT));
	}
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = data;
	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = data;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(2, input, sizeof(INPUT));
	if (shift)
	{
		input[0].type = INPUT_KEYBOARD;
		input[0].ki.wVk = VK_SHIFT;
		input[0].ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, input, sizeof(INPUT));
	}
	return TRUE;
}

BOOL CLPublic::SendUnicode(wchar_t data)
{
	INPUT input[2];
	memset(input, 0, 2 * sizeof(INPUT));

	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = 0;
	input[0].ki.wScan = data;
	input[0].ki.dwFlags = 0x4;//KEYEVENTF_UNICODE;

	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = 0;
	input[1].ki.wScan = data;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP | 0x4;//KEYEVENTF_UNICODE;

	SendInput(2, input, sizeof(INPUT));
	return TRUE;
}

BOOL CLPublic::SendKey(WORD wVk)
{
	INPUT input[2];
	memset(input, 0, 2 * sizeof(INPUT));

	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = wVk;

	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = wVk;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(2, input, sizeof(INPUT));
	return TRUE;
}

BOOL CLPublic::SendKeys(const wchar_t* data)
{
	short vk;
	BOOL shift;
	size_t len = wcslen(data);
	for (int i = 0; i < len; i++)
	{
		if (data[i] >= 0 && data[i] < 256) //ascii字符
		{
			vk = VkKeyScanW(data[i]);
			if (vk == -1)
			{
				SendUnicode(data[i]);
			}
			else
			{
				if (vk < 0)
				{
					vk = ~vk + 0x1;
				}

				shift = vk >> 8 & 0x1;

				if (GetKeyState(VK_CAPITAL) & 0x1)
				{
					if (data[i] >= 'a' && data[i] <= 'z' || data[i] >= 'A' && data[i] <= 'Z')
					{
						shift = !shift;
					}
				}
				SendAscii(vk & 0xFF, shift);
			}
		}
		else //unicode字符
		{
			SendUnicode(data[i]);
		}
	}
	return TRUE;
}

BOOL CLPublic::SendKeys(const char* data)
{
	wchar_t *  udata;
	int  udataLen = ::MultiByteToWideChar(CP_ACP, 0, data, -1, NULL, 0);
	udata = new  wchar_t[udataLen + 1];
	memset(udata, 0, (udataLen + 1)*sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP, 0, data, -1, udata, udataLen);
	SendKeys(udata);
	delete[]udata;
	return TRUE;
}

BOOL CLPublic::ForceExit()
{
	
#ifdef _WIN64
	
#else
	CONTEXT ct = { 0 };
	DWORD dwMainThreadId = CLThread::GetMainThreadId();
	if (dwMainThreadId == NULL)
		return FALSE;

	HANDLE hThread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, dwMainThreadId);
	if (hThread == NULL)
		return FALSE;

	ct.ContextFlags = CONTEXT_ALL;
	GetThreadContext(hThread, &ct);
	ct.Eip = NULL;//强制崩溃
	::SetThreadContext(hThread, &ct);
#endif
	
	
	return TRUE;
}

BOOL CLPublic::FileExit(LPCWSTR pwchPath)
{
	return PathFileExistsW(pwchPath);
}

BOOL CLPublic::FileExist(_In_ CONST std::wstring& wsPath)
{
	return FileExit(wsPath.c_str());
}

BOOL CLPublic::FileExistA(LPCSTR pchPath)
{
	return PathFileExistsA(pchPath);
}

BOOL CLPublic::CopyText2Clipboard(LPCWSTR pwchText)
{
	if (!OpenClipboard(NULL))
	{
		return FALSE;
	}
	EmptyClipboard();
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_DDESHARE, sizeof(WCHAR)*MAX_PATH);
	if (hglbCopy == NULL)
	{
		CloseClipboard();
		return FALSE;
	}
	// Lock the handle and copy the text to the buffer.
	CHAR *lptstrCopy = (CHAR*)GlobalLock(hglbCopy);
	WideCharToMultiByte(CP_OEMCP, NULL, pwchText, -1,  lptstrCopy, MAX_PATH, NULL, NULL);
	GlobalUnlock(hglbCopy);
	SetClipboardData(CF_TEXT, hglbCopy);
	CloseClipboard();
	return TRUE;
}

BOOL CLPublic::FormatTime(IN ULONGLONG ulResult, OUT LPWSTR pwchText, _In_ UINT uTextSize)
{
	ULONGLONG ulDay = 0;
	ULONGLONG ulHour = 0;
	ULONGLONG ulMin = 0;
	ULONGLONG ulSecond = 0;

	//convert MilliSecond to Second
	ulResult /= 1000;
	
	if (ulResult >= 60 * 60 * 24)//Separate Day
	{
		ulDay = ulResult / (60 * 60 * 24);
		ulResult -= ulDay * (60 * 60 * 24);
	}
	if (ulResult >= 60 * 60)//Separate Hour
	{
		ulHour = ulResult / (60 * 60);
		ulResult -= ulHour * (60 * 60);
	}
	if (ulResult >= 60)//Separate Minunte
	{
		ulMin = ulResult / 60;
		ulResult -= ulMin * 60;
	}

	//Get Last Second
	ulSecond = ulResult;

	//format to Text
	swprintf_s(pwchText, uTextSize, L"%I64dDay %I64dHour %I64dMin %I64dSec", ulDay, ulHour, ulMin, ulSecond);
	return TRUE;
}

DWORD CLPublic::GetSystemVerson()
{
	SYSTEM_INFO info;
	OSVERSIONINFOEX os;

	os.dwOSVersionInfoSize = sizeof(os);
	GetSystemInfo(&info);
	if (GetVersionExW((OSVERSIONINFO *)&os))
	{
		switch (os.dwMajorVersion)
		{                        //判断主版本号 
		case 4:
			switch (os.dwMinorVersion)
			{                //判断次版本号 
			case 0:
				if (os.dwPlatformId == VER_PLATFORM_WIN32_NT)
					return WINDOW_SYSTEM_NT4;
				else if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
					return WINDOW_SYSTEM_95;
			case 10:
				return WINDOW_SYSTEM_98;
			case 90:
				return WINDOW_SYSTEM_ME;
			}
			break;
		case 5:
			switch (os.dwMinorVersion)
			{               //再比较dwMinorVersion的值 
			case 0:
				return WINDOW_SYSTEM_2000;
			case 1:
				return WINDOW_SYSTEM_XP;
			case 2:
				if (os.wProductType == VER_NT_WORKSTATION && info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
					return WINDOW_SYSTEM_XP_64;
				else if (GetSystemMetrics(SM_SERVERR2) == 0)
					return WINDOW_SYSTEM_2003;
				else if (GetSystemMetrics(SM_SERVERR2) != 0)
					return WINDOW_SYSTEM_2003_R2;
			}
			break;
		case 6:
			switch (os.dwMinorVersion)
			{
			case 0:
				if (os.wProductType == VER_NT_WORKSTATION)/*VER_NT_WORKSTATION是桌面系统 */
					return WINDOW_SYSTEM_Vista;
				else
					return WINDOW_SYSTEM_2008;
			case 1:
				if (os.wProductType == VER_NT_WORKSTATION)
				{
					if (info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || info.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_IA64)
						return WINDOW_SYSTEM_WIN7_64;
					else
						return WINDOW_SYSTEM_WIN7_32;
				}
				else
				{
					return WINDOW_SYSTEM_2008_R2;
				}
			}
			break;
		default:
			return WINDOW_SYSTEM_UnKnown;
		}
	}


	return WINDOW_SYSTEM_UnKnown;
}

LPVOID CLPublic::GetEIP()
{
#ifdef _WIN64
	return nullptr;
#else
	_asm
	{
		CALL lbCALL
		lbCALL :
		POP EAX
	}
#endif // _WIN64
}

size_t CLPublic::GetHash(_In_ CONST std::string& str)
{
	size_t result = 0;
	for (auto it = str.cbegin(); it != str.cend(); ++it) {
		result = (result * 131) + *it;
	}
	return result;
}

size_t CLPublic::GetHash(_In_ CONST std::wstring& wstr)
{
	size_t result = 0;
	for (auto it = wstr.cbegin(); it != wstr.cend(); ++it) {
		result = (result * 131) + *it;
	}
	return result;
}


BOOL CLPublic::TimeOut_By_Condition(_In_ DWORD dwMaxTimeOut, _In_ std::function<BOOL(VOID)> fn)
{
	BOOL bRetCode = TRUE;
	auto ulTick = ::GetTickCount64();
	while (static_cast<DWORD>(::GetTickCount64() - ulTick) <= dwMaxTimeOut)
	{
		if (fn())
		{
			bRetCode = FALSE;
			break;
		}
		::Sleep(100);
	}
	return bRetCode;
}