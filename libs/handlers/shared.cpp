

/* @(#)shared.cpp
 */


#include "handlers/shared.hpp"
#include "handlers/index.hpp"
#include "handlers/tree.hpp"
#include "handlers/pl.hpp"
#include "handlers/file_system.hpp"
#include "handlers/login.hpp"
#include "handlers/users.hpp"
#include "handlers/info.hpp"


void
handlers::link_up(){

	static handlers::PL pl_;
	static handlers::Index index_;
	static handlers::Tree tree_;
	static handlers::Files files_;
	static handlers::Login login_;
	static handlers::Users users_;
	static handlers::Info info_;

	BOOST_LOGL(www,info) << "LINKING UP HANDLERS";
};


void
handlers::requireAuth( Spinny::User::ptr user, Spinny::User::Role role ){
	if ( ! user->has_at_least( role ) ){
		throw( ews::error("User Account lacks proper permission") );
	}
}


std::string
handlers::json_q( const std::string& str ){
	return boost::replace_all_copy( str, "\'", "\\\'");
}
