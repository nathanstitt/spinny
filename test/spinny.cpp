#include "testing.hpp"
#include  <boost/thread/thread.hpp>
#include "boost/filesystem/operations.hpp"
#include "spinny/spinny.hpp"


#include <cstdlib>

#ifdef WINDOWS
#define sleep Sleep
#endif


SUITE(SpinnySuite) {


static  char *help_args[] =
{
	"-h",
};


struct Spin : DummyApp {

};

static sqlite::connection* con1 = 0;
static sqlite::connection* con2 = 0;
void set_con1(){
	con1 = sqlite::db();
	sleep(1);
}
void set_con2(){
	con2 = sqlite::db();
	sleep(1);
}


TEST( DB ){
	Spin s;
	boost::thread t1( &set_con1 );
	boost::thread t2( &set_con2 );
	t1.join();
	t2.join();
 	CHECK( con1 != con2 );
}

TEST( Config ){
	Spin s;
	CHECK_EQUAL( s.db_path.string(), Spinny::instance()->config<string>( "db" ) );
}


} // SUITE( SpinnySuite )


int
spinny( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SpinnySuite");
}
