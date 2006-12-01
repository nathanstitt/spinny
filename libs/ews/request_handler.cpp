#include "request_handler.hpp"
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

	handlers_t**
	register_new_handler( request_handler *handler, request_handler::RequestPhase phase ){
		static handlers_t *begining = new handlers_t;
		static handlers_t *middle = new handlers_t;
		static handlers_t *end = new handlers_t;
		static handlers_t *last = new handlers_t;

		static handlers_t* handlers[] = { begining, middle, end, last };

		if ( handler ){
			handlers[ phase ]->push_back( handler );
		}
		return handlers;
	}

	request_handler::request_handler( const char *name, RequestPhase phase ) : name_( name ), phase_(phase) {
//		std::cout << "Registering Handler" << name_ << " at phase: " << phase_ << std::endl;
		register_new_handler( this, phase );
	}

	bool
	request_handler::handle_request( const request& req, reply &rep ) {

		handlers_t **handlers = register_new_handler( NULL, End );

		RequestStatus status=Continue;

 		for ( int phase = Beginning; Stop != status && phase <= Last; ++phase ){
			for ( handlers_t::iterator h=handlers[ phase ]->begin(); h!=handlers[phase]->end(); ++h ){
				BOOST_LOGL( www, debug ) << "Passing Req to Handler " << (*h)->name();
				try {
					status = (*h)->handle( req, rep );
				}
				catch ( const ews::not_found_error &e ){
					BOOST_LOGL( www,err ) << "Caught Not Found err";
					rep.set_to( reply::not_found );
					rep.clear_contents();
					rep.content << "<html>\n<head><title>Not Found</title></head>"
						    << "<body><h1>404 Not Found error retrieving " << e.what() << "</h1></body>"
						    << "</html>";
					status = Stop;
					break;
				}
				catch ( const ews::error &e ){
					rep.set_to( reply::internal_server_error );
					rep.clear_contents();
					rep.content << "<html>\n<head><title>Internal Server Error</title></head>"
						    << "<body><h1>" << e.what() << "</h1></body>"
						    << "</html>";
					status = Stop;
					break;
				}
				catch ( const std::exception &e ){
					BOOST_LOGL( www, err ) << "Handler: " << (*h)->name()
							       << " raised std::exception - " << e.what();
					rep.set_to( reply::internal_server_error );
					status = Stop;
					break;
				}
				if ( Stop == status ){
					break;
				}
			}
		}

		if ( ! rep.user && req.user ){
			rep.user = req.user;
		}
		if ( req.user ){
			std::string ticket( "Ticket=" );
			ticket += req.user->ticket;
			ticket += ";";
			rep.set_header( "Set-Cookie", ticket );
		}
		return ( Stop == status );
	}
	request_handler::~request_handler(){}

	const char *
	request_handler::name() const {
		return name_;
	}

	request_handler::RequestPhase
	request_handler::phase() const {
		return phase_;
	}

} // namespace ews
