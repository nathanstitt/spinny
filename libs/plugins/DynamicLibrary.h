// DynamicLibrary.h: Wraps dynamic linking capabilities
//                   of Win32, Solaris, and Linux.

#ifndef DYNAMICLIBRARY_H_INCLUDED
#define DYNAMICLIBRARY_H_INCLUDED

#include <vector>

typedef void(*DLPROC)();

class DynamicLibrary  
{
private:

	class cache_info
	{
	public:
		DLPROC	procAddr;
		bool	testFlag;

	public:
		cache_info(void) : procAddr(0), testFlag(false) {};
	};

private:

#ifdef _WIN32
	HANDLE						libHandle;
#else
	void*						libHandle;
#endif

	std::vector<cache_info>		cache;

public:

	DynamicLibrary(void) : libHandle(0) {};
	DynamicLibrary(const char *libName);
	virtual ~DynamicLibrary(void);

	bool Load(const char *libName);
	bool Unload(void);
	bool IsLoaded(void) const { return (libHandle != 0); };

	DLPROC GetProcAddr(const char *procName) const;
	DLPROC GetProcAddrCached(const char *procName,
							 unsigned int procId);
#ifdef _WIN32
	DLPROC GetProcAddr(int ordinal) const;
	DLPROC GetProcAddrCached(int ordinal);
#endif

};

#endif

