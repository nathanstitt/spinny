
#include "handlers/users.hpp"
#include "spinny/user.hpp"
#include "spinny/settings.hpp"
#include <json/json.h>
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace handlers;

Users::Users() : ews::request_handler( "Users", Beginning ) {}

ews::request_handler::RequestStatus
Users::handle( const ews::request& req, ews::reply& rep ) const {


	if ( req.u1 != "users" ){
		return Continue;
	}
	requireAuth( req.user, Spinny::User::AdminRole );

	rep.set_basic_headers( "json" );

	if ( req.u2 == "list" ){
		boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
		unsigned int page = req.single_value<unsigned int>("page")-1;
		unsigned int num  = req.single_value<unsigned int>("pageSize");
		std::string sort = "login";
		if ( req.svalue("sortColumn") == "3" ){
			sort = "last_visit";
		}
		bool descending = ( req.svalue("sortDir") == "DESC" );
		Spinny::User::result_set users = Spinny::User::list( page*num, num,sort, descending );

		sqlite::commas comma;
		rep.content << "{ Users:  [ \n";
		for ( Spinny::User::result_set::iterator user = users.begin(); user != users.end(); ++user ){
			boost::posix_time::time_duration last = now - user->last_visit;
			rep.content << comma << "{ 'id':" << user->db_id() << ",'login':'"
				    << boost::replace_all_copy( user->login, "\"", "\\\"")
				    << "','password':'********','last_visit':'"
				    << boost::posix_time::to_simple_string(last) << "','role':\'"
				    << user->role() << "\'}\n";
		}
		rep.content << "]}\n";
	} else if ( req.u2 == "del" ){
		Spinny::User::ptr user = Spinny::User::load( req.single_value<sqlite::id_t>( "uid" ) );
		user->destroy();
	} else if ( req.u2 == "save" ){
		Json::Reader reader;
		Json::Value json;
		std::string data = req.svalue("data");
		if ( data.empty() || ! reader.parse( data, json ) || json.type() != Json::arrayValue ){
			BOOST_LOGL( www, err ) << "Failed to parse initial json: " << data;
			throw ews::error("Malformed JSON data received" );
		} else {
			int size = json.size();
			for ( int index = 0; index < size; ++index ) {
				Json::Value json_user = json[ index ];
				sqlite::id_t uid = json_user["id"].asUInt();
				BOOST_LOGL(www,info) << "Saving User id: " << uid 
 						     << "\nLogin: " << json_user["login"].asString()
 						     << "\n Pass: " << (json_user["password"].asString() == "********"
 								       ? "(Unchanged)" : json_user["password"].asString() )
 						     << "\n Role: " << json_user["role"].asString();
				Spinny::User::ptr user;
				if ( uid ){
					user = Spinny::User::load( uid );
					user->login = json_user["login"].asString();
					if (  json_user["password"].asString() != "********" ){
						user->set_password( json_user["password"].asString() );
					}
					user->set_role(
						static_cast<Spinny::User::Role>(
							boost::lexical_cast<int>( json_user["role"].asString() ) ) );
					user->save();

				} else {
					BOOST_LOGL(www,debug) << "About to create user";

					Spinny::User::ptr user = Spinny::User::create( json_user["login"].asString(),
										       json_user["password"].asString() );
					user->set_role(
						static_cast<Spinny::User::Role>(
							boost::lexical_cast<int>( json_user["role"].asString() ) ) );
					user->save();
				} 
			}
		}
	}

	return Stop;
}
