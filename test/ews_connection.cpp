
#include "testing.hpp"
#include "ews_client.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/detail/predicate.hpp>
#include "ews/server.hpp"
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <algorithm>

SUITE(EwsConnection) {
	

class CustomHandler
	: public ews::request_handler
{
	/// Handle a request and produce a reply.
	virtual request_handler::result
	handle( const ews::request& req, ews::reply& rep ){
		if ( ! boost::starts_with( req.url,"/test" ) ){
			return cont;
		}
		rep.status=ews::reply::ok;
		for ( ews::request::varibles_t::const_iterator vars = req.varibles.begin(); req.varibles.end() != vars; ++vars ){
			for ( ews::request::varible_t::const_iterator val = vars->second.begin(); vars->second.end() != val; ++ val ){
				if ( ! rep.content.str().empty() ){
					rep.content << "&";
				}
				rep.content << vars->first << "=" << *val;
			}
		}

		rep.add_header( "X-HANDLED-BY", "CustomHandler" );
		rep.set_basic_headers( "txt" );

		return stop;
	}
	std::string name() const { "CustomHandler"; }


};


static CustomHandler ca;

TEST( Status ){
	DummyApp da;
	EWSTestClient ews;
	CHECK_EQUAL( 404, ews.get( "/non/existant/url" ).status );
	CHECK_EQUAL( 200, ews.get( "/testurl/" ).status );
}

TEST( Body ){
 	DummyApp da;
 	EWSTestClient ews;
 	CHECK_EQUAL( "gadzoo=baa", ews.get( "/testurl/?gadzoo=baa" ).body );
}

TEST( Headers ){
 	DummyApp da;
 	EWSTestClient ews;
 	EWSTestClient::Page p = ews.get( "/testurl/?x=1" );
    	CHECK_EQUAL( "3", p.headers["Content-Length"] );
   	CHECK_EQUAL( "text/plain", p.headers["Content-Type"] );
  	CHECK_EQUAL( "CustomHandler", p.headers["X-HANDLED-BY"] );
}

TEST( Get ){
  	DummyApp da;
 	EWSTestClient ews;
	EWSTestClient::Page p = ews.get( "/testurl/?x=1" );
	CHECK_EQUAL( "CustomHandler", p.headers["X-HANDLED-BY"] );
	CHECK_EQUAL( 200, p.status );
	CHECK_EQUAL( "x=1" ,p.body );
}

TEST( Post ){
  	DummyApp da;
 	EWSTestClient ews;
	EWSTestClient::Page p = ews.post( "/testurl/?x=1","Foo=bar&Bar=foo" );
	CHECK_EQUAL( "CustomHandler", p.headers["X-HANDLED-BY"] );
	CHECK_EQUAL( 200, p.status );
	CHECK_EQUAL( "Bar=foo&Foo=bar&x=1" ,p.body );

//      using namespace boost::lambda;
// 	std::for_each( p.headers.begin(), p.headers.end(),
// 		       cout << "RecHeader: " <<
// 		       bind(&std::map<std::string,std::string>::value_type::first, _1) << " => " <<
// 		       bind(&std::map<std::string,std::string>::value_type::second, _1) << "\n"
// 		);
}

} // SUITE(SqliteTables)

int
ews_connection( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("EwsConnection");
}
