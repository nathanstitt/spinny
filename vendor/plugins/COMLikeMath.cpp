// COMLikeMath.cpp: Defines class COMLikeMath which
//                  implements IBase, IDoMath, and
//                  INamed interfaces.  Application
//                  that uses these interface knows
//                  nothing about underlying COMLike-
//                  Math object.

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <iomanip>
#include "IDoMath.h"
#include "INamed.h"


#ifdef _WIN32
#define LINKAGE		__declspec(dllexport)
#else
#define LINKAGE
#endif


class COMLikeMath : IDoMath, INamed
{
private:

	int		refCount;
	char	myName[100];

public:

	COMLikeMath(void);
	~COMLikeMath(void);

	virtual bool QueryInterface(const char *iid,
								IBase **iface);
	virtual void Destroy(void);

	virtual int Add(int x, int y);
	virtual int Sub(int x, int y);
	virtual int Mul(int x, int y);
	virtual int Div(int x, int y);

	virtual void Who(char *name, int len);
};


// function required to support creation of interface
// provider (i.e. COMLikeMath) by ClassFactory object

extern "C" bool LINKAGE cf_create_interface(const char *iid,
											IBase **iface)
{
	*iface = NULL;

	COMLikeMath *ptr;

	try
	{
		ptr = new COMLikeMath;

		if (!ptr->QueryInterface(iid, iface))
		{
			ptr->Destroy();
			ptr = NULL;
		}
	}
	catch(...)
	{
		ptr = NULL;
	}

	return ptr != NULL;
}

COMLikeMath::COMLikeMath(void) : refCount(0)
{
	::sprintf(myName, "COMLikeMath:0x%X", (int)this);
}

COMLikeMath::~COMLikeMath(void)
{
	std::cout << "COMLikeMath is destroyed."
			  << std::endl;
}

///////////////////////////////////////////////////////
//  IBase implementation
///////////////////////////////////////////////////////

bool COMLikeMath::QueryInterface(const char *iid,
								 IBase **iface)
{
	*iface = NULL;

	if (::strcmp(iid, "IDoMath") == 0)
		*iface = static_cast<IDoMath*>(this);
	else if (::strcmp(iid, "IBase") == 0)
		*iface = static_cast<IDoMath*>(this);
	else if (::strcmp(iid, "INamed") == 0)
		*iface = static_cast<INamed*>(this);

	if (*iface != NULL)
		refCount++;

	return *iface != NULL;
}

void COMLikeMath::Destroy(void)
{
	if (--refCount == 0)
		delete this;
}

///////////////////////////////////////////////////////
//  IDoMath implementation
///////////////////////////////////////////////////////

int COMLikeMath::Add(int x, int y)
{
	return x + y;
}

int COMLikeMath::Sub(int x, int y)
{
	return x - y;
}

int COMLikeMath::Mul(int x, int y)
{
	return x * y;
}

int COMLikeMath::Div(int x, int y)
{
	if (y != 0)
		return x / y;

	return 0;
}

///////////////////////////////////////////////////////
//  INamed implementation
///////////////////////////////////////////////////////

void COMLikeMath::Who(char *name, int len)
{
	int myNameLen = ::strlen(myName);

	len = (len <= myNameLen ? len - 1: myNameLen);

	for (int i = 0; i < len; i++)
		name[i] = myName[i];

	name[len] = '\0';
}
