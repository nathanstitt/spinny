

#include "testing.hpp"
#include "sqlite_testing.hpp"


SUITE(SqliteConnection) {

TEST( Open ) {
	connection con;
	CHECK( ! filesystem::exists(SQLITE_TEST_FILE) );
	con.open( SQLITE_TEST_FILE );
	CHECK( filesystem::exists(SQLITE_TEST_FILE) );
	con.close();
	CHECK( filesystem::remove( SQLITE_TEST_FILE ) );
}

TEST( ExecWithArg){
	Conn c;
	c.con.exec<none>("insert into foo values (42,'billy goat')");
	CHECK_EQUAL( c.con.exec<int>("select * from foo limit 1"), 42 );
}

TEST( ExecWithoutArg){
	Conn c;
	c.con << "insert into foo values ( 87,'billy goat' )";
	c.con.exec<none>();
	c.con << "select * from foo limit 1";
	CHECK_EQUAL( c.con.exec<int>(), 87 );
}

TEST( Close ){
	connection con( SQLITE_TEST_FILE );
	con.exec<none>("create table foo1(int)");
	con.close();
	CHECK_THROW( con.exec<none>("create table foo2(int)"), database_error );
	CHECK( filesystem::remove( SQLITE_TEST_FILE ) );
}

TEST( InsertID ){
	Conn c;
	CHECK_EQUAL( c.con.insertid(), 0 );
	c.con << "insert into foo values(34,'billy goat')";
	c.con.exec<none>();
	CHECK( c.con.insertid() > 0 );
}

TEST( DBCreate ){
	{
		connection con( SQLITE_TEST_FILE );
	}
	CHECK( filesystem::exists( SQLITE_TEST_FILE ) );
	CHECK( filesystem::remove( SQLITE_TEST_FILE ) );
}

TEST( Stream ) {
	connection con;
	con << "0123456789";
	CHECK_EQUAL( con.current_statement(), "0123456789" );
	con.clear_cmd();
	CHECK_EQUAL( con.current_statement(), "" );
}




} // SUITE(SqliteConnection)





int
sqlite_connection( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SqliteConnection");

}
