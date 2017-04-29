#include "stdafx.h"
#include "TimeTick.h"

CTimeTick::CTimeTick()
{
	Reset();
}

CTimeTick::~CTimeTick()
{
}

VOID CTimeTick::Reset()
{
	ulTick = ::GetTickCount64();
}

ULONGLONG CTimeTick::GetSpentTime(_In_ em_TimeTick emTimeTick) CONST
{
	switch (emTimeTick)
	{
	case CTimeTick::em_TimeTick_Hour:
		return (::GetTickCount64() - ulTick) / 1000 / 60 / 60;
	case CTimeTick::em_TimeTick_Minute:
		return (::GetTickCount64() - ulTick) / 1000 / 60;
	case CTimeTick::em_TimeTick_Second:
		return (::GetTickCount64() - ulTick) / 1000;
	case em_TimeTick_Millisecond:
		return (::GetTickCount64() - ulTick);
	default:
		break;
	}
	return NULL;
}
