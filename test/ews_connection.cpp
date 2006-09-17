
#include "testing.hpp"


SUITE(EwsConnection) {
	
TEST( Start ){
	DummyApp da;


}

} // SUITE(SqliteTables)

int
ews_connection( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("EwsConnection");

}
