// ClassFactory.cpp: Used to create initial interface
//                   pointer.  This class wraps the
//                   cf_create_interface function exported
//                   by dynamic libraries supporting COM-
//                   like capabilities.

#ifdef _WIN32
#define NO_GDI
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "ClassFactory.h"

bool ClassFactory::CreateInterface(const char *iid,
								   IBase **iface)
{
	*iface = NULL;

	bool(*f)(const char *, IBase **) =
		reinterpret_cast<bool(*)(const char *, IBase**)>(
		GetProcAddr( "cf_create_interface" ) );

	if (f)
		return f(iid, iface);

	return false;
}
