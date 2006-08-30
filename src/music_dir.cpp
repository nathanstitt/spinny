

#include "music_dir.hpp"
#include "boost/filesystem/operations.hpp"
#include "song.hpp" 
#include <vector>
#include <algorithm>
#include <iterator>

class md_desc : public sqlite::table::description {
public:
	virtual const char* table_name() const {
		return "music_dirs";
	};
	virtual int num_fields() const {
		return 2;
	}
	virtual const char** fields() const {
		static const char *fields[] = {
			"parent_id",
			"name",
		};
		return fields;
	}
	virtual const char** field_types() const {
		static const char *field_types[] = {
			"int",
			"string",
		};
		return field_types;
	};
};

static md_desc table_desc;


const sqlite::table::description*
MusicDir::table_description(){
	return &table_desc;
}


const sqlite::table::description*
MusicDir::m_table_description() const {
	return &table_desc;
}


void
MusicDir::table_insert_values( std::ostream &str ) const {
	str << _parent_id << "," << sqlite::q(_name);
}

void
MusicDir::table_update_values( std::ostream &str ) const {
	str << "parent_id=" << _parent_id << ",name=" << sqlite::q(_name);
}


void
MusicDir::initialize_from_db( const sqlite::reader *reader ) {
	_parent_id = reader->get<int>(0);
	_name	   = reader->get<std::string>(1);
}

MusicDir::MusicDir() : sqlite::table(), _parent_id(0), _name("") {

}


// public methods



MusicDir
MusicDir::create_root( const boost::filesystem::path &path ){
 	MusicDir md;
 	md._name=path.string();
 	md._parent_id = 0;
 	md.save();
	return md;
}

MusicDir
MusicDir::add_child( const std::string &name ){
	MusicDir md;
	md._name=name;
	md._parent_id = this->db_id();
	md.save();
	return md;
}

MusicDir
MusicDir::load( sqlite::id_t db_id ){
	return Spinny::db()->load<MusicDir>( db_id );
}


MusicDir::result_set
MusicDir::roots(){
	return Spinny::db()->load_many<MusicDir>( "parent_id", 0 );
}

bool
MusicDir::save() {
	return Spinny::db()->save<MusicDir>(*this);
}


bool
MusicDir::is_root() const {
 	return ( _parent_id == 0 );
}


string
MusicDir::name() const {
	return _name; 
}



MusicDir
MusicDir::parent() const {
	return Spinny::db()->load<MusicDir>( _parent_id );
}

Song::result_set
MusicDir::songs(){
 	return Spinny::db()->load_many<Song>( "dir_id", db_id() );
}

MusicDir::result_set
MusicDir::children() const {
	return Spinny::db()->load_many<MusicDir>( "parent_id", db_id() );
}





boost::filesystem::path
MusicDir::path() const {
	vector<string> dirs;
	dirs.push_back( _name );
	MusicDir md = *this;
	while ( ! md.is_root() ) {
		md=md.parent();
		dirs.push_back( md._name );
	}
	boost::filesystem::path p("", boost::filesystem::native );
	for ( vector<string>::reverse_iterator ri = dirs.rbegin(); ri != dirs.rend(); ++ri ){
		p /= *ri; 
	}

	return p;
}


bool
MusicDir::is_valid(){
	return boost::filesystem::exists( this->path() );
}


void
MusicDir::sync(){
	boost::filesystem::path path = this->path();

 	if ( ! boost::filesystem::exists( this->path() ) )
 		return;

 	boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end

 	for ( boost::filesystem::directory_iterator itr( path ); itr != end_itr; ++itr ){
 		if ( boost::filesystem::is_directory( *itr ) ) {
 			MusicDir child = this->add_child( itr->leaf() );
 			child.sync();
 		} else if ( Song::is_interesting( *itr ) ){
			Song::create_from_file( *this, itr->leaf() );
 		}
 	}
}

