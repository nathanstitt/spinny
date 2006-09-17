// TimeTest.cpp: Exports ten do-nothing functions
//               for purposes of timing calls
//               with DynamicLibrary::GetProcAddr()
//               and GetProcAddCached() methods.
//
//               The more functions exported, the
//               longer the symbol lookup time (at least
//               under Win32, on Solaris/Linux it doesn't
//               seem to make a difference).  I figured ten
//               exported functions would be fairly realistic
//               number for a plug-in to export.

#ifdef _WIN32
#define LINKAGE     __declspec(dllexport)
#else
#define LINKAGE
#endif


extern "C" void LINKAGE time_test_0( void )
{
    return;
}

extern "C" void LINKAGE time_test_1( void )
{
    return;
}

extern "C" void LINKAGE time_test_2( void )
{
    return;
}

extern "C" void LINKAGE time_test_3( void )
{
    return;
}

extern "C" void LINKAGE time_test_4( void )
{
    return;
}

// this is the function that actually gets called
extern "C" void LINKAGE time_test_5( void )
{
    return;
}

extern "C" void LINKAGE time_test_6( void )
{
    return;
}

extern "C" void LINKAGE time_test_7( void )
{
    return;
}

extern "C" void LINKAGE time_test_8( void )
{
    return;
}

extern "C" void LINKAGE time_test_9( void )
{
    return;
}
