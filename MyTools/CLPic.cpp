#include "stdafx.h"
#include "CLPic.h"
#include "Log.h"
#include "CLResManager.h"

#define _SELF L"CLPic.cpp"
BOOL CLPic::ScreenShot(_In_ HWND hWnd, _In_ CONST std::wstring& wsPath) CONST
{
	if (hWnd == NULL)
		hWnd = ::GetDesktopWindow();

	RECT WindowRect;
	if (!::GetWindowRect(hWnd, &WindowRect))
	{
		LOG_CF(CLog::em_Log_Type_Exception, L"GetWindowRect in CLPic.cpp = FALSE;");
		return FALSE;
	}

	UINT uWidth = WindowRect.right - WindowRect.left;
	UINT uHeight = WindowRect.bottom - WindowRect.top;

	HDC hSrcDC = ::GetWindowDC(hWnd);
	if (hSrcDC == NULL)
	{
		LOG_CF(CLog::em_Log_Type_Exception, L"GetWindowDC in CLPic.cpp = NULL;");
		return FALSE;
	}
	
	SetResDeleter(hSrcDC, [hWnd](HDC& hDc){ ::ReleaseDC(hWnd, hDc); });

	HDC hMemDC = ::CreateCompatibleDC(hSrcDC);
	if (hMemDC == NULL)
	{
		LOG_CF(CLog::em_Log_Type_Exception, L"CreateCompatibleDC in CLPic.cpp = NULL; Err=%d", ::GetLastError());
		return FALSE;
	}
	SetResDeleter(hMemDC, [](HDC& hMemDC){  ::DeleteDC(hMemDC); });

	HBITMAP hBitmap = ::CreateCompatibleBitmap(hSrcDC, uWidth, uHeight);
	if (hBitmap == NULL)
	{
		LOG_CF(CLog::em_Log_Type_Exception, L"CreateCompatibleDC in CLPic.cpp = NULL;");
		return FALSE;
	}
	SetResDeleter(hBitmap, [](HBITMAP& hBitmap){ ::DeleteObject(hBitmap); });

	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, hBitmap);
	::BitBlt(hMemDC, 0, 0, uWidth, uHeight, hSrcDC, 0, 0, SRCCOPY);

	BITMAP bitmap = { 0 };
	::GetObject(hBitmap, sizeof(BITMAP), &bitmap);
	BITMAPINFOHEADER bi = { 0 };
	BITMAPFILEHEADER bf = { 0 };

	CONST int nBitCount = 24;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bitmap.bmWidth;
	bi.biHeight = bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = nBitCount;
	bi.biCompression = BI_RGB;
	DWORD dwSize = ((bitmap.bmWidth * nBitCount + 31) / 32) * 4 * bitmap.bmHeight;

	HANDLE hDib = GlobalAlloc(GHND, dwSize + sizeof(BITMAPINFOHEADER));
	if (hDib == NULL)
	{
		LOG_CF(CLog::em_Log_Type_Exception, L"GlobalAlloc in CLPic.cpp = NULL;");
		return FALSE;
	}
	SetResDeleter(hDib, [](HANDLE& hDib){ ::GlobalFree(hDib); });

	LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	::GetDIBits(hMemDC, hBitmap, 0, bitmap.bmHeight, (BYTE*)lpbi + sizeof(BITMAPINFOHEADER), (BITMAPINFO*)lpbi, DIB_RGB_COLORS);

	bf.bfType = 0x4d42;
	dwSize += sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bf.bfSize = dwSize;
	bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);


	FILE* pFile = nullptr;
	_wfopen_s(&pFile, wsPath.c_str(), L"wb+");
	if (pFile == nullptr)
	{
		LOG_CF(CLog::em_Log_Type_Exception, L"OpenFile in CLPic.cpp Fiald! Path:%s", wsPath.c_str());
		return FALSE;
	}


	fseek(pFile, 0, SEEK_SET);
	fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, pFile);
	fwrite(lpbi, dwSize, 1, pFile);
	fclose(pFile);

	GlobalUnlock(hDib);
	return TRUE;
}

