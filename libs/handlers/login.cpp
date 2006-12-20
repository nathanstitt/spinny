

#include "handlers/login.hpp"
#include "spinny/user.hpp"
#include "spinny/settings.hpp"

using namespace handlers;

Login::Login() : ews::request_handler( "Login", Beginning ) {}

ews::request_handler::RequestStatus
Login::handle( const ews::request& req, ews::reply& rep ) const {


	if ( ! req.user && ! boost::ends_with( req.url, ".gif" ) && req.u2.empty() ){
		rep.set_template( "login.html" );

		rep.set_hdf_value( "AllowGuests",
				   Spinny::Settings::get<string>("AllowGuests") );

		rep.set_basic_headers( "html" );
		return Stop;
	} else if ( req.u2 == "validate" ) {
		BOOST_LOGL( www, debug ) << "Authenicating " << req.svalue( "login" );

		std::string mode =  Spinny::Settings::get<std::string>("AllowGuests");
		if ( mode == "WithPassword" ) {
			

		} else if ( mode == "WithName") {


		} else {
			Spinny::User::ptr user = Spinny::User::with_login( req.svalue( "login" ) );
			if ( user && user->authen( req.svalue("pass") ) ){
				BOOST_LOGL( www, info ) << req.svalue( "login" ) << " authenticated successfully";
				rep.user = user;
 				rep.set_to( ews::reply::moved_temporarily );
 				rep.set_header( "Location", "/" );
			} else {
				BOOST_LOGL( www, info ) << req.svalue( "login" ) << " failed login ( "
							<< ( user ? "not found" : "password mismatch" ) << " )";

				rep.set_hdf_value("FailedAuth",1);
			}

			return Stop;

		};

	}

	return Continue;
}
