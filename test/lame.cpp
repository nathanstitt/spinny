
#include "testing.hpp"
#include "boost/filesystem/fstream.hpp"
#include <iostream>
#include "spinny/playlist.hpp"
#include "spinny/music_dir.hpp"
#include "streaming/lame.hpp"
using namespace std;


SUITE(LameTestSuit) {

TEST( Decode ){
 	DummyApp da;
	EnableLogging el("strm");

	da.populate_fixture("music");
	Spinny::MusicDir::ptr md = Spinny::MusicDir::create_root( da.music_path );
	md->sync();

	Spinny::Song::result_set songs = md->songs();
 	Spinny::PlayList::ptr pl = Spinny::PlayList::create( 24,  std::string("24 Kbs"), std::string("BooYah") );

	pl->save();

	for ( Spinny::Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
		pl->insert( s.shared_ptr(), 0 );
	}

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

	CHECK( boost::filesystem::file_size( da.fixtures_path / "lame-test.mp3" ) );
	CHECK( seconds > 352400 && seconds < 352500 );
	CHECK_EQUAL( read, 1057552 );

	os.close();
}


}


int
lame( int, char ** ) 
{
	return UnitTest::RunAllTests("LameTestSuit");
}
