

#include "handlers/login.hpp"
#include "spinny/user.hpp"


using namespace handlers;

Login::Login() : ews::request_handler( "Login", Beginning ) {}

ews::request_handler::RequestStatus
Login::handle( const ews::request& req, ews::reply& rep ) const {






	return Stop;
}
