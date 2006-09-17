// IDoMath.h: Interface which declares some very
//            basic arithmetic methods.

#ifndef IDOMATH_H_INCLUDED
#define IDOMATH_H_INCLUDED

#include "BaseInterface.h"

interface IDoMath : IBase
{
	virtual int Add(int x, int y) = 0;
	virtual int Sub(int x, int y) = 0;
	virtual int Mul(int x, int y) = 0;
	virtual int Div(int x, int y) = 0;
};

#endif
