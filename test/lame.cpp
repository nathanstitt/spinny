
#include "testing.hpp"
#include "boost/filesystem/fstream.hpp"
#include <iostream>
#include "spinny/playlist.hpp"
#include "spinny/music_dir.hpp"
#include "streaming/lame.hpp"
using namespace std;

extern "C" {
#include "lame/lame.h"
}


int
lame( int, char ** ) 
{
 	DummyApp da;
	EnableLogging el("lame");
	da.populate_music_fixtures();

	
	Spinny::MusicDir::ptr md = Spinny::MusicDir::create_root( "/tmp/music" );
	md->sync();

	Spinny::Song::result_set songs = md->songs();
 	Spinny::PlayList::ptr pl = Spinny::PlayList::create( 128,  std::string("128 Kbs"), std::string("BooYah") );

	pl->save();

	for ( Spinny::Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
		pl->insert( s.shared_ptr(), 0 );
	}
	
	Lame l(pl);

	l.doall( "/tmp/out.mp3" );

	

}
