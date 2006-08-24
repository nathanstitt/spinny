#include "testing.hpp"

using namespace sqlite;

SUITE(SqliteReader) {

TEST( Get ){
	DummyApp c;
	c.con->exec<none>("insert into testing (col1,col2) values ( 42,'FooBear' )");
	command cmd(*c.con,"select col1,col2 from testing order by col1");
	command::reader_iterator it=cmd.reader_begin();
	// hehe
	reader &r=*it;

	CHECK_EQUAL( 42, r.get<int>(0) );
	CHECK_EQUAL( "FooBear", r.get<string>(1) );
	CHECK_EQUAL( 0, r.get<int>(1) );
	CHECK_EQUAL( "42", r.get<std::string>(0) );
	CHECK_EQUAL( 42, r.get<double>(0) );
}


TEST( ColName ){
	DummyApp c;
	c.con->exec<none>("insert into testing (col1,col2) values ( 42,'FooBear' )");
	command cmd(*c.con,"select col1,col2 from testing order by col1");
	command::reader_iterator it=cmd.reader_begin();

	// hehe
	reader &r=*it;

	CHECK_EQUAL( "col1", r.colname(0) );
	CHECK_EQUAL( "col2", r.colname(1) );
}

TEST( Equality ){
	DummyApp c;
	c.con->exec<none>("insert into testing (col1,col2) values ( 42,'FooBear' )");
	command cmd(*c.con,"select col1 from testing order by col1");

	command::reader_iterator it=cmd.reader_begin();
	CHECK( it != cmd.reader_end() );
	// hehe
	reader &r=*it;

	CHECK( r == 42 );

//	CHECK_EQUAL( 42, reader );
}

} //  SUITE(SqliteReader)



int
sqlite_reader( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SqliteReader");
}
