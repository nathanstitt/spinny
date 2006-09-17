#include <iostream>
#include "spinny.hpp"
#include "sqlite/sqlite.hpp"
#include "id3lib/tag.h"
#include "boost/thread/thread.hpp"
#include "boost/thread/xtime.hpp"

using namespace std;
using namespace sqlite;

void tss_cleanup_implemented(void)
    {
}


struct thread_alarm
{
   thread_alarm(int secs) : m_secs(secs) { }
   void operator()()
   {
       boost::xtime xt;
       boost::xtime_get(&xt, boost::TIME_UTC);
       xt.sec += m_secs;

       boost::thread::sleep(xt);

       std::cout << "alarm sounded..." << std::endl;
   }
   int m_secs;
};

// int
// main(int argc, char **argv) {
// 	ID3_Tag myTag;
// 	int secs = 1;
// 	connection *conn = new connection();
// 	cout << "Hello World" << endl;
// 	std::cout << "setting alarm for 5 seconds..." << std::endl;
// 	thread_alarm alarm(secs);
// 	boost::thread thrd(alarm);
// 	thrd.join();

// 	return Spinny::run( argc, argv );
// }




#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <cassert>

boost::thread_specific_ptr<int> value;

boost::thread_specific_ptr<std::string> s_value;

void increment()
{
    int* p = value.get();
    ++*p;
}

void thread_proc()
{
    value.reset(new int(0)); // initialize the thread's storage
    for (int i=0; i<10; ++i)
    {
        increment();
        int* p = value.get();
        assert(*p == i+1);
    }
}

int main(int , char*)
{
    boost::thread_group threads;
    for (int i=0; i<5; ++i)
        threads.create_thread(&thread_proc);
    threads.join_all();
}
