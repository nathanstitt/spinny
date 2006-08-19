#include "testing.hpp"

#include "music_dir.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"

#include "config.h"

SUITE(SpinnyMusicDir) {


TEST( CreateRoot ){
	DummyApp da;
	boost::filesystem::path mpath( TESTING_FIXTURES_PATH );
	mpath/="music";
	MusicDir md = MusicDir::create_root( mpath );
  	CHECK( md.is_root() );

	std::vector<MusicDir> roots=MusicDir::roots();

	CHECK( roots.front() == md );
	CHECK( ! roots.empty() );
	CHECK( roots.end() != std::find( roots.begin(), roots.end(), md ) );
}

TEST( Name ){
	DummyApp da;
	da.con->exec<sqlite::none>( "insert into music_dir (parent_id, name ) values ( 340, 'foo' )" );
	sqlite::id_t id = da.con->insertid();
	MusicDir md = da.con->load<MusicDir>( id  );
	CHECK_EQUAL( "foo", md.name() );
}

TEST( Parent ){
	DummyApp da;

	da.con->exec<sqlite::none>( "insert into music_dir (parent_id,name ) values ( 0, 'parent' )" );
	sqlite::id_t parent_id = da.con->insertid();
	*da.con << "insert into music_dir (parent_id,name ) values ( " << parent_id << ", 'child' )";
	da.con->exec<sqlite::none>();
	sqlite::id_t child_id = da.con->insertid();


	MusicDir parent = da.con->load<MusicDir>( parent_id );
	MusicDir child  = da.con->load<MusicDir>( child_id );
	CHECK( parent.is_root() ); 
	CHECK_EQUAL( "parent", parent.name() );
	CHECK_EQUAL( parent_id, parent.db_id() );
	CHECK_EQUAL( "child",  child.name() );
	CHECK_EQUAL( child_id, child.db_id() );
	
	MusicDir	loaded_parent = child.parent();
	CHECK_EQUAL( loaded_parent.name(), parent.name() );
	CHECK_EQUAL( loaded_parent.db_id(),parent.db_id() );
	CHECK( loaded_parent ==  parent );
}

} // SUITE(SpinnyMusicDir)




int
spinny_music_dir( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("SpinnyMusicDir");
}
