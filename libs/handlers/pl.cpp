/* @(#)pl.cpp
 */


#include "handlers/pl.hpp"
#include "sqlite/comma.hpp"

#include "spinny/playlist.hpp"
#include "spinny/song.hpp"
#include "spinny/artist.hpp"
#include "spinny/album.hpp"
#include "boost/algorithm/string/predicate.hpp"
#include "boost/algorithm/string/classification.hpp"

#include "boost/filesystem/path.hpp"
#include "boost/filesystem/exception.hpp" 
#include "boost/filesystem/fstream.hpp"

using namespace handlers;

void
insert( Spinny::PlayList::ptr pl,sqlite::commas &comma, ews::reply &rep ){
	rep.content << comma << "\n    " << "{"
		    << "\"id\":" << pl->db_id()
		    << ",\"name\":\"" << json_q( pl->name() ) << "\""
		    << ",\"description\":\"" <<  json_q( pl->description() ) << "\"}";
}

PL::PL() : ews::request_handler( "Playlists", Middle ) {}

template< typename T >
void
test_loaded( T obj ){
	if ( ! obj ){
		throw ews::error("Failed to load!");
	}
}

void
add_something( Spinny::PlayList::ptr pl, const std::string &type, sqlite::id_t id, sqlite::id_t pos ){
	if ( type == "song" ){
		Spinny::Song::ptr song = Spinny::Song::load( id );
		test_loaded( song );
		pl->insert( song,  pos );
	} else if ( type == "dir" ){
		Spinny::MusicDir::ptr md = Spinny::MusicDir::load( id );
		test_loaded( md );
		pl->insert( md, pos );
	} else if ( type == "artist" ){
		Spinny::Artist::ptr artist = Spinny::Artist::load( id );
		test_loaded( artist );
		pl->insert( artist, pos );
	} else if ( type == "pl" ){
		Spinny::PlayList::ptr pl = Spinny::PlayList::load( id );
		test_loaded( pl );
		pl->insert( pl, pos );
	}
}

