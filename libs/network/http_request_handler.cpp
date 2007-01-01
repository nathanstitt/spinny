#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "network/mime_types.hpp"
#include "network/reply.hpp"
#include "network/request.hpp"
#include "network/server.hpp"
#include <list>
#include "boost/date_time/posix_time/posix_time.hpp"

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
					if ( Stop == status ){
						rep.set_header( "X-HANDLED-BY", (*h)->name() );
					}

				}
				catch ( const ews::not_found_error &e ){
					BOOST_LOGL( www,err ) << (*h)->name() << " raised ews::not_found_error - " << e.what() 
							      << " : referer: " << req.get_header<std::string>("HTTP_REFERER");
					rep.set_to( reply::not_found );
					rep.clear_contents();
					rep.content << "<html>\n<head><title>Not Found</title></head>"
						    << "<body><h1>404 Not Found error retrieving " << e.what() << "</h1></body>"
						    << "</html>";
					status = Stop;
					break;
				}
				catch ( const ews::forbidden_error &e ){
					BOOST_LOGL( www,err ) << (*h)->name() << " raised ews::error - " << e.what();
					rep.set_to( reply::forbidden );
					rep.clear_contents();
					rep.content << "<html>\n<head><title>Forbidden</title></head>"
						    << "<body><h1>403 Forbidden " << e.what() << "</h1></body>"
						    << "</html>";
					status = Stop;
					break;
				}
				catch ( const ews::error &e ){
					BOOST_LOGL( www,err ) << (*h)->name() << " raised ews::error - " << e.what();
					rep.set_to( reply::internal_server_error );
					rep.clear_contents();
					rep.content << "<html>\n<head><title>Internal Server Error</title></head>"
						    << "<body><h1>500 " << e.what() << "</h1></body>"
						    << "</html>";
					status = Stop;
					break;
				}
				catch ( const std::exception &e ){
					BOOST_LOGL( www,err ) << (*h)->name() << " raised std::exception - " << e.what();
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
		if ( rep.user ){
 			std::string ticket( "Ticket=" );
			ticket += rep.user->ticket;
			ticket += "; path=/;";
			rep.set_header( "Set-Cookie", ticket );
			rep.user->last_visit = boost::posix_time::second_clock::local_time();
			rep.user->save();
		}
		if ( Stop != status ){
			rep.set_to( reply::not_found );
		}

		return true;
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
