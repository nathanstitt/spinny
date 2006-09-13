

#include "music_dir.hpp"
#include "boost/filesystem/operations.hpp"
#include "song.hpp" 
#include <vector>

#include <list>
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



MusicDir::ptr
MusicDir::create_root( const boost::filesystem::path &path ){
	MusicDir::ptr md( new MusicDir );
 	md->_name=path.string();
 	md->_parent_id = 0;
	return md;
}

MusicDir::ptr
MusicDir::add_child( const std::string &name ){
	MusicDir::ptr md( new MusicDir );
	md->_name=name;
	md->_parent_id = this->db_id();
	return md;
}

MusicDir::ptr
MusicDir::load( sqlite::id_t db_id ){
	return Spinny::db()->load<MusicDir>( db_id );
}


MusicDir::result_set
MusicDir::roots(){
	return Spinny::db()->load_many<MusicDir>( "parent_id", 0 );
}

bool
MusicDir::save() const {
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



MusicDir::ptr
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
	const MusicDir *md = this;
	while ( ! md->is_root() ) {
		ptr p = md->parent();
		md=&(*p);
		dirs.push_back( md->_name );
	}
	boost::filesystem::path p( "", boost::filesystem::native );
	for ( vector<string>::reverse_iterator ri = dirs.rbegin(); ri != dirs.rend(); ++ri ){
		p /= *ri; 
	}

	return p;
}


bool
MusicDir::is_valid(){
	return boost::filesystem::exists( this->path() );
}

typedef std::list<boost::filesystem::path> list_t;

template<class T> struct
insert_path : public unary_function<T, void>
{
	insert_path( list_t &l ) : ii(l,l.begin()){ }
	void operator() (T &x) { *ii++=x.path(); }
	std::insert_iterator<list_t> ii;
};


void
MusicDir::sync(){
	boost::filesystem::path path = this->path();
	this->save_if_needed();


 	if ( ! boost::filesystem::exists( this->path() ) )
 		return;

	result_set rs = this->children();
	list_t dirs;
	std::for_each( rs.begin(), rs.end(), insert_path<MusicDir>(dirs) );
	

 	boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
 	for ( boost::filesystem::directory_iterator itr( this->path() ); itr != end_itr; ++itr ){
 		if ( boost::filesystem::is_directory( *itr ) ) {
			MusicDir::ptr child = this->add_child( itr->leaf() );
			child->save();
 			child->sync();
 		} else if ( Song::is_interesting( *itr ) ){
			try {
				Song::create_from_file( *this, itr->leaf() );
			}
			catch( Song::file_error & ){ }
 		}
 	}
}

