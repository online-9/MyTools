#ifndef __MYTOOLS_PUBLIC_CLPUBLIC_H__
#define __MYTOOLS_PUBLIC_CLPUBLIC_H__

#include "ToolsPublic.h"
#include <deque>

#define WINDOW_SYSTEM_NT4				0x1 //Microsoft Windows NT 4.0
#define WINDOW_SYSTEM_95				0x2 //Microsoft Windows 95
#define WINDOW_SYSTEM_98				0x3 //Microsoft Windows 98
#define WINDOW_SYSTEM_ME				0x4 //Microsoft Windows Me
#define WINDOW_SYSTEM_2000				0x5 //Microsoft Windows 2000
#define WINDOW_SYSTEM_XP				0x6 //Microsoft Windows XP
#define WINDOW_SYSTEM_XP_64				0x7 //Microsoft Windows XP Professional x64 Edition
#define WINDOW_SYSTEM_2003				0x8 //Microsoft Windows Server 2003
#define WINDOW_SYSTEM_2003_R2			0x9 //Microsoft Windows Server 2003 R2
#define WINDOW_SYSTEM_Vista				0xA //Microsoft Windows Vista
#define WINDOW_SYSTEM_2008				0xB //Microsoft Windows Server 2008
#define WINDOW_SYSTEM_WIN7_32			0xC //Microsoft Windows 7 x32
#define WINDOW_SYSTEM_WIN7_64			0xD //Microsoft Windows 7 x64
#define WINDOW_SYSTEM_2008_R2			0xE //Microsoft Windows Server 2008 R2
#define WINDOW_SYSTEM_UnKnown			0xF //未知操作系统


class CLPublic
{
public:
	static size_t   GetHash(_In_ CONST std::string& str);													// 获取哈希值
	static size_t	GetHash(_In_ CONST std::wstring& wstr);													// 获取哈希值
	static VOID		GetCRC32Table();															//建立CRC表
	static VOID		GetCRC32(IN LPSTR inStr,IN UINT uSize, OUT LPSTR OutStr);					//获取CRC值
	static VOID		GetCRC32_DWORD(IN LPCSTR inStr, IN UINT uSize, OUT DWORD& OutValue);
	static DWORD	GetCRC32_DWORD(IN LPCSTR inStr, IN UINT uSize);
	static DWORD	GetCRC32_DWORD(IN LPCWSTR inStr);
	static VOID		GetCRC32_Long(IN LPSTR inStr, IN LONG lgSize, OUT LPSTR OutStr);
	static BOOL		SimulationKey(DWORD dwASCII, HWND hWnd, DWORD dwTime = 10);					//模拟按键
	static DWORD	GetVirKey(DWORD dwASCII);													//获取虚拟码
	static BOOL		SimulationMouse(int x, int y);												//模拟鼠标
	static BOOL		SimulationRightMouse(int x, int y);											//模拟鼠标
	static BOOL		SendAscii(wchar_t data, BOOL shift);
	static BOOL		SendUnicode(wchar_t data);
	static BOOL		SendKey(WORD wVk);
	static BOOL		SendKeys(const wchar_t* data);
	static BOOL		SendKeys(const char* data);
	static BOOL		ForceExit();																//自身强制崩溃
	static BOOL		FileExit(LPCWSTR pwchPath);													//文件是否存在Unicode
	static BOOL		FileExist(_In_ CONST std::wstring& wsPath);
	static BOOL		FileExistA(LPCSTR pchPath);													//文件是否存在ASCII
	static BOOL		CopyText2Clipboard(LPCWSTR pwchText);										//CopyText To Clipboard
	static BOOL		FormatTime( IN ULONGLONG uResult, OUT LPWSTR pwchText, _In_ UINT uTextSize);//format time to Text
	static DWORD	GetSystemVerson();															// 获取当前系统
	static __inline LPVOID   GetEIP();

	template<typename PtrT>
	inline static VOID		SetPtr(PtrT* p, CONST PtrT& t)
	{
		if (p != nullptr)
			*p = t;
	}

