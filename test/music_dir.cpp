#include "testing.hpp"

#include "music_dir.hpp"

SUITE(MusicDir) {



TEST( ById ){

// 	sqlite::command cmd( *(Spinny::db()), "insert into foo values ( 42,'billy goat' )" );
	
// 	md.name="AllMusic";
// 	md.set_as_root();
//	md.set_parent_id( 
//	MusicDir md=MusicDir::with_id( 
}


TEST( isRoot ){
// 	MusicDir md;
// 	CHECK( ! md.is_root() );
// 	md.set_as_root();
// 	CHECK( MD.is_root() );
}


} // SUITE(MusicDir)




int
music_dir( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("MusicDir");
}
