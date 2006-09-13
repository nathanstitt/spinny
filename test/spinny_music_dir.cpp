#include "testing.hpp"

#include "music_dir.hpp"
#include "song.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"

#include "config.h"

SUITE(SpinnyMusicDir) {


TEST( CreateRoot ){
	DummyApp da;

	boost::filesystem::path mpath( TESTING_FIXTURES_PATH );
	mpath/="music";

	MusicDir::ptr md = MusicDir::create_root( mpath );
  	CHECK( md->is_root() );
	md->save();

 	MusicDir::result_set roots = MusicDir::roots(); 

  	CHECK( roots.begin() != roots.end() );

  	MusicDir::iterator it=roots.begin();

  	CHECK_EQUAL( mpath.string(), it->name() );

	CHECK( *it == *md );

	CHECK( roots.end() != std::find_if( roots.begin(), roots.end(), sqlite::dref_eq<MusicDir>(md) ) );
}

TEST( Load ){
	DummyApp da;

	boost::filesystem::path mpath( TESTING_FIXTURES_PATH );
	mpath/="music";
	MusicDir::ptr md1 = MusicDir::create_root( mpath );
	md1->save();
  	CHECK( md1->is_root() );

	MusicDir::ptr md2 = MusicDir::load( md1->db_id() );
	
	CHECK( md2 );

  	CHECK( *md1 == *md2 );
  	CHECK_EQUAL( md1->name(), md2->name() );
}

TEST( Name ){
	DummyApp da;
	da.con->exec<sqlite::none>( "insert into music_dirs (parent_id, name ) values ( 340, 'foo' )" );
	sqlite::id_t id = da.con->insertid();
	MusicDir::ptr md = da.con->load<MusicDir>( id  );
	CHECK_EQUAL( "foo", md->name() );
}

TEST( Parent ){
	DummyApp da;

	da.con->exec<sqlite::none>( "insert into music_dirs (parent_id,name ) values ( 0, 'parent' )" );
	sqlite::id_t parent_id = da.con->insertid();
	*da.con << "insert into music_dirs (parent_id,name ) values ( " << parent_id << ", 'child' )";
	da.con->exec<sqlite::none>();
	sqlite::id_t child_id = da.con->insertid();


	MusicDir::ptr parent = da.con->load<MusicDir>( parent_id );
	MusicDir::ptr child  = da.con->load<MusicDir>( child_id );
	CHECK( parent->is_root() ); 
	CHECK_EQUAL( "parent", parent->name() );
	CHECK_EQUAL( parent_id, parent->db_id() );
	CHECK_EQUAL( "child",  child->name() );
	CHECK_EQUAL( child_id, child->db_id() );
	
	MusicDir::ptr loaded_parent = child->parent();
	CHECK_EQUAL( loaded_parent->name(), parent->name() );
	CHECK_EQUAL( loaded_parent->db_id(),parent->db_id() );
	CHECK( *loaded_parent ==  *parent );
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

	MusicDir::ptr parent = da.con->load<MusicDir>( parent_id );
	CHECK_EQUAL( parent_id, parent->db_id() );
	MusicDir::ptr child  = da.con->load<MusicDir>( child_id );

 	MusicDir::result_set children=parent->children();
 	CHECK( children.end() != children.begin() );

 	CHECK( std::find_if( children.begin(), children.end(), sqlite::dref_eq<MusicDir>(child) ) != children.end() );
}


TEST( Path ){
	DummyApp da;

	boost::filesystem::path orig_path( TESTING_FIXTURES_PATH );
	orig_path/="music";
	MusicDir::ptr md = MusicDir::create_root( orig_path );
  	CHECK( md->is_root() );
	CHECK( orig_path == md->path() ); 

}

TEST( Sync ){
	DummyApp da;

	boost::filesystem::path mpath( TESTING_FIXTURES_PATH );
	mpath/="music";

	MusicDir::ptr md = MusicDir::create_root( mpath );
  	CHECK( md->is_root() );
	md->save();
	MusicDir::result_set childs = md->children();
	MusicDir::result_set::iterator it = childs.begin();

	CHECK(  it == childs.end() );

	boost::filesystem::create_directory( mpath / "foo" );

	md->sync();

	CHECK_EQUAL( 2, da.con->exec<int>( "select count(*) from music_dirs") );
	CHECK_EQUAL( 9, da.con->exec<int>( "select count(*) from songs") );
	CHECK_EQUAL( 5, da.con->exec<int>( "select count(*) from artists") );
	CHECK_EQUAL( 5, da.con->exec<int>( "select count(*) from albums") );
	CHECK_EQUAL( 5, da.con->exec<int>( "select count(*) from albums_artists") );

}

struct name_eq{
	bool operator()( Song &s ){
		return s.title() == "Blue Mode (Take 1)";
	}
};

TEST( Songs ){
 	DummyApp da;
 	boost::filesystem::path mpath( TESTING_FIXTURES_PATH );
 	MusicDir::ptr md = MusicDir::create_root( mpath / "music" );
 	md->save();
 	md->sync();
 	Song::result_set songs = md->songs();
 	CHECK( songs.begin() != songs.end() );
 	CHECK( std::find_if( songs.begin(), songs.end(), name_eq() ) != songs.end() );
}



} // SUITE(SpinnyMusicDir)




int
spinny_music_dir( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("SpinnyMusicDir");
}
