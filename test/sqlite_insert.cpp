#include <string>
#include <iostream>
#include <stdexcept>
using namespace std;

#include "sqlite/sqlite.hpp"
#include "sqlite_db_utils.hpp"

using namespace sqlite;

int
sqlite_insert( int argc, char *argv[] )
{
	bool failed=false;
	sqlite::connection con("test.db");

	populate_db( con );
 	con << "select * from people_test where age=548;";
 	FooDoggy fd=con.exec<FooDoggy>();
 	assert( fd.age == 548 );
	
	command cmd(con, "insert into people_test values(?,?);");
	cmd.bind(1, 883556 );
	cmd.bind(2, "FooField" );
 	cmd.exec<none>();
	cmd.close();

 	con << "select * from people_test where age=883556;";
 	fd=con.exec<FooDoggy>();
 	assert( fd.name == "FooField" );

	remove_db( con );
	return failed;
}
