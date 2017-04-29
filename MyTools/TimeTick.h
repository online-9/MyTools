#ifndef __MYTOOLS_TIME_TIMETICK_H__
#define __MYTOOLS_TIME_TIMETICK_H__

#include "ToolsPublic.h"

class CTimeTick
{
public:
	CTimeTick();
	~CTimeTick();

	VOID Reset();

	enum em_TimeTick { em_TimeTick_Hour, em_TimeTick_Minute, em_TimeTick_Second, em_TimeTick_Millisecond };
	ULONGLONG GetSpentTime(_In_ em_TimeTick emTimeTick) CONST;
private:
	ULONGLONG ulTick;
};



#endif // !__MYTOOLS_TIME_TIMETICK_H__
