/* @(#)album.cpp
 */

#include <config.h>
#include "album.hpp"
#include "artist.hpp"
#include "boost/filesystem/operations.hpp"
#include "id3lib/tag.h"

class album_desc : public sqlite::table::description {
public:
	virtual const char* table_name() const {
		return "albums";
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

static album_desc table_desc;

const sqlite::table::description*
Album::table_description(){
	return &table_desc;
}


const sqlite::table::description*
Album::m_table_description() const {
	return &table_desc;
}

void
Album::table_insert_values( std::ostream &str ) const {
	str << sqlite::q(_name);
}

void
Album::table_update_values( std::ostream &str ) const {
	str << "name=" << sqlite::q(_name);
}

void
Album::initialize_from_db( const sqlite::reader *reader ) {
	_name	   = reader->get<std::string>(0);
}

Album::Album() : sqlite::table(),
		 _name(""){}

bool
Album::save() const {
	return sqlite::db()->save(*this);
}
// END DB METHODS

Album::result_set
Album::all(){
	return sqlite::db()->load_many<Album>( "", 0 );
}


void
Album::add_artist( const Artist::ptr &artist ){
	this->save_if_needed();
	sqlite::connection *con = sqlite::db();
	*con << "insert into albums_artists( album_id, artist_id ) values ( " << this->db_id() << ","<< artist->db_id()<<")";
	con->exec<sqlite::none>();
}

Song::result_set
Album::songs() const {
	return sqlite::db()->load_many<Song>( "album_id", db_id() );
}

Album::ptr
Album::find_or_create( const Artist::ptr &artist, const std::string &name ){
	Album::ptr ret=sqlite::db()->load_one<Album>( "name", name );
	if ( ! ret ){
		Album *a = new Album;
		a->_name=name;
		a->save();
		a->add_artist( artist );
		ret.reset( a );
	}
	return ret;
}


std::string
Album::name() const {
	return _name;
}


Artist::result_set
Album::artists() const {
	return Artist::with_album( this );
}
