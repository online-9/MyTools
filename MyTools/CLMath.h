#ifndef MYTOOLS_MYTOOLS_MATH_CLMATH_H__
#define MYTOOLS_MYTOOLS_MATH_CLMATH_H__

#include "ClassInstance.h"

class CLMath : public CClassInstance<CLMath>
{
public:
	CLMath() = default;
	~CLMath() = default;

	inline static float Q_rsqrt(float fNumber);
private:

};



#endif