/* @(#)artist.cpp
 */

#include <config.h>
#include "artist.hpp"
#include "song.hpp"
#include "album.hpp"

#include "boost/filesystem/operations.hpp"
#include "id3lib/tag.h"

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

class albums_artists_desc : public sqlite::table::description {
public:
	virtual const char* table_name() const {
		return "albums_artists";
	};
	virtual int num_fields() const {
		return 2;
	}
	virtual const char** fields() const {
		static const char *fields[] = {
			"album_id",
			"artist_id",
		};
		return fields;
	}
	virtual const char** field_types() const {
		static const char *field_types[] = {
			"int",
			"int",
		};
		return field_types;
	};
};


static artist_desc table_desc;
static albums_artists_desc albums_artists_table_desc;


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
	str << sqlite::q(_name);
}

void
Artist::table_update_values( std::ostream &str ) const {
	str << "name=" << sqlite::q(_name);
}


void
Artist::initialize_from_db( const sqlite::reader *reader ) {
	_name	   = reader->get<std::string>(0);
}


Artist::Artist() : sqlite::table(),
		   _name(""){}


bool
Artist::save() const {
	return sqlite::db()->save(*this);
}
// END DB METHODS


Artist::result_set
Artist::all(){
	return sqlite::db()->load_many<Artist>( "", 0 );
}


Song::result_set
Artist::songs() const {
	return sqlite::db()->load_many<Song>( "artist_id", db_id() );
}



Artist::result_set
Artist::with_album( const Album *alb ){
	sqlite::connection *con = sqlite::db();
	*con << "select ";
	table_desc.insert_fields( *con );
	*con << ",artists.rowid from artists, albums_artists where artists.rowid=albums_artists.artist_id"
	     << " and albums_artists.album_id = " << alb->db_id();
	return con->load_stored<Artist>();
}

Artist::result_set
Artist::name_starts_with( const std::string &name ){
	std::string where("upper(name) like '");
	where += sqlite::q( name, false );
	where += "%'";
	return sqlite::db()->load_where<Artist>( where );
}

sqlite::id_t
Artist::count(){
	return sqlite::db()->count<Artist>();
}

Album::result_set
Artist::albums() const {
	sqlite::connection *con = sqlite::db();
	const sqlite::table::description *td = Album::table_description();
	*con << "select ";
	td->insert_fields( *con );
	*con << ",albums.rowid from albums, albums_artists where albums.rowid=albums_artists.album_id"
	     << " and albums_artists.artist_id = " << this->db_id();
	return con->load_stored<Album>();
}


Artist::ptr
Artist::find_or_create( const std::string &name ){
	Artist::ptr ret=sqlite::db()->load_one<Artist>( "name", name );
	if ( ! ret ){
		Artist *a = new Artist;
		a->_name=name;
		a->save();
		ret.reset( a );
	}
	return ret;
}


std::string
Artist::name() const {
	return _name;
}
