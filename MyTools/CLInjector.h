#ifndef __CINJECTOR_H__
#define __CINJECTOR_H__ TRUE

#include "Character.h"

typedef struct _CL_INJECTOR_INFO
{
	WCHAR wchLoadPath[MAX_PATH];			//LoadLibrary Path
	CHAR  chAESCode[64];
}CL_INJECTOR_INFO, *PCL_INJECTOR_INFO;

class CLInjector
{
public:
	CLInjector();
	~CLInjector();

	static BOOL EncrypeInjector();
private:

};



#endif//__CINJECTOR_H__