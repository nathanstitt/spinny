
/* @(#)testing.hpp
 */

#ifndef _TESTING_H
#define _TESTING_H 1

#include <iostream>
#include <stdexcept>
#include "unit_test/test.h"
#include "boost/filesystem/operations.hpp"
#include <boost/noncopyable.hpp>
#include "sqlite.hpp"
#include "spinny.hpp"
#include <boost/log/functions.hpp>

// evil I know... just couldn't help myself
using namespace std;
using namespace boost;

#define SQLITE_TEST_DB_FILE "./test.db"
#define STD_ARG_SIZE 3
static  char *std_args[STD_ARG_SIZE] =
{
	"foo.exe",
	"--db",
	SQLITE_TEST_DB_FILE,
};

struct DummyApp : boost::noncopyable 
{
	DummyApp() {

		Spinny::run( STD_ARG_SIZE, std_args );
  		con=Spinny::db();
  		con->exec<sqlite::none>("create table testing( col1 int, col2 string, col3 int )");
	}

	~DummyApp() {
		Spinny::stop();
		filesystem::remove( SQLITE_TEST_DB_FILE );
	}
	sqlite::connection *con;
};


struct EnableLogging {

	EnableLogging(){
		boost::logging::manipulate_logs("*")
 			.del_modifier("time")
 			.del_modifier("prefix")
 			.del_modifier("enter")
			.add_appender(&boost::logging::write_to_cout)    // all messages are written to cout
			.add_modifier(&boost::logging::prepend_prefix,"prefix" )
			.add_modifier( boost::logging::prepend_time("$yy$MM$dd $hh:$mm:$ss "), "time" )
			.add_modifier(&boost::logging::append_enter,"enter");
	}
	
	~EnableLogging(){
 		boost::logging::manipulate_logs("*")
 			.enable( boost::logging::level::disable_all );
	}


};


#endif /* _TESTING_H */

