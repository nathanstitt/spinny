

#include "testing.hpp"

#include "spinny/playlist.hpp"
#include "spinny/song.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"



SUITE(SpinnyPlayList){


TEST( Load ){
	DummyApp da;

	PlayList::ptr p = PlayList::create( 128, "128 Kbs" );
	p->save();

 	PlayList::ptr pl = PlayList::load( p->db_id() );

 	CHECK_EQUAL( "128 Kbs", pl->name() );

	
}

TEST( Create ){
 	DummyApp da;
 	PlayList::ptr pl = PlayList::create( 128, "128 Kbs" );
	pl->save();
 	CHECK_EQUAL( "128 Kbs", pl->name() );
}

TEST( PushBack ){
 	DummyApp da;
	da.populate_music_fixtures();

	MusicDir::ptr md = MusicDir::create_root( da.music_path );
	md->sync();

	Song::result_set songs = md->songs();

 	PlayList::ptr pl = PlayList::create( 128, "128 Kbs" );

	for ( Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
		pl->push_back( *s );
	}

	CHECK_EQUAL( pl->size(), songs.size() );
}

TEST( Clear ){
 	DummyApp da;
	da.populate_music_fixtures();

	MusicDir::ptr md = MusicDir::create_root( da.music_path );
	md->sync();

	Song::result_set songs = md->songs();

 	PlayList::ptr pl = PlayList::create( 128, "128 Kbs" );

	for ( Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
		pl->push_back( *s );
	}

	CHECK_EQUAL( pl->size(), songs.size() );

	pl->clear();

	CHECK_EQUAL( 0, pl->size() );
}

TEST( Remove ){
 	DummyApp da;

	da.populate_music_fixtures();

	MusicDir::ptr md = MusicDir::create_root( da.music_path );
	md->sync();

	Song::result_set songs = md->songs();

 	PlayList::ptr pl = PlayList::create( 128, "128 Kbs" );

	for ( Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
		pl->push_back( *s );
	}

	CHECK_EQUAL( songs.size(),pl->size() );

	unsigned int every_other=0;
	for ( Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
		if ( ++every_other % 2 ){
			pl->remove( *s );
		}
	}

	CHECK_EQUAL( songs.size()/2, pl->size() );

}

} // SUITE(SpinnyPlayList)



int
spinny_playlist( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("SpinnyPlayList");
}
