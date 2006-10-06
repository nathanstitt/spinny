
#include "testing.hpp"
#include "ews_client.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/detail/predicate.hpp>
#include "ews/server.hpp"

SUITE(EwsConnection) {
	

class CustomHandler
	: public ews::request_handler
{

	/// Handle a request and produce a reply.
	virtual bool handle_request(const ews::request& req, ews::reply& rep, const boost::filesystem::path &doc_root ){
		rep.status=ews::reply::ok;
		rep.content << "Foo";
		rep.set_basic_headers( "txt" );
		rep.add_header( "X-HANDLED-BY", "CustomHandler" );
		return true;
	}

	bool can_handle( const ews::request &req ){
		if ( boost::starts_with( req.url,"/test" ) ){
			return true;
		} else {
			return false;
		}
	}
};



static CustomHandler ca;

// TEST( Status ){
// 	DummyApp da;
// 	EWSTestClient ews;
// 	CHECK_EQUAL( 404, ews.get( "/non/existant/url" ).status );
// 	CHECK_EQUAL( 200, ews.get( "/testurl/" ).status );
// }

// TEST( Body ){
// 	DummyApp da;
// 	EWSTestClient ews;
// 	CHECK_EQUAL( "Foo", ews.get( "/testurl/" ).body );
// }

// TEST( Headers ){
// 	DummyApp da;
// 	EWSTestClient ews;
// 	EWSTestClient::Page p = ews.get( "/testurl/?x=1" );
//   	CHECK_EQUAL( "3", p.headers["Content-Length"] );
//  	CHECK_EQUAL( "text/plain", p.headers["Content-Type"] );
// 	CHECK_EQUAL( "CustomHandler", p.headers["X-HANDLED-BY"] );
// }

TEST( Post ){
 	DummyApp da;
	EnableLogging el;
	boost::logging::manipulate_logs("sql")
		.disable();

 	EWSTestClient ews;
	EWSTestClient::Page p = ews.post( "/testurl/?x=1","Foo=bar&Bar=foo" );
	
	

}

} // SUITE(SqliteTables)

int
ews_connection( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("EwsConnection");

}
