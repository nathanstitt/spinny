#include "testing.hpp"

#include "song.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/fstream.hpp" 
#include "config.h"
#include <iostream>


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
	boost::filesystem::path mpath( TESTING_FIXTURES_PATH, boost::filesystem::native );
	mpath/="music";

	boost::filesystem::remove_all( mpath );

	boost::filesystem::create_directory( mpath );

	MusicDir md = MusicDir::create_root( mpath );

	CHECK_THROW( Song::create_from_file( md, "song.mp3" ), Song::file_error );
	
	boost::filesystem::ofstream file( mpath );

	CHECK( Song::create_from_file( md,"song.mp3" ) );

}
}



int
spinny_song( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("SpinnySong");
}

