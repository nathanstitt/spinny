#include "testing.hpp"
#include "sqlite_testing.hpp"


SUITE(SqliteTransaction) {

TEST( Begin ) {
	Conn c;
	transaction t(c.con);
	c.con.exec<none>("insert into foo values( 42, 'billy goat' )");
	CHECK_EQUAL( c.con.exec<int>("select * from foo limit 1"), 42 );
	t.rollback();
	CHECK_EQUAL( c.con.exec<int>("select * from foo limit 1"), 0 );
}

TEST( COMMIT ){
	Conn a;
	transaction t(a.con);
	a.con.exec<none>("insert into foo values( 42, 'billy goat' )");
	CHECK_EQUAL( a.con.exec<int>("select * from foo limit 1"), 42 );

	connection b( SQLITE_TEST_FILE );
	CHECK_EQUAL( b.exec<int>("select * from foo limit 1"), 0 );

	t.commit();

	CHECK_EQUAL( b.exec<int>("select * from foo limit 1"), 42 );
}

TEST( RollBack ){
	Conn a;
	transaction t(a.con);
	a.con.exec<none>("insert into foo values( 42, 'billy goat' )");
	CHECK_EQUAL( a.con.exec<int>("select * from foo limit 1"), 42 );

	connection b( SQLITE_TEST_FILE );
	CHECK_EQUAL( b.exec<int>("select * from foo limit 1"), 0 );

	t.rollback();

	CHECK_EQUAL( b.exec<int>("select * from foo limit 1"), 0 );
	CHECK_EQUAL( a.con.exec<int>("select * from foo limit 1"), 0 );
}


} //  SUITE(SqliteTransaction)



int
sqlite_transaction( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SqliteTransaction");
}
