#include "testing.hpp"

using namespace	sqlite;

SUITE(SqliteTransaction) {

TEST( Begin ) {
	DummyApp c;
	transaction t(*c.con);
	c.con->exec<sqlite::none>("insert into testing (col1,col2) values( 42, 'billy goat' )");
	CHECK_EQUAL( c.con->exec<int>("select * from testing limit 1"), 42 );
	t.rollback();
	CHECK_EQUAL( c.con->exec<int>("select * from testing limit 1"), 0 );
}

TEST( COMMIT ){
	DummyApp a;
	transaction t(*a.con);
	a.con->exec<sqlite::none>("insert into testing (col1,col2) values( 42, 'billy goat' )");
 	CHECK_EQUAL( a.con->exec<int>("select * from testing limit 1"), 42 );
 	connection b( SQLITE_TEST_DB_FILE );
 	CHECK_EQUAL( b.exec<int>("select * from testing limit 1"), 0 );
 	t.commit();
 	CHECK_EQUAL( b.exec<int>("select * from testing limit 1"), 42 );
}

TEST( RollBack ){
	DummyApp a;
	transaction t(*a.con);
	a.con->exec<sqlite::none>("insert into testing (col1,col2) values( 42, 'billy goat' )");
	CHECK_EQUAL( a.con->exec<int>("select * from testing limit 1"), 42 );

	connection b( SQLITE_TEST_DB_FILE );
	CHECK_EQUAL( b.exec<int>("select * from testing limit 1"), 0 );

	t.rollback();

	CHECK_EQUAL( b.exec<int>("select * from testing limit 1"), 0 );
	CHECK_EQUAL( a.con->exec<int>("select * from testing limit 1"), 0 );
}


} //  SUITE(SqliteTransaction)



int
sqlite_transaction( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SqliteTransaction");
}
