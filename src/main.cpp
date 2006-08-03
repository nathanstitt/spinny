#include <iostream>
#include "sqlite/sqlite.hpp"
#include "id3lib/tag.h"
#include "boost/thread/thread.hpp"
#include "boost/thread/xtime.hpp"

using namespace std;
using namespace sqlite;

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

int
main(int argc, char **argv)
{
	ID3_Tag myTag;
	int secs = 5;
	connection *conn = new connection();
	cout << "Hello World" << endl;
	std::cout << "setting alarm for 5 seconds..." << std::endl;
	thread_alarm alarm(secs);
	boost::thread thrd(alarm);
	thrd.join();
}
