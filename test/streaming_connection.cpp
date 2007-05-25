#include "testing.hpp"
#include "boost/filesystem/fstream.hpp"
#include "spinny/playlist.hpp"
#include "streaming/lame.hpp"
#include "streaming_client.hpp"
#include "boost/filesystem/path.hpp"
#include "streaming/stream.hpp"
#include <fstream>
using namespace Spinny;

SUITE(StreamingConnection) {

void setup( DummyApp &da ){
	da.populate_fixture("music");
	Spinny::MusicDir::ptr md = Spinny::MusicDir::create_root( da.music_path );
	md->sync();

	Spinny::Song::result_set songs = md->songs();
 	Spinny::PlayList::ptr pl = Spinny::PlayList::create( std::string("24 Kbs"), std::string("BooYah"), 24 );

	pl->save();

	for ( Spinny::Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
		pl->insert( s.shared_ptr(), 0 );
	}

}

TEST( Icy ){
	DummyApp da;
	EnableLogging el("strm");
	setup(da);
 	Spinny::PlayList::ptr pl = Spinny::PlayList::load( 2 );

	try {
		boost::filesystem::ofstream os(  da.fixtures_path / "lame-test.mp3", std::ios::out | std::ios::binary );
		unsigned int seconds=0;
		int read=0;
    
		{
			Streaming::Lame l(pl);
			while ( read < 1024*1024*1 ) {
				Streaming::Chunk c = l.get_chunk();
				read += c.size();
				seconds+=c.milliseconds();
				CHECK( c.size() );
				os.write( asio::buffer_cast<const char*>(c.data), c.size() );
			}
		}
	} 
	catch ( const StreamingTestClient::error &e ){

	}
}



}



int
streaming_connection( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("StreamingConnection");
}

