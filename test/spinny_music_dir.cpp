#include "testing.hpp"

#include "music_dir.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"

#include "config.h"

SUITE(SpinnyMusicDir) {


TEST( CreateRoot ){
	DummyApp da;

	boost::filesystem::path mpath( TESTING_FIXTURES_PATH, boost::filesystem::native );
	mpath/="music";
	MusicDir md = MusicDir::create_root( mpath );
  	CHECK( md.is_root() );

	MusicDir::result_set roots = MusicDir::roots(); 

	CHECK( roots.begin() != roots.end() );
	MusicDir::iterator it=roots.begin();

	CHECK( *it == md );

	CHECK( roots.end() != std::find( roots.begin(), roots.end(), md ) );
}

TEST( Name ){
	DummyApp da;
	da.con->exec<sqlite::none>( "insert into music_dirs (parent_id, name ) values ( 340, 'foo' )" );
	sqlite::id_t id = da.con->insertid();
	MusicDir md = da.con->load<MusicDir>( id  );
	CHECK_EQUAL( "foo", md.name() );
}

TEST( Parent ){
	DummyApp da;

	da.con->exec<sqlite::none>( "insert into music_dirs (parent_id,name ) values ( 0, 'parent' )" );
	sqlite::id_t parent_id = da.con->insertid();
	*da.con << "insert into music_dirs (parent_id,name ) values ( " << parent_id << ", 'child' )";
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

TEST( Table ){
	DummyApp da;
	CHECK( da.test_table_obj<MusicDir>() );
}

TEST( Children ){
	DummyApp da;

	da.con->exec<sqlite::none>( "insert into music_dirs ( parent_id,name ) values ( 0, 'parent' )" );
	sqlite::id_t parent_id = da.con->insertid();
	CHECK( parent_id );
	*da.con << "insert into music_dirs ( parent_id,name ) values ( " << parent_id << ", 'child' )";
	da.con->exec<sqlite::none>();
	sqlite::id_t child_id = da.con->insertid();

	MusicDir parent = da.con->load<MusicDir>( parent_id );
	CHECK_EQUAL( parent_id, parent.db_id() );
	MusicDir child  = da.con->load<MusicDir>( child_id );

	MusicDir::result_set children=parent.children();
	CHECK( children.end() != children.begin() );

	CHECK( std::find( children.begin(), children.end(), child ) != children.end() );
}


TEST( Path ){
	DummyApp da;

	boost::filesystem::path orig_path( TESTING_FIXTURES_PATH, boost::filesystem::native );
	orig_path/="music";
	MusicDir md = MusicDir::create_root( orig_path );
  	CHECK( md.is_root() );

	CHECK( orig_path == md.path() );

}


TEST( Sync ){
	DummyApp da;

	boost::filesystem::path mpath( TESTING_FIXTURES_PATH, boost::filesystem::native );
	mpath/="music";

	if ( ! boost::filesystem::exists(mpath ) ){
		boost::filesystem::create_directory( mpath );
	}

	MusicDir md = MusicDir::create_root( mpath );
  	CHECK( md.is_root() );

	MusicDir::result_set childs = md.children();
	MusicDir::result_set::iterator it = childs.begin();

	CHECK(  it == childs.end() );

	boost::filesystem::create_directory( mpath / "foo" );

	md.sync();

	MusicDir::result_set childs2 = md.children();

 	CHECK( childs2.begin() != childs2.end() );

 	CHECK_EQUAL( "foo", childs2.begin()->name() );

}

} // SUITE(SpinnyMusicDir)




int
spinny_music_dir( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("SpinnyMusicDir");
}