ews::request_handler::RequestStatus
PL::handle( const ews::request& req, ews::reply& rep ) const {
//	BOOST_LOGL( www, info ) << name() << " examine " << req.url << " req.u1: " << req.u1;

 	if ( req.u1 != "pl"){
		return Continue;
	}

	sqlite::commas comma;

	if ( req.u2 == "songs" ){
		BOOST_LOGL(www,debug) << "Listing Songs for pl id: " << req.svalue( "pl_id" );

		Spinny::PlayList::ptr pl = Spinny::PlayList::load( req.single_value<sqlite::id_t>( "pl_id" ) );
		if ( ! pl ){
			throw ews::error("Unable to load playlist");
		}
		if ( req.u3 == "list" ){
			Spinny::Song::result_set songs = pl->songs( req.svalue("sort"),
								    req.single_value<sqlite::id_t>("start"),
								    req.single_value<sqlite::id_t>("limit") );
			rep.content << "{ Songs:  [ \n";
			for ( Spinny::Song::result_set::iterator song = songs.begin(); song != songs.end(); ++song ){
				rep.content << comma << "{ 'id':" << song->db_id()
					    << ",'tr':" << song->track() 
					    << ",'tt':'" << json_q( song->title() ) << "', "
					    << "'at':'" << json_q( song->artist()->name() ) << "', "
					    << "'al':'" << json_q( song->album()->name() ) << "',"
					    << "'ln':" << song->length() << "}\n";
			}
			rep.content  << "], Size: " << pl->size() << " }\n";
		} else if ( req.u3 == "reorder" ) {
			for ( ews::request::varibles_t::const_iterator var=req.varibles.begin();
			      req.varibles.end() != var; ++var ){
				if ( boost::all( var->first, boost::is_digit() ) ) {
					BOOST_LOGL( www,debug ) << "Song id: " << var->first << " == " << var->second.front();
					pl->set_song_order(  boost::lexical_cast<sqlite::id_t>(var->first),
							     boost::lexical_cast<int>( var->second.front() ) );
				}
			}
		} else if ( req.u3 == "addsongs" ){
			ews::request::varibles_t::const_iterator song_ids=req.varibles.find( "song_id" );
			if ( song_ids != req.varibles.end() ){
				sqlite::id_t pos = req.single_value<sqlite::id_t>( "pos" );
				for ( ews::request::varible_t::const_iterator song_id = song_ids->second.begin();
					      song_ids->second.end() != song_id; ++song_id ){

					Spinny::Song::ptr song = pl->load_song( boost::lexical_cast<sqlite::id_t>( *song_id ) );
					test_loaded( song );
					pl->insert( song,  pos++ );
				}
			}
		} else if ( req.u3 == "add" ){
			add_something( pl, req.u4, req.single_value<sqlite::id_t>( "id" ), req.single_value<sqlite::id_t>( "pos" ) );
		} else if ( req.u3 == "rm" ){
			pl->remove( req.single_value<sqlite::id_t>( "song_id" ) );
		} else if ( req.u3 == "addpl" ){
			ews::request::varibles_t::const_iterator ids = req.varibles.find("pl_id");
			if ( ids != req.varibles.end() ){
				for ( ews::request::varible_t::const_iterator pl_id = ids->second.begin();
					      ids->second.end() != pl_id; ++pl_id ){

					BOOST_LOGL( www,debug ) << "Adding Playlist id's " << *pl_id << " songs to playlist";

					Spinny::PlayList::ptr p = Spinny::PlayList::load( boost::lexical_cast<sqlite::id_t>( *pl_id ) );
					
					pl->insert( p, req.single_value<sqlite::id_t>( "position" ) );
				}
			}
			
		} else if ( req.u3 == "select" ){
			BOOST_LOGL(www,debug) << "Loading song " << req.single_value<sqlite::id_t>( "song_id" );
			Spinny::Song::ptr song = pl->load_song( req.single_value<sqlite::id_t>( "song_id" ) );
			Spinny::App::instance()->streaming->select_song( pl, song );
		}
	} else if ( req.u2 == "list" ){

		BOOST_LOGL(www,debug) << "Loading PL";
		rep.content << "{Playlists: [";
		Spinny::PlayList::result_set rs = Spinny::PlayList::all();
		BOOST_LOGL(www,debug) << "Loading PL all()";
		for ( Spinny::PlayList::result_set::iterator pl = rs.begin(); rs.end() != pl; ++pl ){

			BOOST_LOGL(www,debug) << "Loading PL id: " << pl->db_id();
			insert( pl.shared_ptr(), comma, rep );
		}
		rep.content << "]}";
	} else if ( req.u2 == "update" ){
		Spinny::PlayList::ptr pl;
		if ( req.svalue("pl_id" ) == "0" ){
			pl = Spinny::PlayList::create();
		} else {
			pl = Spinny::PlayList::load( req.single_value<sqlite::id_t>("pl_id" ) );
		}
		if ( req.u3 == "name" ){
			pl->set_name( req.svalue("text") );
		} else {
			pl->set_description( req.svalue("text") );
		}
		pl->save();
	} else if ( req.u2 == "reorder" )  {
		for ( ews::request::varibles_t::const_iterator var=req.varibles.begin();
		      req.varibles.end() != var; ++var ){
			if ( boost::all( var->first, boost::is_digit() ) ) {
				BOOST_LOGL( www,debug ) << var->first << " == " << var->second.front();
				Spinny::PlayList::set_order(  boost::lexical_cast<sqlite::id_t>(var->first), boost::lexical_cast<int>( var->second.front() ) );
			}
		}

	}



	rep.set_header( "X-HANDLED-BY", name() );
	rep.set_basic_headers( "json" );

	return Stop;
}





