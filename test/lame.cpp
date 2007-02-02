
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
	EnableLogging el("lame");

	da.populate_fixture("music");
	Spinny::MusicDir::ptr md = Spinny::MusicDir::create_root( da.music_path );
	md->sync();

	Spinny::Song::result_set songs = md->songs();
 	Spinny::PlayList::ptr pl = Spinny::PlayList::create( 24,  std::string("24 Kbs"), std::string("BooYah") );

	pl->save();

	for ( Spinny::Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
		pl->insert( s.shared_ptr(), 0 );
	}
	
	
   	boost::filesystem::ofstream os("/tmp/out.mp3", std::ios::out | std::ios::binary);
    
	{
		Streaming::Lame l(pl);
		int read=0;

		while ( read < 1024*1024*1 ) {
			asio::const_buffer buf = l.get_chunk();
			read += asio::buffer_size( buf );
			os.write( asio::buffer_cast<const char*>(buf), asio::buffer_size( buf ) );
		}
		BOOST_LOGL(strm,info)<< "Out Loop";
	}
	BOOST_LOGL(strm,info)<< "shutdown";

	os.close();
}


}

int
lame( int, char ** ) 
{
	return UnitTest::RunAllTests("LameTestSuit");
}
