#include "handlers/shared.hpp"
#include "handlers/info.hpp"
#include "spinny/song.hpp"
#include "spinny/artist.hpp"
#include "sqlite/comma.hpp"

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

	} else if ( req.u2 == "find" ){
		sqlite::commas comma;
		unsigned int page = req.single_value<unsigned int>("page")-1;
		unsigned int num  = req.single_value<unsigned int>("pageSize");
		std::string query = req.svalue("q");
		Spinny::Song::result_set songs = Spinny::Song::find( query, page*num, num );
		rep.content << "{ TotalCount:" << Spinny::Song::num_matching( query ) << ",\nSongs: [ \n";
		for ( Spinny::Song::result_set::iterator song = songs.begin(); song != songs.end(); ++song ){
			rep.content << comma << "{ 'id':" << song->db_id()
				    << ",'title':'" << json_q( song->title() ) << "'}\n";
		}

		rep.content  << "]}\n";

	}

	rep.set_basic_headers( "json" );

	return Stop;

}
