#include "request_handler.hpp"
#include "files_request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include <list>


namespace ews {

typedef std::list<request_handler*> handlers_t;

static files_request_handler files_handler_;

handlers_t*
register_new_handler( request_handler *handler ){
	static handlers_t *handlers = new handlers_t;
	if ( handler ){
		handlers->push_back( handler );
	}
	return handlers;
}

request_handler::request_handler( bool should_register ) {
	if ( should_register ){
		register_new_handler( this );
	}
}

request_handler*
request_handler::find_handler( const request& req ){
	handlers_t *handlers = register_new_handler( NULL );
	for ( handlers_t::iterator h=handlers->begin(); h!=handlers->end(); ++h ){
		if ( (*h)->can_handle( req ) ){
			return *h;
		}
	}
	return &files_handler_;
}

	request_handler::~request_handler(){}

} // namespace ews
