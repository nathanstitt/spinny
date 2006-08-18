#include "testing.hpp"
#include "sqlite_testing.hpp"


SUITE(SqliteCommand) {

TEST( Bind ){
	Conn c;
	c.con << "insert into foo (col1) values ( ? )";
	command cmd(c.con);

	cmd.bind(1, 42);
	cmd.exec<none>();
   	cmd.bind(1, 36);
	cmd.exec<sqlite::none>();
 	cmd.bind(1, 23);
 	cmd.exec<sqlite::none>();
	CHECK_EQUAL( 101, c.con.exec<int>("select sum(col1) from foo") );
}

TEST( Exec ){
	Conn c;
	command cmd( c.con, "insert into foo values ( 42,'billy goat' )" );
	cmd.exec<none>();
}

TEST( Close ){
	Conn c;
	command cmd(c.con,"insert into foo (col1) values ( ? )");
	cmd.bind(1, 42);
	cmd.exec<sqlite::none>();
  	cmd.bind(1, 36);
	cmd.exec<sqlite::none>();
	cmd.close();
	CHECK_THROW( cmd.bind(1, 42), sqlite::database_error );
}

TEST( Iterate ){

	Conn c;
	{
		c.con << "insert into foo (col1) values ( ? )";
		command cmd(c.con);
		cmd.bind(1, 42);
		cmd.exec<sqlite::none>();
		cmd.bind(1, 36);
		cmd.exec<sqlite::none>();
		cmd.bind(1, 23);
		cmd.exec<sqlite::none>();
	}
	command cmd(c.con,"select sum(col1) from foo");
	command::iterator it=cmd.begin();
	CHECK_EQUAL( 101, it->get<int>() );

	command c2(c.con,"select col1 from foo order by col1");
	command::iterator it2=c2.begin();

	CHECK( it2 != c2.end() );
	CHECK_EQUAL( 23, it2->get<int>() );
	++it2;
	CHECK( it2 != c2.end() );
	CHECK_EQUAL( 36, it2->get<int>() );

	++it2;
	CHECK( it2 != c2.end() );
	CHECK_EQUAL( 42, it2->get<int>() );
	++it2;

	CHECK( it2 == c2.end() );

	// but can we start over with the results?
	it2=c2.begin();
	CHECK( it2 != c2.end() );
	CHECK_EQUAL( 23, it2->get<int>() );
	
}

TEST( InsertTemplatedValues ) {
	Conn c;
	c.con << "insert into foo (col1,col2) values ( ?,?  )";
	command cmd(c.con);
	FooDoggy fd;
	fd.name="Bowzer";
	for( int i=0; i< 10 ; ++i ){
		fd.age=rand()*20;
		cmd.bind<FooDoggy>(fd);
		cmd.exec<none>();
	}

	CHECK_EQUAL( 10, c.con.exec<int>("select count(col1) from foo") );
	CHECK_EQUAL( "Bowzer", c.con.exec<string>("select col2 from foo") );
}


TEST( SelectTemplatedValues ) {
	Conn c;
	c.con.exec<none>("insert into foo values( 23,'BowWow' )");
	command cmd(c.con,"select col1,col2 from foo");
	FooDoggy fd( cmd.begin()->get<FooDoggy>() );

 	CHECK_EQUAL( 23, fd.age );
 	CHECK_EQUAL( "BowWow", fd.name );
}


} //  SUITE(SqliteCommand)



int
sqlite_command( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SqliteCommand");
}
