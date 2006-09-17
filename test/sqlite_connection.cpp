#include "testing.hpp"

using namespace sqlite;

SUITE(SqliteConnection) {

TEST( Open ) {
	boost::filesystem::path db("./test.db");
	connection con;
	CHECK( ! filesystem::exists( db.string() ) );
	con.open( db.string() );
	CHECK( filesystem::exists( db.string() ) );
	con.close();
	CHECK( filesystem::remove( db.string() ) );
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
	boost::filesystem::path db("./test.db");
	connection con( db.string() );
	con.exec<none>("create table foo1(int)");
	con.close();
	CHECK_THROW( con.exec<none>("create table foo2(int)"), database_error );
	CHECK( filesystem::remove( db.string() ) );
}

TEST( InsertID ){
	DummyApp c;
	CHECK_EQUAL( c.con->insertid(), 0 );
	*c.con << "insert into testing (col1,col2) values (34,'billy goat')";
	c.con->exec<none>();
	CHECK( c.con->insertid() > 0 );
}

TEST( DBCreate ){
	boost::filesystem::path db("./test.db");
	{
		connection con( db.string() );
	}
	CHECK( filesystem::exists( db.string() ) );
	CHECK( filesystem::remove( db.string() ) );
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
