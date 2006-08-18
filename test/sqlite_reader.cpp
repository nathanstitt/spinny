#include "testing.hpp"
#include "sqlite_testing.hpp"


SUITE(SqliteReader) {

TEST( Get ){
	Conn c;
	c.con.exec<none>("insert into foo (col1,col2) values ( 42,'FooBear' )");
	command cmd(c.con,"select col1,col2 from foo order by col1");
	command::iterator it=cmd.begin();
	// hehe
	reader &r=*it;

	CHECK_EQUAL( 42, r.get<int>(0) );
	CHECK_EQUAL( "FooBear", r.get<string>(1) );
	CHECK_EQUAL( 0, r.get<int>(1) );
	CHECK_EQUAL( "42", r.get<std::string>() );
	CHECK_EQUAL( 42, r.get<double>() );
}


TEST( ColName ){
	Conn c;
	command cmd(c.con,"select col1,col2 from foo order by col1");
	command::iterator it=cmd.begin();
	// hehe
	reader &r=*it;

	CHECK_EQUAL( "col1", r.colname(0) );
	CHECK_EQUAL( "col2", r.colname(1) );
}

TEST( Equality ){
	Conn c;
	c.con.exec<none>("insert into foo (col1,col2) values ( 42,'FooBear' )");
	command cmd(c.con,"select col1 from foo order by col1");
	command::iterator it=cmd.begin();
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
