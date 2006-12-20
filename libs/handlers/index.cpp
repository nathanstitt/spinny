#include "ews/server.hpp"
#include "handlers/index.hpp"
#include "spinny/music_dir.hpp"
#include "spinny/album.hpp"
#include "spinny/artist.hpp"
#include "boost/lambda/lambda.hpp"
#include "boost/lambda/bind.hpp"
#include "boost/nondet_random.hpp"
#include "handlers/shared.hpp"
#include <algorithm>


using namespace handlers;



static
void
insert( ews::reply &rep,
	int num,
	char id,
	std::string title,
	sqlite::id_t num_children,
	JsonReqType type )
{
	boost::replace_all( title, "\"","\\\"" );
	rep.content << "  " << num
		    << " {"
		    << "\n     label=" << title
		    << "\n     id=" << id
		    << "\n     type=" << (char)type 
		    << "\n     ch=" << num_children
		    << "\n    }\n";

}
static
void
insert( ews::reply &rep,
	int num,
	sqlite::id_t id,
	std::string title,
	sqlite::id_t num_children,
	JsonReqType type )
{
	boost::replace_all( title, "\"","\\\"" );
	rep.content << "  " << num
		    << " {"
		    << "\n     label=" << title
		    << "\n     id=" << id
		    << "\n     type=" << (char)type 
		    << "\n     ch=" << num_children
		    << "\n    }\n";

}



void
hdf_insert_dirs( ews::reply &rep ){ 
	rep.content << "RootDirectories {\n";
	Spinny::MusicDir::result_set rs = Spinny::MusicDir::roots();
	Spinny::MusicDir::ptr songs_dir;
	if ( rs.size() == 1 ){
		songs_dir = rs.begin().shared_ptr();
		rs = songs_dir->children();
	}
	int num=0;
	for(Spinny::MusicDir::result_set::iterator md = rs.begin(); rs.end() != md; ++md ){
		insert( rep, num++, md->db_id(), md->filesystem_name(), md->num_children()+md->num_songs(), JsonDir );
	}
	if ( songs_dir ){
		Spinny::Song::result_set songs=songs_dir->songs();
		for( Spinny::Song::result_set::iterator song = songs.begin(); songs.end() != song; ++song ){
			insert( rep, num++, song->db_id(), song->name(), 0, JsonSong );
		}
	}
	rep.content << "}\n";
}

void
hdf_insert_albums( ews::reply &rep ){ 
	rep.content << "Albums {\n";
	int num=0;
	if ( Spinny::Album::count() > 100 ){
		Spinny::Album::starting_char_t chars = Spinny::Album::starting_chars();
		for ( Spinny::Album::starting_char_t::const_iterator c = chars.begin(); chars.end() != c; ++c ){
			std::string title;
			title += c->first;
			insert( rep, num++, c->first, title, c->second, JsonAlbumsFirstChar );
		}
	} else {
		Spinny::Album::result_set rs = Spinny::Album::all();
		for ( Spinny::Album::result_set::iterator album = rs.begin(); rs.end() != album; ++album ){
			insert( rep, num++,album->db_id(),album->name(),album->num_songs(), JsonAlbum );
		}
	}
	rep.content << "}\n";
}

void
hdf_insert_artists( ews::reply &rep ){ 
	rep.content << "Artists {\n";
	int num=0;
	if ( Spinny::Artist::count() > 100 ){
		Spinny::Artist::starting_char_t chars = Spinny::Artist::starting_chars();
		for ( Spinny::Artist::starting_char_t::const_iterator c = chars.begin(); chars.end() != c; ++c ){
			std::string title;
			title += c->first;
			insert( rep, num++, c->first, title, c->second, JsonArtistsFirstChar );
		}
	} else {
		Spinny::Artist::result_set rs = Spinny::Artist::all();
		for ( Spinny::Artist::result_set::iterator artist = rs.begin(); rs.end() != artist; ++artist ){
			insert( rep, num++,artist->db_id(),artist->name(),artist->num_songs(), JsonArtistsAlbum );
		}
	}
	rep.content << "}\n";
}


std::string
make_ticket(){

	std::string ticket;
	boost::random_device dev;
	for ( int i=0; i<10; ++i ){
		BOOST_LOGL( www,info ) << "RAND " << i << " : " << dev();
		ticket+="a";
	}
	return ticket;
}

Index::Index() : ews::request_handler( "Index", Middle ) {}

ews::request_handler::RequestStatus
Index::handle( const ews::request& req, ews::reply& rep ) const {
	BOOST_LOGL( www, debug ) << name() << " examine " << req.url;

	if ( ! req.u1.empty() ){
		return Continue;
	}

	rep.set_basic_headers( "html" );

	if ( req.user->has_modify_role() ) {
		rep.set_template( "full.html" );

		hdf_insert_dirs( rep );
		hdf_insert_albums( rep );
		hdf_insert_artists( rep );
		rep.set_hdf_value( "isAdmin", req.user->is_admin() );
		rep.set_hdf_value( "NumArtists", Spinny::Artist::count() );
		rep.set_hdf_value( "NumAlbums", Spinny::Album::count() );
		rep.set_hdf_value( "NumSongs", Spinny::Song::count() );
	} else {
		

	}

	

	return Stop;
}



