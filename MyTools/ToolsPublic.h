#ifndef __MYTOOLS_PUBLIC_TOOLSPUBLIC_H__
#define __MYTOOLS_PUBLIC_TOOLSPUBLIC_H__

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <process.h>
#include <xstring>
#pragma component(browser, off, references)
#include <functional>
#pragma component(browser, on, references)
#include <algorithm>
#include <memory>
#include <vector>

using namespace std;

#define _L(x) __L(x)
#define __L(x) L##x

#define TL_EXIT_ERROR(Condition)	\
	do								\
	{								\
		if (!(Condition))			\
			goto Function_Exit;		\
	} while (false)

#define TL_EXIT_SUCCESS(Condition)	\
	do								\
	{								\
		if (Condition)				\
			goto Function_Exit;		\
	} while (false)

#define TL_EXIT_ERROR_RET_CODE(Condition, nCode)	\
	do												\
	{												\
		if (!(Condition))							\
		{											\
			bRetCode = nCode;						\
			goto Function_Exit;						\
		}											\
	} while (false)

#define TL_EXIT_SUCCESS_RET_CODE(Condition, nCode)	\
	do												\
	{												\
		if (Condition)								\
		{											\
			bRetCode = nCode;						\
			goto Function_Exit;						\
		}											\
	} while (false)

#define TL_COM_RELEASE(pInterface)	\
	do								\
	{								\
		if (pInterface)				\
		{                           \
			(pInterface)->Release();\
			(pInterface) = NULL;    \
		}                           \
	} while (false)

#define TL_DELETE_ARRAY(pArray)     \
	do								\
	{								\
		if (pArray)					\
		{                           \
			delete [](pArray);		\
			(pArray) = NULL;		\
		}                           \
	} while (false)


#define TL_DELETE(p)				\
	do								\
	{								\
		if (p)						\
		{							\
			delete (p);				\
			(p) = NULL;				\
		}							\
	} while (false)

#define PrintDebug_W(Condition, lpwszFormat, ...)								\
	if (Condition)																\
	{																			\
		Log(LOG_LEVEL_EXCEPTION, lpwszFormat, __VA_ARGS__);									\
		return FALSE;															\
	}

typedef unsigned (WINAPI *PTHREAD_START) (void*);
#define cbBEGINTHREADEX(psa, cbStackSize, pfnStartAddr,							\
	pvParam, dwCreateFlags, pdwThreadID)										\
	((HANDLE)_beginthreadex(													\
	(void *)(psa),																\
	(unsigned)(cbStackSize),													\
	(PTHREAD_START)(pfnStartAddr),												\
	(void *)(pvParam),															\
	(unsigned)(dwCreateFlags),													\
	(unsigned *)(pdwThreadID)))


#define VK_0																	48
#define VK_1																	49
#define VK_2																	50
#define VK_3																	51
#define VK_4																	52
#define VK_5																	53
#define VK_6																	54
#define VK_7																	55
#define VK_8																	56
#define VK_9																	57

//下面是自编的
#define VK_D																	59
#define VK_J																	58
#define VK_ARROW_TOP															38
#define VK_ARROW_LEFT															37
#define VK_ARROW_RIGHT															39
#define VK_ARROW_DOWN															40
#define VK_SHIFT_0																0x101
#define VK_SHIFT_1																0x102
#define VK_SHIFT_2																0x103
#define VK_SHIFT_3																0x104
#define VK_SHIFT_4																0x105
#define VK_SHIFT_5																0x106
#define VK_SHIFT_6																0x107
#define VK_SHIFT_7																0x108
#define VK_SHIFT_8																0x109
#define VK_SHIFT_9																0x10A
#define VK_SHIFT_D																0x10B
#define VK_SHIFT_J																0x10C


#endif//_TOOLSPUBLIC_H__