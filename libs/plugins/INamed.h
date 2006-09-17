// INamed.h: Interface that declares a method to
//           return plug-in name.

#ifndef INAMED_H_INCLUDED
#define INAMED_H_INCLUDED

#include "BaseInterface.h"

interface INamed : IBase
{
	virtual void Who(char *name, int len) = 0;
};

#endif
