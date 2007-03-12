/* @(#)album.cpp
 */


#include "spinny/album.hpp"
#include "spinny/artist.hpp"
#include "boost/filesystem/operations.hpp"

namespace Spinny {

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
	if ( reader->num_columns() == 3 ){
		num_songs_=reader->get<sqlite::id_t>( 1 );
		counts_loaded_=true;
	}
}

Album::Album() : sqlite::table(),
		 _name(""),
		 num_songs_(0),
		 counts_loaded_(false)
{}

bool
Album::save() const {
	return sqlite::db()->save(*this);
}
// END DB METHODS


Album::ptr
Album::load( sqlite::id_t db_id ){
	return sqlite::db()->load<Album>( db_id );
}



Album::result_set
Album::all(){
	return sqlite::db()->load_all<Album>( "upper(name)" );
}

Album::result_set
Album::with_artist_id( sqlite::id_t db_id ){
	std::string where("rowid in ( select album_id from albums_artists where artist_id = ");
	where += boost::lexical_cast<std::string>( db_id );
	where += ")";
	return sqlite::db()->load_where<Album>( where,"upper(name)" );
}

Album::result_set
Album::name_starts_with( const std::string &name ){
	sqlite::connection *con = sqlite::db();
	*con << "select ";
	table_desc.insert_fields( *con );
	*con << ",(select count(*) from songs where album_id=albums.rowid),albums.rowid"
	     << " from albums where upper(name) like '" << sqlite::q( name, false ) << "%'" << "order by upper(name)";
	return sqlite::db()->load_stored<Album>();
}

Album::starting_char_t
Album::starting_chars() {
	starting_char_t ret;
	sqlite::command cmd( sqlite::db(), "select upper( substr(name,1,1) ),count(rowid) from albums where length(name)>0 group by upper( substr(name,1,1) )");
	for ( sqlite::command::reader_iterator it=cmd.reader_begin(); cmd.reader_end() != it; ++it ){
		ret.push_back( starting_char_t::value_type( it->get<char>(0), it->get<unsigned int>(1) ) );
	}
	return ret;
}

sqlite::id_t
Album::count(){
	return sqlite::db()->count<Album>();
}

sqlite::id_t
Album::num_songs(){
	if ( counts_loaded_ ){
		return num_songs_;
	} else {
		sqlite::connection *con = sqlite::db();
		*con << "select count(*) from songs where album_id = "
		     << this->db_id();
		return con->exec<sqlite::id_t>();
	}
}

void
Album::add_artist( const Artist::ptr &artist ){
	sqlite::connection *con = sqlite::db();
	*con << "select count(*) from albums_artists where album_id = " << this->db_id() << " and artist_id = " << artist->db_id();
	if ( ! con->exec<int>() ){
		this->save_if_needed();
		*con << "insert into albums_artists( album_id, artist_id ) values ( " << this->db_id() << "," << artist->db_id() << ")";
		BOOST_LOGL( app,info ) << "Added artist " << artist->db_id() << " to " << this->db_id();
		con->exec<sqlite::none>();
	}
}

Song::result_set
Album::songs() const {
	return sqlite::db()->load_many<Song>( "album_id", db_id(),"track,upper(title)" );
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


} // namespace Spinny
