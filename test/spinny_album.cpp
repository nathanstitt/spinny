#include "testing.hpp"
#include "album.hpp"
#include "artist.hpp"

SUITE(SpinnyAlbum) {


TEST( Table ){
	DummyApp da;
	CHECK( da.test_table_obj<Album>() );
}


TEST( FromSong ){

	DummyApp da;
	boost::filesystem::path mpath( TESTING_FIXTURES_PATH );
	MusicDir::ptr md = MusicDir::create_root( mpath / "music" );
	Song::ptr song = Song::create_from_file( *md, "sonny_stitt.mp3" );
	Album::ptr alb = song->album();

	CHECK_EQUAL( "The Bebop Recordings, 1949-1952", alb->name() );
}


TEST( Songs ){

	DummyApp da;
	boost::filesystem::path mpath( TESTING_FIXTURES_PATH );
	MusicDir::ptr md = MusicDir::create_root( mpath / "music" );
	Song::ptr song = Song::create_from_file( *md, "sonny_stitt.mp3" );
	Album::ptr alb = song->album();

	CHECK_EQUAL( "The Bebop Recordings, 1949-1952", alb->name() );
	Song::result_set songs = alb->songs();
	CHECK( songs.begin() != songs.end() );

	CHECK_EQUAL( "Blue Mode (Take 1)", songs.begin()->title() );
}


TEST( All ){
	DummyApp da;
	boost::filesystem::path mpath( TESTING_FIXTURES_PATH );
	MusicDir::ptr md = MusicDir::create_root( mpath / "music" );
	Song::ptr song = Song::create_from_file( *md, "sonny_stitt.mp3" );

	Album::result_set albums = Album::all();

   	CHECK( albums.begin() != albums.end() );

	Album::ptr alb=albums.begin().shared_ptr();

	CHECK_EQUAL( "The Bebop Recordings, 1949-1952", alb->name() );

}
struct name_eq{
	name_eq( const string &s ) : an(s){}
	string an;
	bool operator()( const Artist &s ){
		return s.name() == an;
	}
};

TEST( Artists ){

	DummyApp da;
	boost::filesystem::path mpath( TESTING_FIXTURES_PATH );
	MusicDir::ptr md = MusicDir::create_root( mpath / "music" );
	md->save();
	Song::ptr song = Song::create_from_file( *md, "sonny_stitt.mp3" );

  	Artist::result_set rs = Artist::all();
  	CHECK( rs.begin() != rs.end() );

 	Album::ptr alb = song->album();
  	CHECK_EQUAL( "The Bebop Recordings, 1949-1952", alb->name() );

  	rs = alb->artists();


   	CHECK( rs.begin() != rs.end() );
 	
 	CHECK( std::find_if( rs.begin(), rs.end(), name_eq("Sonny Stitt") ) != rs.end() );

}

}



int
spinny_album( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("SpinnyAlbum");
}

