#include "testing.hpp"
#include "artist.hpp"



SUITE(SpinnyArtist) {


TEST( Table ){
	DummyApp da;
	CHECK( da.test_table_obj<Artist>() );
}




}



int
spinny_artist( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("SpinnyArtist");
}

