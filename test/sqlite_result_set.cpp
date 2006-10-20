#include "testing.hpp"
#include "sqlite_testing.hpp"

using namespace	sqlite;

SUITE(SqliteResultSet) {

TEST( Where ){

	DummyApp c;

	c.con->exec<sqlite::none>( "insert into foo_table (parent_id,name) values ( 0, 'root' )" );
	c.con->exec<sqlite::none>( "insert into foo_table (parent_id,name) values ( 1, 'child1' )" );
	c.con->exec<sqlite::none>( "insert into foo_table (parent_id,name) values ( 2, 'child2' )" );

	sqlite::result_set<FooTable> rs = c.con->load_where<FooTable>("rowid < 200");

	sqlite::result_set<FooTable>::iterator it = rs.begin();

	FooTable &ft = *it;
	CHECK_EQUAL( "root", ft.name );
	CHECK( it != rs.end() );
	CHECK_EQUAL( "root", it->name );
	it++;

	CHECK_EQUAL( "child1", it->name );
	++it;
	CHECK_EQUAL( "child2", it->name );
	++it;
	CHECK( it == rs.end() );
}

TEST( Sort ){

	DummyApp c;

	c.con->exec<sqlite::none>( "insert into foo_table (parent_id,name) values ( 0, 'root' )" );
	c.con->exec<sqlite::none>( "insert into foo_table (parent_id,name) values ( 1, 'child1' )" );
	c.con->exec<sqlite::none>( "insert into foo_table (parent_id,name) values ( 2, 'child2' )" );

	sqlite::result_set<FooTable> rs = c.con->load_all<FooTable>("parent_id desc");
	sqlite::result_set<FooTable>::iterator it = rs.begin();

	FooTable &ft = *it;
	CHECK_EQUAL( "root", ft.name );
	CHECK( it != rs.end() );
	CHECK_EQUAL( "root", it->name );
	it++;

	CHECK_EQUAL( "child1", it->name );
	++it;
	CHECK_EQUAL( "child2", it->name );
	++it;
	CHECK( it == rs.end() );
}

TEST( All ){

	DummyApp c;

	c.con->exec<sqlite::none>( "insert into foo_table (parent_id,name) values ( 0, 'root' )" );
	c.con->exec<sqlite::none>( "insert into foo_table (parent_id,name) values ( 1, 'child1' )" );
	c.con->exec<sqlite::none>( "insert into foo_table (parent_id,name) values ( 2, 'child2' )" );

	sqlite::result_set<FooTable> rs = c.con->load_all<FooTable>();
	sqlite::result_set<FooTable>::iterator it = rs.begin();

	FooTable &ft = *it;
	CHECK_EQUAL( "root", ft.name );
	CHECK( it != rs.end() );
	CHECK_EQUAL( "root", it->name );
	it++;

	CHECK_EQUAL( "child1", it->name );
	++it;
	CHECK_EQUAL( "child2", it->name );
	++it;
	CHECK( it == rs.end() );
}

} //  SUITE(SqliteResultSet)



int
sqlite_result_set( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SqliteResultSet");
}
