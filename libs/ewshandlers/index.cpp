#include "ews/server.hpp"


namespace {

class Index
	: public ews::request_handler
{
	Index(){
		BOOST_LOGL( ewslog, info ) << "New Index H: " << (int)this;
	}

	/// Handle a request and produce a reply.
	virtual request_handler::result
	handle( const ews::request& req, ews::reply& rep ){
		BOOST_LOGL( ewslog, info ) << "Index H: " << req.url;
		if ( req.url != "/" ){
			return cont;
		}

		rep.set_template( "index.html" );

		rep.add_header( "X-HANDLED-BY", "IndexHandler" );
		rep.set_basic_headers( "html" );

		return stop;
	}
	std::string name() const { return "CustomHandler"; }
	
};

	static Index handler_;
}
