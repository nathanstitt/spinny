#include <iostream>
#include <stdexcept>
using namespace std;

#include "sqlite/sqlite.hpp"

#include "sqlite_db_utils.hpp"

using namespace sqlite;


int
sqlite_select( int argc, char *argv[] )
{
	bool failed=false;
	sqlite::connection con("test.db");
	populate_db( con );

	con << "select * from people_test;";
	command cmd( con );
	reader r=cmd.exec<reader>();
	int age=1;
	while(r.read()){
		FooDoggy fd=r.get<FooDoggy>();
		assert( fd.age == age++ );
		assert( fd.name == "Bar" );
	}

	FooDoggy ba=con.find_by_field<FooDoggy>( "age", 345 );

	cmd.close();
	remove_db( con );
	return failed;
}
