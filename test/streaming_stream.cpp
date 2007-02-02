#include "testing.hpp"
#include "spinny/playlist.hpp"
#include "boost/filesystem/path.hpp"
#include "streaming/stream.hpp"
#include <fstream>
using namespace Spinny;

SUITE(Streaming) {


TEST( Decode ){
	DummyApp da;
	EnableLogging el("strm");

// 	da.populate_fixture("music");

// 	MusicDir::ptr md = MusicDir::create_root( da.music_path );

// 	md->save();
//  	CHECK_THROW( Song::create_from_file( *md, "song.mp3" ), Song::file_error );

// 	Song::ptr song = Song::create_from_file( *md,"sonny_stitt.mp3" );

//  	PlayList::ptr pl;// = PlayList::ptrcreate( 128,  std::string("128 Kbs"), std::string("BooYah") );

// // 	pl->insert( song, 0 );
// // 	pl->save();

// 	Streaming::Stream s( pl );

// 	s.decode_song( song );


// 	std::fstream out("/tmp/file.wav", ios::out );
// 	out.write( s.mp3_data, s.mp3_data_len );
	//CHECK( data );
}



}



int
streaming_stream( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("Streaming");
}

