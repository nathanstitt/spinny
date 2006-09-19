
#include "unit_test/test.h"

#include "google/sparse_hash_map"
#include "google/dense_hash_map"

SUITE(GoogleHashMaps) {

TEST( Sparse ){
        google::sparse_hash_map<int,std::string> hm;
        hm[ 1 ] = "bar";
        hm[ 2 ] = "foo";
        CHECK( hm.find( 1 ) != hm.end() );
        CHECK_EQUAL( "foo", hm[2] );
}

// TEST( Dense ){
//         google::dense_hash_map<int,std::string> hm;
//         hm[ 1 ] = "bar";
//         hm[ 2 ] = "foo";
//         CHECK( hm.find( 1 ) != hm.end() );
//         CHECK_EQUAL( "foo", hm[2] );
// }


} // SUITE(Google)

int
google_hash_maps( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("GoogleHashMaps");

}
