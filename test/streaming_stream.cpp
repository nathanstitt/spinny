#include "testing.hpp"
#include "spinny/playlist.hpp"
#include "streaming_client.hpp"
#include "boost/filesystem/path.hpp"
#include "streaming/stream.hpp"
#include <fstream>
using namespace Spinny;

SUITE(StreamingStream) {




TEST( Decode ){
	DummyApp da;
	EnableLogging el("strm");

	try {
		StreamingTestClient stc;

		std::vector<unsigned short int> data = stc.read( 10 );
	} 
	catch ( const StreamingTestClient::error &e ){

	}
}



}



int
streaming_stream( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("StreamingStream");
}

