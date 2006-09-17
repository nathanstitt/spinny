// BaseInterface.h: Serves as the root ancestor for
//                  all interfaces.

#ifndef BASEINTERFACE_H_INCLUDED
#define BASEINTERFACE_H_INCLUDED

#define interface struct

interface IBase
{
	virtual bool QueryInterface(const char *iid,
								IBase **iface ) = 0;
	virtual void Destroy(void) = 0;
};

#endif
