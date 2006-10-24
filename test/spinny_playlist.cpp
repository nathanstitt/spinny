

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
// 	PlayList::ptr pl = PlayList::create( 128, "128 Kbs" );
// 	CHECK_EQUAL( "128 Kbs", pl->name() );
}



} // SUITE(SpinnyPlayList)



int
spinny_playlist( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("SpinnyPlayList");
}
