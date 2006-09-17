// SimpleMath.cpp: Wraps functions exported by dynamic
//                 libraries supporting Simple Math API.

#ifdef _WIN32
#define NO_GDI
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "SimpleMath.h"

// indices for GetProcAddrCached() calls
#define SIMPLEMATH_ADD		0
#define SIMPLEMATH_SUB		1
#define SIMPLEMATH_MUL		2
#define SIMPLEMATH_DIV		3
#define SIMPLEMATH_WHO		4


int SimpleMath::Add(int x, int y)
{
	int(*f)(int,int) = reinterpret_cast<int(*)(int,int)>(
		GetProcAddrCached("simple_math_add", SIMPLEMATH_ADD));

	if (f)
		return f(x, y);

	return -1;
}

int SimpleMath::Sub(int x, int y)
{
	int(*f)(int,int) = reinterpret_cast<int(*)(int,int)>(
		GetProcAddrCached("simple_math_sub", SIMPLEMATH_SUB));

	if (f)
		return f(x, y);

	return -1;
}

int SimpleMath::Mul(int x, int y)
{
	int(*f)(int,int) = reinterpret_cast<int(*)(int,int)>(
		GetProcAddrCached("simple_math_mul", SIMPLEMATH_MUL));

	if (f)
		return f(x, y);

	return -1;
}

int SimpleMath::Div(int x, int y)
{
	int(*f)(int,int) = reinterpret_cast<int(*)(int,int)>(
		GetProcAddrCached("simple_math_div", SIMPLEMATH_DIV));

	if (f)
		return f(x, y);

	return -1;
}

void SimpleMath::Who(char *str, int nChars)
{
	void(*f)(char*,int) = reinterpret_cast<void(*)(char*,int)>(
		GetProcAddrCached("simple_math_who", SIMPLEMATH_WHO));

	if (f)
		f(str, nChars);
}
