#include "testing.hpp"
#include "spinny/playlist.hpp"
#include "streaming_client.hpp"
#include "boost/filesystem/path.hpp"
#include "streaming/stream.hpp"
#include <fstream>
#include "boost/lambda/lambda.hpp"
#include "boost/lambda/bind.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/xtime.hpp"

using namespace Spinny;
using namespace boost::lambda;

SUITE(StreamingStream) {




TEST( Decode ){
	DummyApp da;
	EnableLogging el("strm");

	da.populate_fixture("music");

	MusicDir::ptr md = MusicDir::create_root( da.music_path );
	md->sync();

 	Song::result_set songs = md->songs();

  	PlayList::ptr pl = PlayList::load( 1 );//create( 128,  std::string("128 Kbs"), std::string("BooYah") );

  	for ( Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
  		pl->insert( s.shared_ptr(), 0 );
  	}


 	StreamingTestClient stc;
	std::size_t read=0;


	unsigned int meta_pos = boost::lexical_cast<unsigned int>( stc.header.line[ "ICY-METAINT" ] );


   	CHECK_EQUAL( 200, stc.header.status );
  	CHECK( meta_pos );

 	std::vector<char> data( meta_pos+1 );

	boost::regex re(".*StreamTitle='.*");
	for ( unsigned int chunk_num = 0; chunk_num < 5; ++chunk_num ){
		cout << "-------------------------------------------------------" << endl;
		read = asio::read( stc.socket, asio::buffer( data ) );
		unsigned int meta_size = data.back()*16;
		cout << "Read " << read << " Meta size: " << meta_size << endl;
		CHECK( meta_size );
		std::string meta_string;
		if ( meta_size ){
			std::vector<char> meta( meta_size );
			read = asio::read( stc.socket, asio::buffer( meta ) );
			for ( unsigned int i = 0; i < meta_size; ++i ){
				cout << i << " : " << (int)meta[i] << " = " << meta[ i ] << endl;
				meta_string += meta[i];
			}
		}
		cout << "Got: " << meta_string << endl;
//		CHECK( boost::regex_match( meta_string, re ) );
	}
}

}

int
streaming_stream( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("StreamingStream");
}


