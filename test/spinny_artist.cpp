#include "testing.hpp"
#include "spinny/artist.hpp"

using namespace Spinny;

SUITE(SpinnyArtist) {


TEST( Table ){
	DummyApp da;
	CHECK( da.test_table_obj<Artist>() );
}

struct name_eq{
	name_eq( const string &s ) : an(s){}
	string an;
	bool operator()( const Artist &s ){
		return s.name() == an;
	}
};


TEST( Songs ){
	DummyApp da;
	da.populate_fixture("music");
	MusicDir::ptr md = MusicDir::create_root( da.music_path );
	md->sync();

 	Artist::result_set rs = Artist::all();

 	CHECK( rs.begin() != rs.end() );
 	
	CHECK( std::find_if( rs.begin(), rs.end(), name_eq("Ozzy Osbourne") ) != rs.end() );
}

TEST( NameStartsWith ){
	DummyApp da;
	da.populate_fixture("music");
	MusicDir::ptr md = MusicDir::create_root( da.music_path );
	md->sync();

 	Artist::result_set rs = Artist::name_starts_with( "O" );

 	CHECK( rs.begin() != rs.end() );
 	
	CHECK( std::find_if( rs.begin(), rs.end(), name_eq("Ozzy Osbourne") ) != rs.end() );
}

TEST( FromSong ){
	DummyApp da;
	da.populate_fixture("music");
	MusicDir::ptr md = MusicDir::create_root( da.music_path );

	Song::ptr song = Song::create_from_file( *md, "Sonny_Stitt.mp3" );
 
	Artist::ptr artist = song->artist();

	CHECK_EQUAL( "Sonny Stitt", artist->name() );

}


}



int
spinny_artist( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("SpinnyArtist");
}

