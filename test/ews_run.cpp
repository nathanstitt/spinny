
#include "testing.hpp"

#include "ews_client.hpp"
#include "boost/filesystem/fstream.hpp"

SUITE(EwsRun) {
	
TEST( Start ){
	DummyApp da;
	EWSTestClient ews;

	CHECK_EQUAL( 404, ews.get( "/non/existant/url" ).status );
}

TEST( ServeFile ){


	DummyApp da;
	EWSTestClient ews;


	boost::filesystem::ofstream f( da.web_path / "test.txt" );
	f << "A Test";

	
}

} // SUITE(EwsRun)

int
ews_run( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("EwsRun");

}
