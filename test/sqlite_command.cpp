#include "testing.hpp"
#include "sqlite_testing.hpp"

using namespace	sqlite;

SUITE(SqliteCommand) {

TEST( Bind ){
	DummyApp c;
	*c.con << "insert into testing (col1) values ( ? )";
	command cmd(*c.con);

	cmd.bind(1, 42);
	cmd.exec<none>();
   	cmd.bind(1, 36);
	cmd.exec<sqlite::none>();
 	cmd.bind(1, 23);
 	cmd.exec<sqlite::none>();
	CHECK_EQUAL( 101, c.con->exec<int>("select sum(col1) from testing") );
}

TEST( Exec ){
	DummyApp c;
	command cmd( *c.con, "insert into testing (col1,col2) values ( 42,'billy goat' )" );
	cmd.exec<none>();
}

TEST( Close ){
	DummyApp c;
	command cmd(*c.con,"insert into testing (col1) values ( ? )");
	cmd.bind(1, 42);
	cmd.exec<sqlite::none>();
  	cmd.bind(1, 36);
	cmd.exec<sqlite::none>();
	cmd.close();
	CHECK_THROW( cmd.bind(1, 42), sqlite::database_error );
}

TEST( Iterate ){

	DummyApp c;
	{
		*c.con << "insert into testing (col1) values ( ? )";
		command cmd(*c.con);
		cmd.bind(1, 42);
		cmd.exec<sqlite::none>();
		cmd.bind(1, 36);
		cmd.exec<sqlite::none>();
		cmd.bind(1, 23);
		cmd.exec<sqlite::none>();
	}
	command cmd(*c.con,"select sum(col1) from testing");
	command::iterator it=cmd.begin();
	CHECK_EQUAL( 101, it->get<int>(0) );

	command c2(*c.con,"select col1 from testing order by col1");
	command::iterator it2=c2.begin();

	CHECK( it2 != c2.end() );
	CHECK_EQUAL( 23, it2->get<int>(0) );
	++it2;
	CHECK( it2 != c2.end() );
	CHECK_EQUAL( 36, it2->get<int>(0) );

	++it2;
	CHECK( it2 != c2.end() );
	CHECK_EQUAL( 42, it2->get<int>(0) );
	++it2;

	CHECK( it2 == c2.end() );

	// but can we start over with the results?
	it2=c2.begin();
	CHECK( it2 != c2.end() );
	CHECK_EQUAL( 23, it2->get<int>(0) );
	
}

TEST( InsertTemplatedValues ) {
	DummyApp c;
	*c.con << "insert into testing (col1,col2, col3) values ( ?,?,? )";
	command cmd(*c.con);
	FooDoggy fd;
	fd.name="Bowzer";
	for( int i=0; i< 10 ; ++i ){
		fd.age=rand()*20;
		cmd.bind<FooDoggy>(fd);
		cmd.exec<none>();
	}

	CHECK_EQUAL( 10, c.con->exec<int>("select count(col1) from testing") );
	CHECK_EQUAL( "Bowzer", c.con->exec<string>("select col2 from testing") );
}


TEST( SelectTemplatedValues ) {
	DummyApp c;
	c.con->exec<none>("insert into testing (col1,col2,col3) values( 23,'BowWow', 42 )");

	command cmd(*c.con,"select col1,col2,col3,rowid from testing");

 	FooDoggy fd( cmd.begin()->get<FooDoggy>() );
//  	CHECK_EQUAL( 23, fd.age );
//  	CHECK_EQUAL( "BowWow", fd.name );
}


} //  SUITE(SqliteCommand)



int
sqlite_command( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SqliteCommand");
}
