#include "handlers/song.hpp"
#include "spinny/user.hpp"
#include "spinny/song.hpp"
#include "spinny/playlist.hpp"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem/exception.hpp" 
#include "boost/filesystem/fstream.hpp"

using namespace handlers;

Song::Song() : ews::request_handler( "Song", Beginning ) {}

ews::request_handler::RequestStatus
Song::handle( const ews::request& req, ews::reply& rep ) const {

	if ( req.u1 != "song" ){
		return Continue;
	}

	if ( req.u3.empty() || req.u4.empty() ){
		throw ews::error("Unable to determine user ticket or song");
	}

	Spinny::User::ptr user = Spinny::User::with_ticket( req.u3 );
	if ( ! user ){
		throw ews::forbidden_error( "Unown user, please log in and reload playlist." );
	}

	if (  user->last_visit < boost::posix_time::second_clock::local_time() - boost::gregorian::days(1) ){
		throw ews::forbidden_error( "Access has timed out, please log in again" );
	}

	Spinny::Song::ptr song = Spinny::Song::load( boost::lexical_cast<sqlite::id_t>( req.u4 ) );
	if ( ! user ){
		throw ews::not_found_error("Unable to find song");
	}

	if ( req.u2 == "dl" ){
		std::string disposition("attachment; filename=\"");
		disposition+=song->name()+".mp3\"";
		rep.set_header( "Content-disposition", disposition );

		// Open the file to send back.
		boost::filesystem::ifstream is;
		try {
			is.open( song->path(), std::ios::in | std::ios::binary);
		}
		catch ( boost::filesystem::filesystem_error &e ){
			BOOST_LOGL( www, err ) << "Caught filesystem error: " 
					       << e.what() 
					       << " req file was: "
					       << song->path().string();
			throw ews::not_found_error( "Unable to find song on disk.  Perhaps it was moved?" );
		}
		if (!is) {
			BOOST_LOGL( www, debug ) << "File not opened successfully: " 
						 << song->path().string();
			throw ews::not_found_error( "Unable to find song on disk.  Perhaps it was moved?" );
		}

		unsigned int sz=0;
		char buf[512];
		while ( is.read(buf, sizeof(buf)).gcount() > 0 ){
			sz+=is.gcount();
			rep.content.write( buf, is.gcount() );
		}
		rep.set_basic_headers( "mp3" );

		BOOST_LOGL( www, info ) << "Serving song id " << song->db_id() << " len " << sz << " from " << song->path().string();
	}

	return Stop;

}
