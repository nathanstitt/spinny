#include "testing.hpp"
#include  <boost/thread/thread.hpp>
#include "boost/filesystem/operations.hpp"
#include "spinny.hpp"


#include <cstdlib>



SUITE(SpinnySuite) {


static  char *help_args[] =
{
	"-h",
};


#define STD_ARG_SIZE 3
static  char *std_args[STD_ARG_SIZE] =
{
	"foo.exe",
	"--db",
	SQLITE_TEST_DB_FILE,
};

struct Spin {
	Spin(){ Spinny::run( STD_ARG_SIZE, std_args );  }
	~Spin(){ Spinny::stop(); }
};

static sqlite::connection* con1 = 0;
static sqlite::connection* con2 = 0;
void set_con1(){
	con1 = Spinny::db();
	Sleep(1);
}
void set_con2(){
	con2 = Spinny::db();
	Sleep(1);
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
	CHECK_EQUAL( SQLITE_TEST_DB_FILE, Spinny::instance()->config<string>( "db" ) );
}


TEST( Run ){
 	Spinny::run(  STD_ARG_SIZE, std_args );
 	Spinny::stop();
	CHECK( boost::filesystem::remove( SQLITE_TEST_DB_FILE ) );
}} // SUITE( SpinnySuite )


int
spinny( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SpinnySuite");
}
