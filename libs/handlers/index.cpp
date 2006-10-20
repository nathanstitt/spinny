#include "ews/server.hpp"
#include "handlers/index.hpp"
#include "spinny/music_dir.hpp"
#include "spinny/album.hpp"
#include "spinny/artist.hpp"
#include "boost/lambda/lambda.hpp"
#include "boost/lambda/bind.hpp"
#include <algorithm>


using namespace handlers;


ews::request_handler::result
Index::handle( const ews::request& req, ews::reply& rep ) const {
	BOOST_LOGL( ewslog, info ) << "Index H: " << req.url;

	if ( req.u1 != "" ){
		return cont;
	}

	rep.set_template( "index.html" );

	MusicDir::result_set rs = MusicDir::roots();

	rep.set_hdf_value( "NumArtists", Artist::count() );
	rep.set_hdf_value( "NumAlbums", Album::count() );
	rep.set_hdf_value( "NumSongs", Song::count() );

	rep.add_header( "X-HANDLED-BY", "IndexHandler" );
	rep.set_basic_headers( "html" );
	
	return stop;
}
std::string
Index::name() const { return "Index"; }


