// GoodMath.cpp: compiled/linked to create a Simple Math API
//               compliant plug-in.  Plug-in is used by the
//               SimpleMath class (SimpleMath.cpp)

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define LINKAGE		__declspec(dllexport)
#else
#define LINKAGE
#endif

static const char myName[] = "GoodMath";

extern "C" int LINKAGE simple_math_add(int x, int y)
{
	return x + y;
}

extern "C" int LINKAGE simple_math_sub(int x, int y)
{
	return x - y;
}

extern "C" int LINKAGE simple_math_mul(int x, int y)
{
	return x * y;
}

extern "C" int LINKAGE simple_math_div(int x, int y)
{
	if (y != 0)
		return x / y;

	return 0;
}

extern "C" void LINKAGE simple_math_who(char *name, int len)
{
	int myNameLen = ::strlen(myName);

	len = (len <= myNameLen ? len - 1: myNameLen);

	for (int i = 0; i < len; i++)
		name[i] = myName[i];

	name[len] = '\0';
}
