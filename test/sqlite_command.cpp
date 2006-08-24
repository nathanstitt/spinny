#include "testing.hpp"
#include "sqlite_testing.hpp"

using namespace	sqlite;

SUITE(SqliteCommand) {

TEST( Bind ){
	DummyApp c;
	*c.con << "insert into testing (col1) values ( ? )";
	command cmd(*c.con);

	cmd.bind(1, 42);
	cmd.exec<none>();
   	cmd.bind(1, 36);
	cmd.exec<sqlite::none>();
 	cmd.bind(1, 23);
 	cmd.exec<sqlite::none>();
	CHECK_EQUAL( 101, c.con->exec<int>("select sum(col1) from testing") );
}

TEST( Exec ){
	DummyApp c;
	command cmd( *c.con, "insert into testing (col1,col2) values ( 42,'billy goat' )" );
	cmd.exec<none>();
}

TEST( Close ){
	DummyApp c;
	command cmd(*c.con,"insert into testing (col1) values ( ? )");
	cmd.bind(1, 42);
	cmd.exec<sqlite::none>();
  	cmd.bind(1, 36);
	cmd.exec<sqlite::none>();
	cmd.close();
	CHECK_THROW( cmd.bind(1, 42), sqlite::database_error );
}

TEST( ReaderIterate ){
	DummyApp c;
	{
		*c.con << "insert into testing (col1) values ( ? )";
		command cmd(*c.con);
		cmd.bind(1, 42);
		cmd.exec<sqlite::none>();
		cmd.bind(1, 36);
		cmd.exec<sqlite::none>();
		cmd.bind(1, 23);
		cmd.exec<sqlite::none>();
	}
	command cmd(*c.con,"select sum(col1) from testing");
	command::reader_iterator it=cmd.reader_begin();
	CHECK_EQUAL( 101, it->get<int>(0) );

	// how about for no matches?
	command failing_cmd(*c.con,"select col1 from testing where col1=-1");
	CHECK( failing_cmd.reader_begin() == failing_cmd.reader_end() );

	command c2(*c.con,"select col1 from testing order by col1");
	command::reader_iterator it2=c2.reader_begin();

	CHECK( it2 != c2.reader_end() );
	CHECK_EQUAL( 23, it2->get<int>(0) );
	++it2;
	CHECK( it2 != c2.reader_end() );
	CHECK_EQUAL( 36, it2->get<int>(0) );

	++it2;
	CHECK( it2 != c2.reader_end() );
	CHECK_EQUAL( 42, it2->get<int>(0) );
	++it2;

	CHECK( it2 == c2.reader_end() );

	// but can we start over with the results?
	it2=c2.reader_begin();
	CHECK( it2 != c2.reader_end() );
	CHECK_EQUAL( 23, it2->get<int>(0) );
	
}

TEST( TmplIterate ){
	DummyApp c;

	c.con->exec<sqlite::none>( "insert into testing (col1,col2, col3) values ( 12,'Bowzer', 14 )" );

	command cmd( *c.con, "select col1,col2,col3,rowid from testing limit 1" );
	command::iterator<FooDoggy> it=cmd.begin<FooDoggy>();

	CHECK_EQUAL( "Bowzer", it->name );
	CHECK_EQUAL( 1, it->id );
	CHECK_EQUAL( 14, it->num_bones );

	// how about for no matches?
	command failing_cmd(*c.con,"select col1,col2,col3,rowid from testing where col1=-1");
	CHECK( failing_cmd.begin<int>() == failing_cmd.end<int>() );

 	std::vector<FooDoggy> v;

	CHECK( it != cmd.end<FooDoggy>() );
	v.push_back(*it);
	++it;
	CHECK( it == cmd.end<FooDoggy>() );

	command::iterator<FooDoggy> it2=cmd.begin<FooDoggy>();
	CHECK( it2 != cmd.end<FooDoggy>() );
	CHECK( it == cmd.end<FooDoggy>() );


//  	std::copy( cmd.begin<FooDoggy>(), cmd.end<FooDoggy>(), v.begin() );
 	cout << "SIZE: " << v.size() << endl;

}

TEST( InsertTemplatedValues ) {
	DummyApp c;
	*c.con << "insert into testing (col1,col2, col3) values ( ?,?,? )";
	command cmd(*c.con);
	FooDoggy fd;
	fd.name="Bowzer";
	for( int i=0; i< 10 ; ++i ){
		fd.age=rand()*20;
		cmd.bind<FooDoggy>(fd);
		cmd.exec<none>();
	}

	CHECK_EQUAL( 10, c.con->exec<int>("select count(col1) from testing") );
	CHECK_EQUAL( "Bowzer", c.con->exec<string>("select col2 from testing") );
}


TEST( SelectTemplatedValues ) {
	DummyApp c;
	c.con->exec<none>("insert into testing (col1,col2,col3) values( 23,'BowWow', 42 )");
	command cmd(*c.con,"select col1,col2,col3,rowid from testing");
 	FooDoggy fd = cmd.reader_begin()->get<FooDoggy>();

 	CHECK_EQUAL( 23, fd.age );
 	CHECK_EQUAL( "BowWow", fd.name );
}

TEST( RowColumnCounts ){
	DummyApp c;
	c.con->exec<none>("insert into testing (col1,col2,col3) values( 23,'BowWow', 42 )");
	c.con->exec<none>("insert into testing (col1,col2,col3) values( 23,'BowWow', 42 )");
	sqlite::command cmd( *c.con, "select col1,col3 from testing" );
	CHECK_EQUAL( 0, cmd.num_rows() );
	CHECK_EQUAL( 2, cmd.num_columns() );
}


} //  SUITE(SqliteCommand)



int
sqlite_command( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SqliteCommand");
}
