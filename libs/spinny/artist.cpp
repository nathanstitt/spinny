/* @(#)artist.cpp
 */

#include "spinny/artist.hpp"
#include "spinny/song.hpp"
#include "spinny/album.hpp"

#include "boost/filesystem/operations.hpp"


namespace Spinny {

class artist_desc : public sqlite::table::description {
public:
	virtual const char* table_name() const {
		return "artists";
	};
	virtual int num_fields() const {
		return 1;
	}
	virtual const char** fields() const {
		static const char *fields[] = {
			"name",
		};
		return fields;
	}
	virtual const char** field_types() const {
		static const char *field_types[] = {
			"string",
		};
		return field_types;
	};
};



static artist_desc table_desc;



const sqlite::table::description*
Artist::table_description(){
	return &table_desc;
}


const sqlite::table::description*
Artist::m_table_description() const {
	return &table_desc;
}


void
Artist::table_insert_values( std::ostream &str ) const {
	str << sqlite::q(name_);
}

void
Artist::table_update_values( std::ostream &str ) const {
	str << "name=" << sqlite::q(name_);
}


void
Artist::initialize_from_db( const sqlite::reader *reader ) {
	name_	   = reader->get<std::string>(0);
	if ( reader->num_columns() == 4 ){
		num_songs_=reader->get<sqlite::id_t>( 1 );
		num_albums_=reader->get<sqlite::id_t>( 2 );
		counts_loaded_=true;
	}
}


Artist::Artist() : sqlite::table(),
		   name_(""),
		   num_songs_(0),
		   counts_loaded_(false)
{}


bool
Artist::save() const {
	return sqlite::db()->save(*this);
}
// END DB METHODS


Artist::ptr
Artist::load( sqlite::id_t db_id ){
	return sqlite::db()->load<Artist>( db_id );
}

Artist::result_set
Artist::all(){
	return sqlite::db()->load_all<Artist>( "upper(name)" );
}


Song::result_set
Artist::songs() const {
	return sqlite::db()->load_many<Song>( "artist_id", db_id(),"track,upper(title)" );
}



Artist::result_set
Artist::with_album( const Album *alb ){
	sqlite::connection *con = sqlite::db();
	*con << "select ";
	table_desc.insert_fields( *con );
	*con << ",artists.rowid from artists, songs where artists.rowid=songs.artist_id and songs.album_id = "
	     << alb->db_id() << " order by upper(albums.name)";
	return con->load_stored<Artist>();
}

Artist::result_set
Artist::name_starts_with( const std::string &name ){
	sqlite::connection *con = sqlite::db();
	*con << "select ";
	table_desc.insert_fields( *con );
	*con << ",rowid from artists where upper(name) like '" << sqlite::q( name, false ) << "%'" << " order by upper(name)";
	return sqlite::db()->load_stored<Artist>();
}


Artist::starting_char_t
Artist::starting_chars() {
	starting_char_t ret;
	sqlite::command cmd( sqlite::db(), "select upper( substr(name,1,1) ),count(rowid) from artists where length(name)>0 group by upper( substr(name,1,1) ) order by upper( substr(name,1,1) );");
	for ( sqlite::command::reader_iterator it=cmd.reader_begin(); cmd.reader_end() != it; ++it ){
		ret.push_back( starting_char_t::value_type( it->get<char>(0), it->get<unsigned int>(1) ) );
	}
	return ret;
}

sqlite::id_t
Artist::count(){
	return sqlite::db()->count<Artist>();
}

sqlite::id_t
Artist::num_albums(){
	if ( counts_loaded_ ){
		return num_albums_;
	} else {
		sqlite::connection *con = sqlite::db();
		*con << "select count( distinct(album_id) ) from songs where  songs.artist_id = "
		     << this->db_id();
		return con->exec<sqlite::id_t>();
	}
}

sqlite::id_t
Artist::num_songs(){
	if ( counts_loaded_ ){
		return num_songs_;
	} else {

		sqlite::connection *con = sqlite::db();
		*con << "select count(*) from songs where artist_id = "
		     << this->db_id();
		return con->exec<sqlite::id_t>();
	}
}



Album::result_set
Artist::albums() const {
	sqlite::connection *con = sqlite::db();
	const sqlite::table::description *td = Album::table_description();
	*con << "select ";
	td->insert_fields( *con );
	*con << ",albums.rowid from albums,songs where albums.rowid=songs.album_id and songs.artist_id = "
	     << this->db_id() << " order by upper(albums.name)";
	return con->load_stored<Album>();
}


Artist::ptr
Artist::find_or_create( const std::string &name ){
	Artist::ptr ret=sqlite::db()->load_one<Artist>( "name", name );
	if ( ! ret ){
		Artist *a = new Artist;
		a->name_=name;
		a->save();
		ret.reset( a );
	}
	return ret;
}


std::string
Artist::name() const {
	return name_;
}

} // namespace Spinny
