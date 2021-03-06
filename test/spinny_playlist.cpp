

#include "testing.hpp"

#include "spinny/playlist.hpp"
#include "spinny/song.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"


using namespace Spinny;

SUITE(SpinnyPlayList){


TEST( Load ){
	DummyApp da;

	PlayList::ptr p = PlayList::create( std::string("128 Kbs"), std::string("BooYah") );
	p->save();

 	PlayList::ptr pl = PlayList::load( p->db_id() );

 	CHECK_EQUAL( "128 Kbs", pl->name() );

	
}

TEST( Create ){
 	DummyApp da;
 	PlayList::ptr pl = PlayList::create( std::string("128 Kbs"), std::string("BooYah") );
	pl->save();
 	CHECK_EQUAL( "128 Kbs", pl->name() );
}

TEST( Insert ){
 	DummyApp da;
	da.populate_fixture("music");

	MusicDir::ptr md = MusicDir::create_root( da.music_path );
	md->sync();

	Song::result_set songs = md->songs();

 	PlayList::ptr pl = PlayList::create( std::string("128 Kbs"), std::string("BooYah") );

	for ( Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
		pl->insert( s.shared_ptr(), 0 );
	}

	CHECK_EQUAL( pl->size(), songs.size() );

	pl->clear();
	CHECK_EQUAL( pl->size(), 0 );

	pl->insert( md,0 );
	CHECK_EQUAL( pl->size(), songs.size() );

	PlayList::ptr pl2 = PlayList::create( std::string("128 Kbs"), std::string("BooYah") );
	pl2->insert(pl,0);
	CHECK_EQUAL( pl->size(), pl2->size() );
}

TEST( Clear ){
 	DummyApp da;
	da.populate_fixture("music");

	MusicDir::ptr md = MusicDir::create_root( da.music_path );
	md->sync();

	Song::result_set songs = md->songs();

 	PlayList::ptr pl = PlayList::create( std::string("128 Kbs"), std::string("BooYah") );

	for ( Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
		pl->insert( s.shared_ptr(), 0 );
	}

	CHECK_EQUAL( pl->size(), songs.size() );

	pl->clear();

	CHECK_EQUAL( 0, pl->size() );
}

TEST( Remove ){
 	DummyApp da;

	da.populate_fixture("music");

	MusicDir::ptr md = MusicDir::create_root( da.music_path );
	md->sync();

	Song::result_set songs = md->songs();

 	PlayList::ptr pl = PlayList::create( std::string("128 Kbs"), std::string("BooYah") );

	for ( Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
		pl->insert( s.shared_ptr(),0 );
	}

	CHECK_EQUAL( songs.size(),pl->size() );

	unsigned int every_other=0;
	for ( Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
		if ( ++every_other % 2 ){
			pl->remove( s->db_id() );
		}
	}

	CHECK_EQUAL( songs.size()/2, pl->size() );

}

TEST( Update ){
	DummyApp da;
 	PlayList::ptr pl = PlayList::create( std::string("128 Kbs"), std::string("BooYah") );
	pl->save();
 	CHECK_EQUAL( "128 Kbs", pl->name() );
	CHECK_EQUAL( "BooYah", pl->description() );

	pl->set_name("Funny Name");
	pl->set_description("A longer descrip is needed");

 	CHECK_EQUAL( "Funny Name", pl->name() );
	CHECK_EQUAL( "A longer descrip is needed", pl->description() );


	sqlite::id_t id = pl->db_id();

	PlayList::ptr pl2 = PlayList::load( id );
 	CHECK_EQUAL( "128 Kbs", pl2->name() );
	CHECK_EQUAL( "BooYah", pl2->description() );


	pl->save();
	PlayList::ptr pl3 = PlayList::load( id );
	CHECK_EQUAL( "Funny Name", pl3->name() );
	CHECK_EQUAL( "A longer descrip is needed", pl3->description() );

	
}

TEST( At ){
 	DummyApp da;
 	da.populate_fixture("music");

 	MusicDir::ptr md = MusicDir::create_root( da.music_path );
 	md->sync();

 	Song::result_set all_songs = md->songs();

  	PlayList::ptr pl = PlayList::create( std::string("128 Kbs"), std::string("BooYah") );
	int pos=1;
	pl->save();

	for ( Song::result_set::iterator s=all_songs.begin(); all_songs.end() != s; ++s ){
		pl->insert( s.shared_ptr(), pos++ );
	}

	CHECK_EQUAL( pl->size(), all_songs.size() );

	Song::result_set songs = pl->songs();
	Song::result_set::iterator song = songs.begin();
	
	for ( int i = 0; i < pl->size(); ++i ){
		Song::ptr s2 = pl->at( i );
		CHECK_EQUAL( song->db_id(), s2->db_id() );
		song++;
	}


}

} // SUITE(SpinnyPlayList)



int
spinny_playlist( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("SpinnyPlayList");
}
