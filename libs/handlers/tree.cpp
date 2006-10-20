
#include "handlers/tree.hpp"
#include "spinny/music_dir.hpp"
#include "spinny/artist.hpp"
#include "spinny/album.hpp"
#include "boost/algorithm/string.hpp"
//#include "boost/algorithm/string/replace.hpp"
#include <algorithm>
#include "sqlite/comma.hpp"

using namespace handlers;

static
void
insert( ews::reply &rep,
	sqlite::commas &comma,
	sqlite::id_t id,
	const std::string &label, 
	sqlite::id_t num_children,
	const std::string &type )
{
	rep.content << comma << "\n    " << "{"
		    << "\"id\":" << id
		    << ",\"label\":\"" << boost::replace_all_copy( label, "\"", "\\\"") << "\""
		    << ",\"ch\":" << num_children
		    << ",\"type\":\"" << type << "\" }";
}

static
void
insert( ews::reply &rep,
	sqlite::commas &comma,
	char id,
	char label, 
	sqlite::id_t num_children,
	const std::string &type )
{
	rep.content << comma << "\n    " << "{"
		    << "\"id\":\"" << id << "\""
		    << ",\"label\":\"" << label << "\""
		    << ",\"ch\":" << num_children
		    << ",\"type\":\"" << type << "\" }";
}


static void
h_dirs( ews::reply &rep, const std::string &ch ){
	MusicDir::result_set rs = MusicDir::children_of( 
		boost::lexical_cast<sqlite::id_t>( ch ) );
	sqlite::commas comma;
	for ( MusicDir::result_set::iterator md = rs.begin(); rs.end() != md; ++md ){
		insert( rep, comma, md->db_id(), md->path().leaf(), md->num_children(), "dirs" );
	}
}

typedef std::list<std::pair<char, unsigned int> > char_cont_t;
static void
h_first_char( ews::reply &rep, const std::string &type ){
	char_cont_t chars;
	if ( type == "artists" ){
		chars = Artist::starting_chars();
	} else if ( type == "albums" ){
		chars = Album::starting_chars();
	}
	sqlite::commas comma;
	for ( char_cont_t::iterator c = chars.begin(); chars.end() != c; ++c ){
		insert( rep, comma, c->first, c->first, c->second, type );
	}
}

static void
h_all( ews::reply &rep, const std::string &type ){
	sqlite::commas comma;
	if ( type == "artists" ){
		Artist::result_set rs = Artist::all();
		for ( Artist::result_set::iterator artist = rs.begin(); rs.end() != artist; ++artist ){
			insert( rep, comma, artist->db_id(),artist->name(),0,type);
		}
	} else if ( type == "albums" ){
		Album::result_set rs = Album::all();
		for ( Album::result_set::iterator album = rs.begin(); rs.end() != album; ++album ){
			insert( rep, comma, album->db_id(),album->name(),0,type);
		}
	}
}


static void
h_start_with( ews::reply &rep, const std::string &ch, const std::string &type ){
	sqlite::commas comma;
	if ( type == "artists" ){
		Artist::result_set rs = Artist::name_starts_with( ch );
		for ( Artist::result_set::iterator artist = rs.begin(); rs.end() != artist; ++artist ){
			insert( rep,comma,artist->db_id(), artist->name(), 0, type );
		}
	} else if ( type == "albums" ){
		Album::result_set rs = Album::name_starts_with( ch );
		for ( Album::result_set::iterator album = rs.begin(); rs.end() != album; ++album ){
			insert( rep,comma,album->db_id(), album->name(), 0, type );
		}
	}
}



ews::request_handler::result
Tree::handle( const ews::request& req, ews::reply& rep ) const { 
	BOOST_LOGL( ewslog, info ) << "Index T: " << req.url;

	if ( req.u1 != "tree" ) {
		return cont;
	} else if ( req.u3.empty() ){
		return ews::request_handler::error;
	}

	rep.content << "[";

	if( req.u2 == "dirs" ){
		h_dirs( rep, req.u3 );
	} else if ( req.u2 == "artists" || req.u2 == "albums" ){
		if ( req.u3 == "0" ){
			h_first_char( rep, req.u2 );
		} else if ( req.u3 == "ALL" ) {
			h_all( rep, req.u2 );
		} else {
			h_start_with(rep, req.u3, req.u2 );
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


