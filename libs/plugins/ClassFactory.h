// ClassFactory.h: Used to create initial interface
//                 pointer.

#ifndef CLASSFACTORY_H_INCLUDED
#define CLASSFACTORY_H_INCLUDED

#include "DynamicLibrary.h"
#include "BaseInterface.h"

class ClassFactory : protected DynamicLibrary
{
public:

    ClassFactory(void) {};
    ClassFactory(const char *libName) :
      DynamicLibrary(libName) {};

    ~ClassFactory(void){};

    DynamicLibrary::IsLoaded;
    DynamicLibrary::Load;
    DynamicLibrary::Unload;

    bool CreateInterface(const char *iid, IBase **iface);
};

#endif
