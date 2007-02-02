#include "testing.hpp"
#include "spinny/album.hpp"
#include "spinny/artist.hpp"

SUITE(SpinnyAlbum) {

using namespace Spinny;

TEST( Table ){
	DummyApp da;
	CHECK( da.test_table_obj<Spinny::Album>() );
}


TEST( FromSong ){
	DummyApp da;
	da.populate_fixture("music");
	Spinny::MusicDir::ptr md = Spinny::MusicDir::create_root( da.music_path );

	Spinny::Song::ptr song = Spinny::Song::create_from_file( *md, "Sonny_Stitt.mp3" );
	Spinny::Album::ptr alb = song->album();

	CHECK_EQUAL( "The Bebop Recordings, 1949-1952", alb->name() );
}


TEST( Songs ){
	DummyApp da;
	da.populate_fixture("music");
	Spinny::MusicDir::ptr md = Spinny::MusicDir::create_root( da.music_path );

	Spinny::Song::ptr song = Spinny::Song::create_from_file( *md, "Sonny_Stitt.mp3" );
	Spinny::Album::ptr alb = song->album();

	CHECK_EQUAL( "The Bebop Recordings, 1949-1952", alb->name() );
	Song::result_set songs = alb->songs();
	CHECK( songs.begin() != songs.end() );

	CHECK_EQUAL( "Blue Mode (Take 1)", songs.begin()->title() );
}


TEST( All ){
	DummyApp da;
	da.populate_fixture("music");
	MusicDir::ptr md = MusicDir::create_root( da.music_path );

	Song::ptr song = Song::create_from_file( *md, "Sonny_Stitt.mp3" );

	Album::result_set albums = Album::all();

   	CHECK( albums.begin() != albums.end() );

	Album::ptr alb=albums.begin().shared_ptr();

	CHECK_EQUAL( "The Bebop Recordings, 1949-1952", alb->name() );

}

TEST( NameStartsWith ){
	DummyApp da;
	da.populate_fixture("music");
	MusicDir::ptr md = MusicDir::create_root( da.music_path );

	Song::ptr song = Song::create_from_file( *md, "Sonny_Stitt.mp3" );

	Album::result_set albums = Album::name_starts_with("THE");

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
	da.populate_fixture("music");
	MusicDir::ptr md = MusicDir::create_root( da.music_path );

	md->save();
	Song::ptr song = Song::create_from_file( *md, "Sonny_Stitt.mp3" );

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

