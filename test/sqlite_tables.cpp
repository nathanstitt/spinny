
#include "testing.hpp"
#include "sqlite_testing.hpp"

using namespace Spinny;

SUITE(SqliteTables) {


TEST( Register ) {
	DummyApp c;

	typedef std::list<sqlite::table::description*> tl;
	tl *tables = sqlite::register_db_table_check( NULL );

	CHECK( ! tables->empty() );
	CHECK( tables->end() != std::find( tables->begin(), tables->end(), &foo_desc ) );
}


TEST( Check ){
	DummyApp c;
	*c.con << "insert into foo_table (parent_id,name) values ( 3, 'basd' )";
	c.con->exec<sqlite::none>();
}

TEST( SaveNew ){
	DummyApp da;
	FooTable ft;
	ft.parent_id = 43;
	ft.name="testing foo table";
	CHECK( ft.save() );
	CHECK( ft.db_id() );

	CHECK_EQUAL( 43, sqlite::command( *da.con,"select parent_id from foo_table" ).exec<int>() );

	*da.con << "select parent_id,name from foo_table where rowid=" << ft.db_id();
	sqlite::command cmd(*da.con);
	sqlite::command::reader_iterator row=cmd.reader_begin();
	CHECK_EQUAL( 43, row->get<int>(0) );
	CHECK_EQUAL( "testing foo table", row->get<std::string>(1) );

	boost::shared_ptr<FooTable> ft2=da.con->load<FooTable>( ft.db_id() );

	CHECK_EQUAL( ft.db_id(), ft2->db_id() );
	CHECK_EQUAL( ft.parent_id, ft2->parent_id );
	CHECK_EQUAL( ft.name, ft2->name );
}

TEST( SaveExisting ){
	DummyApp da;
	FooTable ft;
	ft.parent_id = 43;
	ft.name="testing foo table";
	CHECK( ! ft.db_id() );
	CHECK( ft.save() );
	CHECK( ft.db_id() );

	CHECK_EQUAL( ft.name, "testing foo table" );
	ft.name="Green";
	CHECK( ft.save() );

	boost::shared_ptr<FooTable> ft2=da.con->load<FooTable>( ft.db_id() );

	CHECK_EQUAL( ft.db_id(), ft2->db_id() );
	CHECK_EQUAL( ft.parent_id, ft2->parent_id );
	CHECK_EQUAL( "Green", ft.name );

}

TEST( Destroy ){

	DummyApp da;
	FooTable ft;
	ft.parent_id = 43;
	ft.name="testing foo table";
	CHECK( ! ft.db_id() );
	CHECK( ft.save() );
	CHECK( ft.db_id() );

	CHECK_EQUAL( 1, da.con->exec<int>( "select count(*) from foo_table") );

	ft.destroy();

	CHECK_EQUAL( 0, da.con->exec<int>( "select count(*) from foo_table") );
}


TEST( Comprehensive ) {
	DummyApp da;
	CHECK( da.test_table_obj<FooTable>() );
}

} // SUITE(SqliteTables)

int
sqlite_tables( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SqliteTables");

}
