// PITest.cpp: Test program.  Invoke from command line as
//
//                     PITest {.dll or .so}
//
//             Runs a timing test using the TimeTest plug-
//             in. Uses SimpleMath object to test dynamic
//             library specified on command line.  Finally,
//             test COM-like capability of ClassFactory and
//             IDoMath/INamed interfaces.

#ifdef _WIN32
#define NO_GDI
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <sys/timeb.h>
#include <time.h>
typedef struct _timeb TIMEB;
#define ftime _ftime

#else

#include <sys/types.h>
#include <sys/timeb.h>
typedef struct timeb TIMEB;
#ifndef ftime
extern "C" int ftime(struct timeb *tb);
#endif

#endif

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include "DynamicLibrary.h"
#include "SimpleMath.h"
#include "ClassFactory.h"
#include "IDoMath.h"
#include "INamed.h"

static void elapsed_time( TIMEB &elapsed,
                          const TIMEB &start,
                          const TIMEB &finish );

void main( int argc, char *argv[] )
{
    if (argc != 2)
    {
        cerr << "usage: " << argv[0]
             << " {dynamic library}" << endl;
        exit( 1 );
    }

    cout << "Testing plug-in..." << endl;

    char str[100];

#ifdef _WIN32
    DynamicLibrary dl(".\\TimeTest.dll");
#else
    DynamicLibrary dl("./TimeTest.so");
#endif

    if (!dl.IsLoaded())
    {
        cerr << "Failed to load TimeTest dynamic library."
             << endl;
        exit( 1 );
    }


    ///////////////////////////////////////////////////////
    //  Test cached call speed
    ///////////////////////////////////////////////////////

    cout << "\nCall performance test..." << endl;

    int i;
    TIMEB tmstart, tmfinish, tmelapsed;
    const int N_CALLS = 10000000;
    void(*p1)(void);

    // first the non-cached version

    cout << "Testing non-cached version..." << endl;

    ftime( &tmstart );

    for (i = 0; i < N_CALLS; i++)
    {
        p1 = reinterpret_cast<void(*)(void)>(
            dl.GetProcAddr( "time_test_5" ));

        if (p1)
            p1();
        else
            cerr << "p1 is NULL function pointer." << endl;
    }

    ftime( &tmfinish );

    elapsed_time( tmelapsed, tmstart, tmfinish );

    cout << "Elapsed time was "
         << tmelapsed.time
         << " sec "
         << tmelapsed.millitm
         << " ms" << endl;

    // now our cached version

    cout << "Testing cached version..." << endl;

    ftime( &tmstart );

    for (i = 0; i < N_CALLS; i++)
    {
        p1 = reinterpret_cast<void(*)(void)>(
            dl.GetProcAddrCached( "time_test_5", 1 ));

        if (p1)
            p1();
        else
            cerr << "p1 is NULL function pointer." << endl;
    }

    ftime( &tmfinish );

    elapsed_time( tmelapsed, tmstart, tmfinish );

    cout << "Elapsed time was "
         << tmelapsed.time
         << " sec "
         << tmelapsed.millitm
         << " ms" << endl;


    ///////////////////////////////////////////////////////
    //  Test SimplePlugIn class
    ///////////////////////////////////////////////////////

    cout << "\nTest SimplePlugIn class..." << endl;

    SimpleMath smath( argv[1] );

    if (smath.IsLoaded())
    {
        smath.Who(str, sizeof(str) - 1);
        cout << "smath name is "
             << str
             << endl;

        cout << "smath.Add(20,30) = "
             << smath.Add(20,30)
             << endl;

        cout << "smath.Mul(10,5) = "
             << smath.Mul(10,5)
             << endl;
    }
    else
        cerr << "Failed to create SimpleMath object."
             << endl;


    ///////////////////////////////////////////////////////
    //  Test a COM-like plug-in
    ///////////////////////////////////////////////////////

    cout << "\nTest COM-like plug-in..." << endl;

    IDoMath *idm;

#ifdef _WIN32
    ClassFactory cf1( ".\\COMLikeMath.dll" );
#else
    ClassFactory cf1( "./COMLikeMath.so" );
#endif

    if (cf1.IsLoaded())
    {
        // create the initial interface pointer

        if (cf1.CreateInterface("IDoMath",
            reinterpret_cast<IBase**>(&idm)))
        {
            int x, y;

            x = 100;
            y = 10;

            cout << "idm->Add() = " << idm->Add(x, y) << endl;
            cout << "idm->Sub() = " << idm->Sub(x, y) << endl;
            cout << "idm->Mul() = " << idm->Mul(x, y) << endl;
            cout << "idm->Div() = " << idm->Div(x, y) << endl;

            INamed *in;

            // obtain INamed interface pointer

            if (idm->QueryInterface( "INamed",
                reinterpret_cast<IBase**>(&in)))
            {
                char name[100];

                in->Who( name, sizeof(name) - 1 );
                cout << name << endl;

                in->Destroy();
            }
            else
                cerr << "Interface INamed is not supported."
                     << endl;

            idm->Destroy();
        }
        else
            cerr << "Interface IDoMath is not supported."
                 << endl;
    }
    else
        cerr << "Failed to load COM-like library." << endl;

    cout << "All done." << endl;
}


static void elapsed_time( TIMEB &elapsed,
                          const TIMEB &start,
                          const TIMEB &finish )
{
    int ft, fm;
    if (finish.millitm < start.millitm)
    {
        fm = 1000;
        ft = 1;
    }
    else
    {
        ft = 0;
        fm = 0;
    }

    elapsed.time = finish.time - ft - start.time;
    elapsed.millitm = finish.millitm + fm - start.millitm;
}

