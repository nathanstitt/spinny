
#include "handlers/tree.hpp"
#include "spinny/music_dir.hpp"
#include "spinny/artist.hpp"
#include "spinny/album.hpp"
#include "boost/algorithm/string/split.hpp"
#include <algorithm>
#include "sqlite/comma.hpp"

using namespace handlers;

ews::request_handler::result
Tree::handle( const ews::request& req, ews::reply& rep ){
	BOOST_LOGL( ewslog, info ) << "Index T: " << req.url;

	if ( ! boost::starts_with( req.url,"/tree/" ) ){
		return cont;
	}

	vector< string > elements;
	boost::split( elements, req.url, boost::is_any_of("/"), boost::token_compress_on );

	if ( elements.size() < 3 || elements[3].empty() ){
		return ews::request_handler::error;
	}

	rep.content << "[";

	if( elements[2] == "dirs" ){

		MusicDir::result_set rs = MusicDir::children_of( 
			boost::lexical_cast<sqlite::id_t>( elements[3] ) );
		sqlite::commas comma;
		for ( MusicDir::result_set::iterator md = rs.begin(); rs.end() != md; ++md ){
			rep.content << comma << "\n    " << "{"
				    << "\"id\":" << md->db_id()
				    << ",\"label\":\"" << md->path().leaf() << "\""
				    << ",\"ch\":" << md->num_children()
				    << ",\"type\":\"dirs\" }";
		}

	} else if ( elements[2] == "artists" ){
		Artist::result_set rs = Artist::name_starts_with( elements[3] );
		for ( Artist::result_set::iterator artist = rs.begin(); rs.end() != artist; ++artist ){
			rep.content << "    {"
				    << "\"id\":" << artist->db_id()
				    << ",\"label\":\"" << artist->name() << "\""
				    << ",\"type\":\"artists\" }";
		}
	} else if ( elements[2] == "albums" ){
		Album::result_set rs = Album::name_starts_with( elements[3] );
		for ( Album::result_set::iterator album = rs.begin(); rs.end() != album; ++album ){
			rep.content << "    {"
				    << "\"id\":" << album->db_id()
				    << ",\"label\":\"" << album->name() << "\""
				    << ",\"type\":\"albums\" }";
		}
	} else {
		return ews::request_handler::error;
	}
	rep.content << "\n]";
	rep.add_header( "X-HANDLED-BY", "TreeHandler" );
	rep.set_basic_headers( "json" );

	return stop;
}
std::string
Tree::name() const { return "Tree"; }


