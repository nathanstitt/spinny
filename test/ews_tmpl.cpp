
#include "testing.hpp"

#include "ews_client.hpp"
#include "boost/filesystem/fstream.hpp"
#include <boost/algorithm/string.hpp>

SUITE(EwsTmpl) {

	boost::filesystem::path template_path;

class TmplTestHandler
	: public ews::request_handler
{
	/// Handle a request and produce a reply.
	virtual request_handler::result
	handle( const ews::request& req, ews::reply& rep ){
		if ( ! boost::starts_with( req.url,"/tmpl/" ) ){
			return cont;
		}

		boost::filesystem::ifstream is( template_path / "test.hdf" );
		char buf[512];
		while (is.read(buf, sizeof(buf)).gcount() > 0){
			rep.content.write( buf, is.gcount() );
		}

		std::string test=req.url;
		boost::erase_first(test, "/tmpl/");
		rep.set_template( test );

		rep.status=ews::reply::ok;
		rep.add_header( "X-HANDLED-BY", "TmplTestHandler" );
		rep.set_basic_headers( "txt" );
		return stop;
	}
	std::string name() const { "CustomHandler"; }


};

static TmplTestHandler tth;
bool is_tmpl( string& filename ) {
        return 
		
		iends_with(filename, ".com");
    }
TEST( Start ){
	DummyApp da;
	da.populate_web();
	template_path=da.template_path;
	EWSTestClient ews;
	EnableLogging el("ewslog");
	boost::filesystem::directory_iterator file( template_path );
	std::string req="/tmpl/";
	filesystem::directory_iterator end_itr;
	for ( file ;file != end_itr; ++file ){
		if ( boost::ends_with( file->leaf(), ".cs") ){
			EWSTestClient::Page p = ews.get( req+file->leaf() );
			cout << p.body << endl;
			break;
		}
	}


}



} // SUITE(EwsTmpl)

int
ews_tmpl( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("EwsTmpl");

}
