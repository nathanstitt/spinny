#include "handlers/shared.hpp"
#include "handlers/info.hpp"
#include "spinny/song.hpp"
#include "spinny/artist.hpp"

using namespace handlers;

Info::Info() : ews::request_handler( "Info", Middle ) {}

ews::request_handler::RequestStatus
Info::handle( const ews::request& req, ews::reply& rep ) const { 

	
	if ( req.u1 != "info" ) {
		return Continue;
	}


	if ( req.u2 == "song" ){
		Spinny::Song::ptr song = Spinny::Song::load( boost::lexical_cast<sqlite::id_t>( req.u3 ) );


		if ( req.u4 == "artist" ) {
			rep.content << song->artist()->name();
		}

	}

	rep.set_basic_headers( "txt" );

	return Stop;

}
