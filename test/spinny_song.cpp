#include "testing.hpp"

#include "spinny/song.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/fstream.hpp" 
//#include "config.h"
#include <iostream>


using namespace Spinny;

SUITE(SpinnySong) {


TEST( Table ){
	DummyApp da;
	CHECK( da.test_table_obj<Song>() );
}

TEST( Interesting ){
	DummyApp da;

	boost::filesystem::path mpath( TESTING_FIXTURES_PATH, boost::filesystem::native );
	mpath/="music";

	CHECK( ! Song::is_interesting( mpath ) );

	CHECK( Song::is_interesting( mpath/"song.mp3" ) );

	CHECK( Song::is_interesting( mpath/"song.mP3" ) );

	CHECK( Song::is_interesting( mpath/"song.MP3" ) );
}

TEST( Create ){
	DummyApp da;
	da.populate_music_fixtures();
	MusicDir::ptr md = MusicDir::create_root( da.music_path );

	md->save();
 	CHECK_THROW( Song::create_from_file( *md, "song.mp3" ), Song::file_error );

	Song::ptr song = Song::create_from_file( *md,"ozzy.mp3" );

	CHECK_EQUAL( "Bark At The Moon", song->title() );
}

TEST( SetFromTag ){
	DummyApp da;
	da.populate_music_fixtures();
	MusicDir::ptr md = MusicDir::create_root( da.music_path );

	md->save();
	Song::ptr song = Song::create_from_file( *md,"sonny_stitt.mp3" );

	CHECK_EQUAL( "Blue Mode (Take 1)", song->title() );
	CHECK_EQUAL( 3, song->track() );
	CHECK_EQUAL( 313, song->length() );
	CHECK_EQUAL( 128000, song->bitrate() );
	CHECK_EQUAL( 1952, song->year() );
}

}



int
spinny_song( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("SpinnySong");
}

