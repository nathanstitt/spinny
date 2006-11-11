
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
	JsonReqType type )
{
	rep.content << comma << "\n    " << "{"
		    << "\"id\":" << id
		    << ",\"label\":\"" << boost::replace_all_copy( label, "\"", "\\\"") << "\""
		    << ",\"ch\":" << num_children
		    << ",\"type\":\"" << (char)type << "\" }";
}

// static
// void
// insert( ews::reply &rep,
// 	sqlite::commas &comma,
// 	sqlite::id_t id,
// 	char label,
// 	sqlite::id_t num_children,
// 	JsonReqType type )
// {
// 	rep.content << comma << "\n    " << "{"
// 		    << "\"id\":" << id
// 		    << ",\"label\":\"" << label
// 		    << ",\"ch\":" << num_children
// 		    << ",\"type\":\"" << (char)type << "\" }";
// }

static void
insert_songs( ews::reply &rep, Song::result_set rs,sqlite::commas &comma ){
	for ( Song::result_set::iterator song = rs.begin(); rs.end() != song; ++song ){
		std::string name = boost::lexical_cast<std::string>( song->track() );
		name += "-" + song->name();
		insert( rep, comma, song->db_id(), name, 0, JsonSong );
	}
}




// static void
// h_all( ews::reply &rep,  ){
// 	sqlite::commas comma;
// 	if ( type == JsonArtist ){
// 		Artist::result_set rs = Artist::all();
// 		for ( Artist::result_set::iterator artist = rs.begin(); rs.end() != artist; ++artist ){
// 			insert( rep, comma, artist->db_id(),artist->name(),0, type );
// 		}
// 	} else if ( type == JsonAlbumsFirstChar ){
// 		Album::result_set rs = Album::all();
// 		for ( Album::result_set::iterator album = rs.begin(); rs.end() != album; ++album ){
// 			insert( rep, comma, album->db_id(),album->name(),0,type);
// 		}
// 	}
// }


// static void
// h_start_with( ews::reply &rep, const std::string &ch, const std::string &type ){
// 	sqlite::commas comma;
// 	if ( type == "artists" ){
// 		Artist::result_set rs = Artist::name_starts_with( ch );
// 		for ( Artist::result_set::iterator artist = rs.begin(); rs.end() != artist; ++artist ){
// 			insert( rep,comma,artist->db_id(), artist->name(), 0, type );
// 		}
// 	} else if ( type == "albums" ){
// 		Album::result_set rs = Album::name_starts_with( ch );
// 		for ( Album::result_set::iterator album = rs.begin(); rs.end() != album; ++album ){
// 			insert( rep,comma,album->db_id(), album->name(), 0, type );
// 		}
// 	}}




ews::request_handler::result
Tree::handle( const ews::request& req, ews::reply& rep ) const { 
//	BOOST_LOGL( www, info ) << "Index T: " << req.url;

	if ( req.u1 != "tree" ) {
		return cont;
	} else if ( req.u2.empty() || req.u3.empty() ){
		return ews::request_handler::error;
	}

	rep.content << "[";
	
	JsonReqType type = static_cast<JsonReqType>( req.u2[0] );
	
	sqlite::commas comma;

	switch ( type ){

	case JsonDir: {
		MusicDir::ptr md = MusicDir::load(  boost::lexical_cast<sqlite::id_t>( req.u3 ) );
		BOOST_LOGL( www, info ) << "Tree loading children of Dir " << md->path().string() << " id: " << md->db_id();
		MusicDir::result_set rs = md->children();
		for ( MusicDir::result_set::iterator md = rs.begin(); rs.end() != md; ++md ){
			insert( rep, comma, md->db_id(), md->path().leaf(), md->num_children()+md->num_songs(), JsonDir );
		}
		insert_songs( rep, md->songs(),comma );
		break;
	}

	case JsonArtistsFirstChar: {
		BOOST_LOGL( www, info ) << "Tree loading artists starting with " << req.u3;
		Artist::result_set rs = Artist::name_starts_with( req.u3 );
		for ( Artist::result_set::iterator artist = rs.begin(); rs.end() != artist; ++artist ){
 			insert( rep,comma, artist->db_id(), artist->name(), artist->num_albums()+artist->num_songs(), JsonArtistsAlbum );
 		}
		break;
	}

	case JsonAlbumsFirstChar: {
		BOOST_LOGL( www, info ) << "Tree loading albums starting with " << req.u3;
		Album::result_set rs = Album::name_starts_with( req.u3 );
		for ( Album::result_set::iterator album = rs.begin(); rs.end() != album; ++album ){
 			insert( rep,comma, album->db_id(), album->name(), album->num_songs(), JsonAlbum );
 		}
		break;
	}

	case JsonAlbum: {
		Album::ptr album = Album::load( boost::lexical_cast<sqlite::id_t>( req.u3 ) );
		BOOST_LOGL( www, info ) << "Tree loading songs for album " << album->name() << " id: " << album->db_id();

		insert_songs( rep, album->songs(),comma );
		break;
	}


	case JsonArtistsAlbum: {
		Artist::ptr artist = Artist::load( boost::lexical_cast<sqlite::id_t>( req.u3 ) );
		BOOST_LOGL( www, info ) << "Tree loading albums and songs for artist " << artist->name() << " id: " << artist->db_id();
		Album::result_set ars = artist->albums();
		for ( Album::result_set::iterator album = ars.begin(); ars.end() != album; ++album ){
			insert( rep, comma, album->db_id(),album->name(), album->num_songs(), JsonAlbum );
		}
		insert_songs( rep, artist->songs(),comma );
		break;
	}

	default:
		BOOST_LOGL( www, err ) << "Tree Ajax handler recieved unknown request code: " << req.u2;
		return ews::request_handler::error;
	}

	rep.content << "\n]";
	rep.add_header( "X-HANDLED-BY", "TreeHandler" );
	rep.set_basic_headers( "json" );

	return stop;
}
std::string
Tree::name() const { return "Tree"; }


