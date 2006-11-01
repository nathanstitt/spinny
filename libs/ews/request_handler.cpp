#include "request_handler.hpp"
#include "files_request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "ews/mime_types.hpp"
#include "ews/reply.hpp"
#include "ews/request.hpp"
#include "ews/server.hpp"
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


	bool
	request_handler::handle_request( const request& req, reply &rep ) {

		handlers_t *handlers = register_new_handler( NULL );

		result res=file;
 		for ( handlers_t::iterator h=handlers->begin(); h!=handlers->end(); ++h ){
 			try {
 				res = (*h)->handle( req, rep );
 			}
 			catch ( const std::exception &e ){
 				BOOST_LOGL( www, err ) << "Handler: " << (*h)->name()
 							  << " raised: " << e.what();
 				return false;
 			}

 			if ( stop == res || file == res ){
				break;
			} else if ( error == res ){
 				BOOST_LOGL( www, err ) << "Handler: " << (*h)->name()
 							  << " returned error.";
 				return false;
			}
 		}
 		if ( stop != res ) {
 			res=files_handler_.handle( req, rep );
 		}
		return ( res==stop );
	}


	request_handler::~request_handler(){}


} // namespace ews
