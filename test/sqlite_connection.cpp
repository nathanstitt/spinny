#include "testing.hpp"

using namespace sqlite;

SUITE(SqliteConnection) {

TEST( Open ) {
	connection con;
	CHECK( ! filesystem::exists(SQLITE_TEST_DB_FILE) );
	con.open( SQLITE_TEST_DB_FILE );
	CHECK( filesystem::exists(SQLITE_TEST_DB_FILE) );
	con.close();
	CHECK( filesystem::remove( SQLITE_TEST_DB_FILE ) );
}

TEST( ExecWithArg){
	DummyApp c;
	c.con->exec<none>("insert into testing (col1,col2) values (42,'billy goat')");
	CHECK_EQUAL( c.con->exec<int>("select * from testing limit 1"), 42 );
}

TEST( ExecWithoutArg){
	DummyApp c;
	*c.con << "insert into testing (col1,col2) values ( 87,'billy goat' )";
	c.con->exec<none>();
	*c.con << "select * from testing limit 1";
	CHECK_EQUAL( c.con->exec<int>(), 87 );
}

TEST( Close ){
	connection con( SQLITE_TEST_DB_FILE );
	con.exec<none>("create table foo1(int)");
	con.close();
	CHECK_THROW( con.exec<none>("create table foo2(int)"), database_error );
	CHECK( filesystem::remove( SQLITE_TEST_DB_FILE ) );
}

TEST( InsertID ){
	DummyApp c;
	CHECK_EQUAL( c.con->insertid(), 0 );
	*c.con << "insert into testing (col1,col2) values (34,'billy goat')";
	c.con->exec<none>();
	CHECK( c.con->insertid() > 0 );
}

TEST( DBCreate ){
	{
		connection con( SQLITE_TEST_DB_FILE );
	}
	CHECK( filesystem::exists( SQLITE_TEST_DB_FILE ) );
	CHECK( filesystem::remove( SQLITE_TEST_DB_FILE ) );
}

TEST( Dump ){
// how to test this?
// settle for compilation	
	DummyApp c;
	c.con->log_table_contents( "testing" );
	
}

TEST( Stream ) {
	connection con;
	con << "0123456789";
	CHECK_EQUAL( con.current_statement(), "0123456789" );
	con.clear_cmd();
	CHECK_EQUAL( con.current_statement(), "" );
}

TEST( Save ){
	DummyApp c;
	

}


} // SUITE(SqliteConnection)





int
sqlite_connection( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SqliteConnection");

}
