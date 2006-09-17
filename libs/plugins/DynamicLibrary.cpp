// DynamicLibrary.cpp: Wraps dynamic linking capabilities
//                     of Win32, Solaris, and Linux.

#ifdef _WIN32
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "DynamicLibrary.h"


DynamicLibrary::DynamicLibrary(const char *libName)
	: libHandle(0)
{
	Load( libName );
}

DynamicLibrary::~DynamicLibrary( void )
{
	Unload();
}

bool DynamicLibrary::Load(const char *libName)
{
	if (libName != NULL && ::strlen(libName) > 0)
	{
		if (libHandle == NULL)
		{
#ifdef _WIN32
			libHandle = ::LoadLibrary( libName );
#else
			libHandle = ::dlopen( libName, RTLD_LAZY );
#endif
		}
	}

	return (libHandle != NULL);
}

bool DynamicLibrary::Unload( void )
{
	if (libHandle != NULL)
	{
#ifdef _WIN32
		::FreeLibrary( libHandle );
#else
		::dlclose( libHandle );
#endif
	}

	libHandle = NULL;

	cache.clear();

	return true;
}

DLPROC DynamicLibrary::GetProcAddr(const char *procName) const
{
	DLPROC proc = NULL;

	if (libHandle != NULL)
	{
#ifdef _WIN32
		proc = (DLPROC) ::GetProcAddress(libHandle, procName);
#else
		proc = (DLPROC) ::dlsym(libHandle, procName);
#endif
	}

	return proc;
}

DLPROC DynamicLibrary::GetProcAddrCached(const char *procName,
										 unsigned int procId)
{
	DLPROC proc = NULL;

	if (libHandle != NULL)
	{
		if ((procId >= cache.size()) || (cache.size() < 1))
			cache.resize(procId + 1, cache_info());

		cache_info &ci = cache[ procId ];

		if (ci.procAddr == NULL && !ci.testFlag)
		{
			ci.testFlag = true;
			ci.procAddr = GetProcAddr(procName);
		}

		proc = ci.procAddr;
	}

	return proc;
}

#ifdef _WIN32
DLPROC DynamicLibrary::GetProcAddr(int ordinal) const
{
	DLPROC proc = NULL;

	if (libHandle != NULL)
	{
		proc = (DLPROC) ::GetProcAddress(libHandle,
								 MAKEINTRESOURCE(ordinal));
	}

	return proc;
}

DLPROC DynamicLibrary::GetProcAddrCached(int ordinal)
{
	return GetProcAddrCached(MAKEINTRESOURCE(ordinal),
							 ordinal);
}
#endif

