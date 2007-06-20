#include "handlers/xspf.hpp"
#include "spinny/user.hpp"
#include "spinny/song.hpp"
#include "spinny/playlist.hpp"

#include "spinny/settings.hpp"
#include <json/json.h>
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace handlers;

Xspf::Xspf() : ews::request_handler( "Xspf", Beginning ) {}

ews::request_handler::RequestStatus
Xspf::handle( const ews::request& req, ews::reply& rep ) const {


	if ( req.u1 != "xspf" ){
		return Continue;
	}


	rep.set_basic_headers( "txt" );

	Spinny::PlayList::ptr pl = Spinny::PlayList::load( boost::lexical_cast<sqlite::id_t>( req.u2 ) );


	rep.set_template( "xspf.html" );

	rep.set_hdf_value( "name", pl->name() );
	rep.set_hdf_value( "description", pl->description() );

	rep.content << "Songs {\n";
	Spinny::Song::result_set songs = pl->songs();
	unsigned int num=0;
	for(Spinny::Song::result_set::iterator song = songs.begin(); songs.end() != song; ++song ){
		
		rep.content << "  " << ++num
			    << " {"
			    << "\n     url=" << "/song/dl/" << req.user->ticket << "/" << song->db_id()
			    << "\n     album=" << boost::replace_all_copy( song->album_name(), "\"","\\\"" )
			    << "\n     artist=" << boost::replace_all_copy( song->artist_name(), "\"","\\\"" )
			    << "\n     title=" << boost::replace_all_copy( song->title(), "\"","\\\"" )
			    << "\n     length=" << song->length() * 1000
			    << "\n     image=" << "/song/img/" << req.user->ticket << "/" << song->db_id()
			    << "\n    }\n";

	}
	rep.content << "}\n";

	return Stop;

}
