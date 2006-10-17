#include "ews/server.hpp"
#include "ews/handlers/index.hpp"

using namespace ews::handlers;

Index::Index() : ews::request_handler( true ){
	std::cout << "New Index H" << std::endl;
}



ews::request_handler::result
Index::handle( const ews::request& req, ews::reply& rep ){
	BOOST_LOGL( ewslog, info ) << "Index H: " << req.url;
	std::cout << "Index H: " << req.url << std::endl;
	if ( req.url != "/" ){
		return cont;
	}

	rep.set_template( "index.html" );
	
	rep.add_header( "X-HANDLED-BY", "IndexHandler" );
	rep.set_basic_headers( "html" );
	
	return stop;
}
std::string
Index::name() const { return "CustomHandler"; }


