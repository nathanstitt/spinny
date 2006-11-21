#include "ews/server.hpp"
#include "handlers/index.hpp"
#include "spinny/music_dir.hpp"
#include "spinny/album.hpp"
#include "spinny/artist.hpp"
#include "boost/lambda/lambda.hpp"
#include "boost/lambda/bind.hpp"
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
	MusicDir::result_set rs = MusicDir::roots();
	MusicDir::ptr songs_dir;
	if ( rs.size() == 1 ){
		songs_dir = rs.begin().shared_ptr();
		rs = songs_dir->children();
	}
	int num=0;
	for(MusicDir::result_set::iterator md = rs.begin(); rs.end() != md; ++md ){
		insert( rep, num++, md->db_id(), md->filesystem_name(), md->num_children()+md->num_songs(), JsonDir );
	}
	if ( songs_dir ){
		Song::result_set songs=songs_dir->songs();
		for( Song::result_set::iterator song = songs.begin(); songs.end() != song; ++song ){
			insert( rep, num++, song->db_id(), song->name(), 0, JsonSong );
		}
	}
	rep.content << "}\n";
}

void
hdf_insert_albums( ews::reply &rep ){ 
	rep.content << "Albums {\n";
	int num=0;
	if ( Album::count() > 100 ){
		Album::starting_char_t chars = Album::starting_chars();
		for ( Album::starting_char_t::const_iterator c = chars.begin(); chars.end() != c; ++c ){
			std::string title;
			title += c->first;
			insert( rep, num++, c->first, title, c->second, JsonAlbumsFirstChar );
		}
	} else {
		Album::result_set rs = Album::all();
		for ( Album::result_set::iterator album = rs.begin(); rs.end() != album; ++album ){
			insert( rep, num++,album->db_id(),album->name(),album->num_songs(), JsonAlbum );
		}
	}
	rep.content << "}\n";
}

void
hdf_insert_artists( ews::reply &rep ){ 
	rep.content << "Artists {\n";
	int num=0;
	if ( Artist::count() > 100 ){
		Artist::starting_char_t chars = Artist::starting_chars();
		for ( Artist::starting_char_t::const_iterator c = chars.begin(); chars.end() != c; ++c ){
			std::string title;
			title += c->first;
			insert( rep, num++, c->first, title, c->second, JsonArtistsFirstChar );
		}
	} else {
		Artist::result_set rs = Artist::all();
		for ( Artist::result_set::iterator artist = rs.begin(); rs.end() != artist; ++artist ){
			insert( rep, num++,artist->db_id(),artist->name(),artist->num_songs(), JsonArtistsAlbum );
		}
	}
	rep.content << "}\n";
}

ews::request_handler::result
Index::handle( const ews::request& req, ews::reply& rep ) const {
	BOOST_LOGL( www, debug ) << name() << " examine " << req.url;

	if ( ! req.u1.empty() ){
		return cont;
	}

	rep.set_template( "index.html" );

	hdf_insert_dirs( rep );
	hdf_insert_albums( rep );
	hdf_insert_artists( rep );

	rep.set_hdf_value( "NumArtists", Artist::count() );
	rep.set_hdf_value( "NumAlbums", Album::count() );
	rep.set_hdf_value( "NumSongs", Song::count() );


	rep.add_header( "X-HANDLED-BY", name() );
	rep.set_basic_headers( "html" );
	
	return stop;
}
std::string
Index::name() const { return "Index"; }


