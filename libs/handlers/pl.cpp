/* @(#)pl.cpp
 */


#include "handlers/pl.hpp"
#include "spinny/playlist.hpp"
#include "sqlite/comma.hpp"

using namespace handlers;

void
insert( PlayList::ptr pl,sqlite::commas &comma, ews::reply &rep ){
	rep.content << comma << "\n    " << "{"
		    << "\"id\":" << pl->db_id()
		    << ",\"name\":\"" << boost::replace_all_copy( pl->name(), "\"", "\\\"") << "\""
		    << ",\"description\":\"" <<  boost::replace_all_copy( pl->description(), "\"", "\\\"") << "\"}";
}

ews::request_handler::result
PL::handle( const ews::request& req, ews::reply& rep ) const {
//	BOOST_LOGL( www, info ) << name() << " examine " << req.url << " req.u1: " << req.u1;

 	if ( req.u1 != "pl"){
		return cont;
	}

	sqlite::commas comma;

	if ( req.u2 == "songs" ){
		rep.content << "{ Songs:  [ \n{ 'id':3441, 'tr':1, 'tt':'A Title 1', 'at':'Billy Bob 1', 'al':'Harmonious Songs','ln':333},\n"
			    << "{ 'id':3442, 'tr':1, 'tt':'A Title 2', 'at':'Billy Bob 2', 'al':'Harmonious Songs','ln':333},\n"
			    << "{ 'id':3443, 'tr':1, 'tt':'A Title 3', 'at':'Billy Bob 3', 'al':'Harmonious Songs','ln':333},\n"
			    << "{ 'id':3444, 'tr':1, 'tt':'A Title 4', 'at':'Billy Bob 4', 'al':'Harmonious Songs','ln':333}\n"
			    << "]}\n";

	} else if ( req.u2 == "list" ){
		rep.content << "{Playlists: [";
		PlayList::result_set rs = PlayList::all();
		for ( PlayList::result_set::iterator pl = rs.begin(); rs.end() != pl; ++pl ){
			insert( pl.shared_ptr(), comma, rep );
		}
		rep.content << "]}";
	} else if ( req.u2 == "create" ){
		BOOST_LOGL( www,info ) << "Creating Playlist:"
				       << "\nName:        " << req.single_value<std::string>("name")
				       << "\nBitrate:     " << req.single_value<int>("bitrate")
				       << "\nDescription: " << req.single_value<std::string>("description");
		PlayList::ptr pl =
			PlayList::create( 0,
					  req.single_value<std::string>("name"),
					  req.single_value<std::string>("description") );
		
		if ( ! pl->save() ){
			BOOST_LOGL( www,err ) << "Playlist Creation Failed!"
				       << "\nName:        " << req.single_value<std::string>("name")
				       << "\nBitrate:     " << req.single_value<int>("bitrate")
				       << "\nDescription: " << req.single_value<std::string>("description");
			return error;
		}
		rep.content << "{Playlists: [";
		insert( pl, comma, rep );
		rep.content << "]}";
	} else if ( req.u2 == "mod" ){
		PlayList::ptr pl = PlayList::load( boost::lexical_cast<sqlite::id_t>( req.u3 ) );
		pl->set_bitrate( 0 );
		pl->set_name( req.single_value<std::string>("name") );
		pl->set_description( req.single_value<std::string>("description") );
		pl->save();
		rep.content << "{Playlists: [";
		insert( pl, comma, rep );
		rep.content << "]}";
	} else if ( req.u2 == "reorder" ) {
		for ( ews::request::varibles_t::const_iterator var=req.varibles.begin();
		      req.varibles.end() != var; ++var ){
			BOOST_LOGL( www,info ) << var->first << " == " << var->second.front();
			PlayList::set_order(  boost::lexical_cast<sqlite::id_t>(var->first), boost::lexical_cast<int>( var->second.front() ) );
		}


	}



	rep.add_header( "X-HANDLED-BY", name() );
	rep.set_basic_headers( "json" );

	return stop;
}
std::string
PL::name() const { return "PlayList"; }





