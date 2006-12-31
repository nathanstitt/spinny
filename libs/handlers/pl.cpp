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

ews::request_handler::RequestStatus
PL::handle( const ews::request& req, ews::reply& rep ) const {
//	BOOST_LOGL( www, info ) << name() << " examine " << req.url << " req.u1: " << req.u1;

 	if ( req.u1 != "pl"){
		return Continue;
	}

	sqlite::commas comma;

	if ( req.u2 == "songs" ){
		BOOST_LOGL(www,info) << "Listing Songs for pl id: " << req.single_value<sqlite::id_t>( "pl_id" );
		Spinny::PlayList::ptr pl = Spinny::PlayList::load( req.single_value<sqlite::id_t>( "pl_id" ) );
		if ( req.u3 == "list" ){
			Spinny::Song::result_set songs = pl->songs();
			rep.content << "{ Songs:  [ \n";
			for ( Spinny::Song::result_set::iterator song = songs.begin(); song != songs.end(); ++song ){
				rep.content << comma << "{ 'id':" << song->db_id()
					    << ",'tr':" << song->track() 
					    << ",'tt':'" << json_q( song->title() ) << "', "
					    << "'at':'" << json_q( song->artist()->name() ) << "', "
					    << "'al':'" << json_q( song->album()->name() ) << "',"
					    << "'ln':" << song->length() << "}\n";
			}
			rep.content  << "]}\n";				

		} else if ( req.u3 == "reorder" ) {
			for ( ews::request::varibles_t::const_iterator var=req.varibles.begin();
			      req.varibles.end() != var; ++var ){
				if ( boost::all( var->first, boost::is_digit() ) ) {
					BOOST_LOGL( www,info ) << "Song id: " << var->first << " == " << var->second.front();
					pl->set_song_order(  boost::lexical_cast<sqlite::id_t>(var->first),
							     boost::lexical_cast<int>( var->second.front() ) );
				}
			}
		} else if ( req.u3 == "addsong" ){
			Spinny::Song::ptr song = Spinny::Song::load( req.single_value<sqlite::id_t>( "song_id" ) );
			pl->insert( song,  req.single_value<sqlite::id_t>( "position" ) );
		} else if ( req.u3 == "rm" ){
			pl->remove( req.single_value<sqlite::id_t>( "song_id" ) );
		} else if ( req.u3 == "addpl" ){
			ews::request::varibles_t::const_iterator ids = req.varibles.find("pl_id");
			if ( ids != req.varibles.end() ){
				for ( ews::request::varible_t::const_iterator pl_id = ids->second.begin();
					      ids->second.end() != pl_id; ++pl_id ){

					BOOST_LOGL( www,info ) << "Adding Playlist id's " << *pl_id << " songs to playlist";

					Spinny::PlayList::ptr p = Spinny::PlayList::load( boost::lexical_cast<sqlite::id_t>( *pl_id ) );
					
					pl->insert( p, req.single_value<sqlite::id_t>( "position" ) );
				}

			}

//			pl->insert( 
			
		}
	} else if ( req.u2 == "list" ){

		BOOST_LOGL(www,info) << "Loading PL";
		rep.content << "{Playlists: [";
		Spinny::PlayList::result_set rs = Spinny::PlayList::all();
		BOOST_LOGL(www,info) << "Loading PL all()";
		for ( Spinny::PlayList::result_set::iterator pl = rs.begin(); rs.end() != pl; ++pl ){

			BOOST_LOGL(www,info) << "Loading PL id: " << pl->db_id();
			insert( pl.shared_ptr(), comma, rep );
		}
		rep.content << "]}";
	} else if ( req.u2 == "create" ){
		BOOST_LOGL( www,info ) << "Creating Playlist:"
				       << "\nName:        " << req.svalue("name")
				       << "\nBitrate:     " << req.single_value<int>("bitrate")
				       << "\nDescription: " << req.svalue("description");
		Spinny::PlayList::ptr pl =
			Spinny::PlayList::create( 0,
						  req.svalue("name"),
						  req.svalue("description") );
		
		if ( ! pl->save() ){
			BOOST_LOGL( www,err ) << "Playlist Creation Failed!"
				       << "\nName:        " << req.svalue("name")
				       << "\nBitrate:     " << req.svalue("bitrate")
				       << "\nDescription: " << req.svalue("description");
			throw ews::error("Failed to create playlist");
		}
		rep.content << "{Playlists: [";
		insert( pl, comma, rep );
		rep.content << "]}";
	} else if ( req.u2 == "modify" ){
		Spinny::PlayList::ptr pl = Spinny::PlayList::load( req.single_value<sqlite::id_t>("pl_id" ) );
		pl->set_bitrate( 0 );
		pl->set_name( req.svalue("name") );
		pl->set_description( req.svalue("description") );
		pl->save();
		rep.content << "{Playlists: [";
		insert( pl, comma, rep );
		rep.content << "]}";
	} else if ( req.u2 == "reorder" )  {
		for ( ews::request::varibles_t::const_iterator var=req.varibles.begin();
		      req.varibles.end() != var; ++var ){
			if ( boost::all( var->first, boost::is_digit() ) ) {
				BOOST_LOGL( www,info ) << var->first << " == " << var->second.front();
				Spinny::PlayList::set_order(  boost::lexical_cast<sqlite::id_t>(var->first), boost::lexical_cast<int>( var->second.front() ) );
			}
		}

	}



	rep.set_header( "X-HANDLED-BY", name() );
	rep.set_basic_headers( "json" );

	return Stop;
}





