// SimpleMath.h: Wraps functions exported by dynamic
//               libraries supporting Simple Math API.

#ifndef SIMPLEMATH_H_INCLUDED
#define SIMPLEMATH_H_INCLUDED

#include "DynamicLibrary.h"

class SimpleMath : public DynamicLibrary
{
public:

	SimpleMath( const char *libName ) :
	  DynamicLibrary(libName){};

	~SimpleMath( void ){};

	int Add(int x, int y);
	int Sub(int x, int y);
	int Mul(int x, int y);
	int Div(int x, int y);

	void Who(char *str, int nChars);
};

#endif
