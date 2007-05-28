
#include "handlers/tree.hpp"
#include "spinny/music_dir.hpp"
#include "spinny/artist.hpp"
#include "spinny/album.hpp"
#include "boost/algorithm/string.hpp"
//#include "boost/algorithm/string/replace.hpp"
#include <algorithm>
#include "sqlite/comma.hpp"
#include "handlers/shared.hpp"

using namespace handlers;

static
void
insert( ews::reply &rep,
	sqlite::commas &comma,
	sqlite::id_t id,
	const std::string &label, 
	sqlite::id_t num_children,
	char *type )
{
	rep.content << comma << "\n    " << "{"
		    << "\"id\":" << id
		    << ",\"text\":\"" << boost::replace_all_copy( label, "\"", "\\\"") << "\"" << ",\"leaf\":";
	if ( num_children ){
		rep.content << "false";
	} else {
		rep.content << "true";
	}
	rep.content << ",\"cls\":\"" << type << "\" }";
}


static void
insert_songs( ews::reply &rep, Spinny::Song::result_set rs,sqlite::commas &comma ){
	for ( Spinny::Song::result_set::iterator song = rs.begin(); rs.end() != song; ++song ){
		if ( song->track() ){
			std::string name = boost::lexical_cast<std::string>( song->track() );
			name += "-" + song->name();
			insert( rep, comma, song->db_id(), name, 0, "song" );
		} else {
			insert( rep, comma, song->db_id(), song->name(), 0, "song" );
		}
	}
}



static void
insert_starting_artists( ews::reply &rep, sqlite::commas &comma ){
	Spinny::Artist::starting_char_t chars = Spinny::Artist::starting_chars();
	for ( Spinny::Artist::starting_char_t::const_iterator c = chars.begin();
	      chars.end() != c;
	      ++c )

	{
		rep.content << comma << "\n    " << "{"
			    << "\"id\":\"a" << (char)c->first
			    << "\",\"text\":\"" << c->first << "\""
			    << ",\"leaf\":false,\"cls\":\"artist\" }";
	}
}


static void
insert_artists( ews::reply &rep, sqlite::commas &comma, Spinny::Artist::result_set rs ){
	for ( Spinny::Artist::result_set::iterator artist = rs.begin(); rs.end() != artist; ++artist ){
		rep.content << comma << "\n    " << "{"
			    << "\"id\":\"b" << artist->db_id()
			    << "\",\"text\":\"" << boost::replace_all_copy( artist->name(), "\"", "\\\"") << "\""
			    << ",\"leaf\":false,\"cls\":\"artist\" }";
	}
}

static void
insert_albums( ews::reply &rep, sqlite::id_t db_id, sqlite::commas &comma ){
	Spinny::Artist::ptr artist = Spinny::Artist::load( db_id );
	Spinny::Album::result_set rs = artist->albums();
	for ( Spinny::Album::result_set::iterator album = rs.begin(); rs.end() != album; ++album ){
		rep.content << comma << "\n    " << "{"
			    << "\"id\":\"c" << album->db_id()
			    << "\",\"text\":\"" << boost::replace_all_copy( album->name(), "\"", "\\\"") << "\""
			    << ",\"leaf\":false,\"cls\":\"album\" }";
	}
}
Tree::Tree() : ews::request_handler( "Tree", Middle ) {}

ews::request_handler::RequestStatus
Tree::handle( const ews::request& req, ews::reply& rep ) const { 


 	if ( req.u1 != "tree" ) {
 		return Continue;
 	}

 	rep.content << "[";
	

	
 	sqlite::commas comma;

	if ( req.u2 == "dir" ){
		BOOST_LOGL( www, info ) << "Tree loading children of Dir id: " << req.svalue("node");

		sqlite::id_t dir_id = req.single_value<sqlite::id_t>("node");

		Spinny::MusicDir::result_set rs = Spinny::MusicDir::children_of( dir_id );

		if ( rs.size() == 1 && ! dir_id ){
			rs = rs.begin()->children();
		}
		for ( Spinny::MusicDir::result_set::iterator md = rs.begin(); rs.end() != md; ++md ){
			insert( rep, comma, md->db_id(), md->filesystem_name(), md->num_children()+md->num_songs(), "dir" );
		}
		if ( dir_id ){
			Spinny::MusicDir::ptr md = Spinny::MusicDir::load( dir_id );
			if ( ! md ){
				throw ews::error("Unable to load music dir");
			}
			insert_songs( rep, md->songs(),comma );
		}
	} else if ( req.u2 == "tag" ) {
		std::string node = req.svalue("node");
		if (  node == "0" ) {
			if ( Spinny::Artist::count() > 100 ){
				insert_starting_artists( rep, comma );
			} else {
				insert_artists( rep, comma, Spinny::Artist::all() );
			}
		} else if ( node.size() < 2 ){
			throw ews::error("Unable to load data");
		} else {
			char t = node[0];
			node.erase( 0, 1 );
			switch ( t ){
			case 'a':
				BOOST_LOGL(www,info) << "Load Artists starting with " << node;
				insert_artists( rep, comma, Spinny::Artist::name_starts_with( node ) );
				break;
			case 'b':
				BOOST_LOGL(www,info) << "Load album id " << node;

				insert_albums( rep, boost::lexical_cast<sqlite::id_t>( node ), comma );
				break;
			case 'c':
				insert_songs( rep, Spinny::Album::load( boost::lexical_cast<sqlite::id_t>( node ) )->songs(), comma );
			}
			
		}
	}

 	rep.content << "\n]";

 	rep.set_basic_headers( "json" );

 	return Stop;


}