	template<class T, class Fn>
	inline static UINT Vec_erase_if(_In_ std::vector<T>& vlst, _In_ Fn _Pred)
	{	
		vlst.erase(std::remove_if(vlst.begin(), vlst.end(), _Pred), vlst.end());
		return vlst.size();
	}

	template<class T, class Fn>
	inline static UINT Dque_erase_if(_In_ std::deque<T>& vlst, _In_ Fn _Pred)
	{
		vlst.erase(std::remove_if(vlst.begin(), vlst.end(), _Pred), vlst.end());
		return vlst.size();
	}

	template<class T, class Fn>
	inline static BOOL Vec_find_if(_In_ CONST std::vector<T>& vlst, _Out_opt_ T* p, _In_ Fn _Pred)
	{
		auto& itr = std::find_if(vlst.begin(), vlst.end(), _Pred);
		if (itr != vlst.end())
			SetPtr(p, *itr);
		return itr != vlst.end();
	}

	// fn() = TRUE  contiune!
	// return TRUE which TimeOut
	static BOOL		TimeOut_By_Condition(_In_ DWORD dwMaxTimeOut, _In_ std::function<BOOL(VOID)> fn);

	template<class T>
	static float GetDisBy3D(_In_ CONST T& CurPoint, _In_ CONST T& TarPoint)
	{
		auto fsum = pow(CurPoint.X - TarPoint.X, 2.0f) + pow(CurPoint.Y - TarPoint.Y, 2.0f) + pow(CurPoint.Z - TarPoint.Z, 2.0f);
		return sqrt(fsum);
	}

	template<class T>
	static float GetDisBy2D(_In_ CONST T& CurPoint, _In_ CONST T& TarPoint)
	{
		auto fsum = pow(CurPoint.X - TarPoint.X, 2.0f) + pow(CurPoint.Y - TarPoint.Y, 2.0f);
		return sqrt(fsum);
	}

	template<class T>
	static int GetRecentlyIndexByVec(_In_ CONST vector<T>& Vec, _In_ CONST T& TarPoint, _In_ float MinDis = FLT_MAX)
	{
		auto Index = -1;
		auto VecMinDis = FLT_MAX;

		for (UINT i = 0; i < Vec.size(); ++i)
		{
			auto fDis = GetDisBy2D(TarPoint, Vec.at(i));
			if (fDis < VecMinDis)
			{
				VecMinDis = fDis;
				Index = static_cast<int>(i);
			}
		}

		return VecMinDis < MinDis ? Index : -1;
	}

	template<class T,class T2>
	static int GetRecentlyIndexByPointVec(_In_ CONST vector<T>& Vec, _In_ CONST T2& TarPoint, _In_ float MinDis = FLT_MAX)
	{
		auto Index = -1;
		auto VecMinDis = FLT_MAX;

		for (UINT i = 0; i < Vec.size(); ++i)
		{
			auto fDis = GetDisBy2D(TarPoint, Vec.at(i).GetPoint());
			if (fDis < VecMinDis)
			{
				VecMinDis = fDis;
				Index = static_cast<int>(i);
			}
		}
		return VecMinDis < MinDis ? Index : -1;
	}
	template<class T, class Finder>
	static T* Vec_find_if(_In_ std::vector<T>& vlst, _In_ Finder _Pred)
	{
		auto& itr = std::find_if(vlst.begin(), vlst.end(), _Pred);
		return itr == vlst.end() ? nullptr : &*itr;
	}

	template<class T, class Finder>
	static CONST T* Vec_find_if_Const(_In_ CONST std::vector<T>& vlst, _In_ Finder _Pred)
	{
		auto& itr = std::find_if(vlst.begin(), vlst.end(), _Pred);
		return itr == vlst.end() ? nullptr : &*itr;
	}

	
	

public:
	CLPublic();
	~CLPublic();

public:
	static ULONG s_Crc32Table[256];
};




#endif