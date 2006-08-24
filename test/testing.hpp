
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
		if ( ! boost::filesystem::exists( TESTING_FIXTURES_PATH ) ){
			boost::filesystem::create_directory( TESTING_FIXTURES_PATH );
		}
		Spinny::run( STD_ARG_SIZE, std_args );
  		con=Spinny::db();
		con->exec<sqlite::none>("create table testing( col1 int, col2 string, col3 int )");
	}

	~DummyApp() {
		Spinny::stop();
		filesystem::remove( SQLITE_TEST_DB_FILE );
	}

	template<typename T>
	bool
	test_table_obj();

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
	



};

template<typename T> inline
bool
DummyApp::test_table_obj(){
	T table;

	const sqlite::table::description *desc = table.m_table_description();
	std::vector< std::string > vals;
	*con << "insert into " << desc->table_name() << " (";
	for ( int i=0; i< desc->num_fields(); ++i ){
		if ( i>0 ){
			*con << ",";
		}
		*con << desc->fields()[i];
	}
	*con << ") values (";
	for ( int i=0; i< desc->num_fields(); ++i ){
		if ( i>0 ){
			*con << ",";
		}

		std::string val = boost::lexical_cast<string>( rand() );
		vals.push_back( val );
		*con << sqlite::q(val);
	}
	*con << ")";

	con->exec<sqlite::none>();
	sqlite::id_t new_id = con->insertid();
        T ft = con->load<T>( new_id );

	ft.set_db_id( 0 );

	*con << "delete from " << desc->table_name() << " where rowid=" << new_id;
	con->exec<sqlite::none>();

	ft.save();
	new_id = ft.db_id();

	for ( int i=0; i< desc->num_fields(); ++i ){
		*con << "select " << desc->fields()[i] << " from " << desc->table_name() << " where rowid=" << new_id;
		std::string ret=con->exec<std::string>();
		if  ( vals[i] != ret ){
			std::string msg="Table: ";
			msg+=desc->table_name();
			msg+=" / Field: ";
			msg+=desc->fields()[i];
			msg+=" ";
			msg+=vals[i];
			msg+=" != ";
			msg+=ret;
			throw std::runtime_error( msg );
		}
	}
	return true;
}

#endif /* _TESTING_H */

