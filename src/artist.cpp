/* @(#)artist.cpp
 */

#include <config.h>
#include "artist.hpp"

#include "song.hpp"

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
Artist::save(){
	return Spinny::db()->save(*this);
}
// END DB METHODS



Song::result_set
Artist::songs() const {
	return Spinny::db()->load_many<Song>( "artist_id", db_id() );
}


// Artist::ptr
// Artist::load_or_create( const std::string &name ){
// 	Artist a = Spinny::db()->load<Song>( "name", name );
// 	if ( a.loaded() ){
// 		return a;
// 	} else {
		
// 	}
// }
