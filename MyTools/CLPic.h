#ifndef __MYTOOLS_PIC_CLPIC_H__
#define __MYTOOLS_PIC_CLPIC_H__

#include "ClassInstance.h"

class CLPic : public CClassInstance<CLPic>
{
public:
	CLPic() = default;
	~CLPic() = default;

	BOOL ScreenShot(_In_ HWND hWnd, _In_ CONST std::wstring& wsPath) CONST;
private:

};



#endif // !__MYTOOLS_PIC_CLPIC_H__
