
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

static void
insert_artists( ews::reply &rep, sqlite::commas &comma ){
	Spinny::Artist::result_set rs = Spinny::Artist::all();
	for ( Spinny::Artist::result_set::iterator artist = rs.begin(); rs.end() != artist; ++artist ){
		rep.content << comma << "\n    " << "{"
			    << "\"id\":\"a" << artist->db_id()
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
			    << "\"id\":\"b" << album->db_id()
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
		sqlite::id_t dir_id = req.single_value<sqlite::id_t>("node");

		Spinny::MusicDir::result_set rs = Spinny::MusicDir::children_of( dir_id );

		BOOST_LOGL( www, info ) << "Tree loading children of Dir id: " << req.svalue("node");

		if ( rs.size() == 1 ){
			rs = rs.begin()->children();
		}
		for ( Spinny::MusicDir::result_set::iterator md = rs.begin(); rs.end() != md; ++md ){
			insert( rep, comma, md->db_id(), md->path().leaf(), md->num_children()+md->num_songs(), "dir" );
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
			insert_artists( rep, comma );
		} else if ( node.size() < 2 ){
			throw ews::error("Unable to load data");
		} else if ( node[0] == 'a' ){
			node.erase( 0, 1 );
			insert_albums( rep, boost::lexical_cast<sqlite::id_t>( node ), comma );
		} else if ( node[0] == 'b' ){
			node.erase( 0, 1 );
			Spinny::Album::ptr album = Spinny::Album::load( boost::lexical_cast<sqlite::id_t>( node ) );
			if ( ! album ){
				throw ews::error("Unable to load songs for album" );
			}
			insert_songs( rep, album->songs(), comma );
		}
	}


 	rep.content << "\n]";

 	rep.set_basic_headers( "json" );

 	return Stop;


}

// //	BOOST_LOGL( www, info ) << "Index T: " << req.url;

// 	if ( req.u1 != "tree" ) {
// 		return Continue;
// 	} else if ( req.u2.empty() || req.u3.empty() ){
// 		throw ews::error("Wasn't given any additional params for request");
// 	}

// 	rep.content << "[";
	
// 	JsonReqType type = static_cast<JsonReqType>( req.u2[0] );
	
// 	sqlite::commas comma;

// 	switch ( type ){

// 	case JsonDir: {
// 		Spinny::MusicDir::ptr md = Spinny::MusicDir::load(  boost::lexical_cast<sqlite::id_t>( req.u3 ) );
// 		BOOST_LOGL( www, info ) << "Tree loading children of Dir " << md->path().string() << " id: " << md->db_id();
// 		Spinny::MusicDir::result_set rs = md->children();
// 		for ( Spinny::MusicDir::result_set::iterator md = rs.begin(); rs.end() != md; ++md ){
// 			insert( rep, comma, md->db_id(), md->path().leaf(), md->num_children()+md->num_songs(), JsonDir );
// 		}
// 		insert_songs( rep, md->songs(),comma );
// 		break;
// 	}

// 	case JsonArtistsFirstChar: {
// 		BOOST_LOGL( www, info ) << "Tree loading artists starting with " << req.u3;
// 		Spinny::Artist::result_set rs = Spinny::Artist::name_starts_with( req.u3 );
// 		for ( Spinny::Artist::result_set::iterator artist = rs.begin(); rs.end() != artist; ++artist ){
//  			insert( rep,comma, artist->db_id(), artist->name(), artist->num_albums()+artist->num_songs(), JsonArtistsAlbum );
//  		}
// 		break;
// 	}

// 	case JsonAlbumsFirstChar: {
// 		BOOST_LOGL( www, info ) << "Tree loading albums starting with " << req.u3;
// 		Spinny::Album::result_set rs = Spinny::Album::name_starts_with( req.u3 );
// 		for ( Spinny::Album::result_set::iterator album = rs.begin(); rs.end() != album; ++album ){
//  			insert( rep,comma, album->db_id(), album->name(), album->num_songs(), JsonAlbum );
//  		}
// 		break;
// 	}

// 	case JsonAlbum: {
// 		Spinny::Album::ptr album = Spinny::Album::load( boost::lexical_cast<sqlite::id_t>( req.u3 ) );
// 		BOOST_LOGL( www, info ) << "Tree loading songs for album " << album->name() << " id: " << album->db_id();

// 		insert_songs( rep, album->songs(),comma );
// 		break;
// 	}

// 	case JsonArtistsAlbum: {
// 		Spinny::Artist::ptr artist = Spinny::Artist::load( boost::lexical_cast<sqlite::id_t>( req.u3 ) );
// 		BOOST_LOGL( www, info ) << "Tree loading albums and songs for artist " << artist->name() << " id: " << artist->db_id();
// 		Spinny::Album::result_set ars = artist->albums();
// 		for ( Spinny::Album::result_set::iterator album = ars.begin(); ars.end() != album; ++album ){
// 			insert( rep, comma, album->db_id(),album->name(), album->num_songs(), JsonAlbum );
// 		}
// 		insert_songs( rep, artist->songs(),comma );
// 		break;
// 	}

// 	default:
// 		BOOST_LOGL( www, err ) << "Tree Ajax handler recieved unknown request code: " << req.u2;
// 		throw ews::error( "Unknown code recieved" );
// 	}

// 	rep.content << "\n]";

// 	rep.set_basic_headers( "json" );

// 	return Stop;
//}


