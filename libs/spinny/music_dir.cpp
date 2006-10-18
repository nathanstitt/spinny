

#include "music_dir.hpp"
#include "boost/filesystem/operations.hpp"
#include "song.hpp" 
#include <vector>
#include <limits>
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
	std::string where("parent_id=0 and name=");
	where += sqlite::q( path.string() );
	MusicDir::result_set mds = sqlite::db()->load_where<MusicDir>( where );
	if ( mds.begin() != mds.end() ){
		return mds.begin().shared_ptr();
	}
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
	return sqlite::db()->load<MusicDir>( db_id );
}


MusicDir::result_set
MusicDir::roots(){
	return sqlite::db()->load_many<MusicDir>( "parent_id", 0 );
}

bool
MusicDir::save() const {
	return sqlite::db()->save<MusicDir>(*this);
}


bool
MusicDir::is_root() const {
 	return ( _parent_id == 0 );
}


std::string
MusicDir::filesystem_name() const {
	return _name;
}


MusicDir::ptr
MusicDir::parent() const {
	return sqlite::db()->load<MusicDir>( _parent_id );
}

Song::result_set
MusicDir::songs(){
 	return sqlite::db()->load_many<Song>( "dir_id", db_id() );
}

MusicDir::result_set
MusicDir::children() const {
	return MusicDir::children_of( db_id() );
}

MusicDir::result_set
MusicDir::children_of( sqlite::id_t db_id ) {
	return sqlite::db()->load_many<MusicDir>( "parent_id", db_id );
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

typedef std::list<MusicDir::ptr> dirs_list_t;
typedef std::list<Song::ptr> songs_list_t;

template<typename T>
struct name_eq {
	name_eq( const std::string &s ) : str(s) { }
	std::string str;
	bool operator()( const typename T::value_type &s ){
		return s->filesystem_name() == str;
	}
};

template<typename T>
struct Konan_The_Destroyer : public unary_function<T, void> {
	void operator()( T &s ){
		s->destroy();
	}
};


void
MusicDir::sync( unsigned char depth ){
	boost::filesystem::path path = this->path();
	this->save_if_needed();

	if ( depth++ >= 255 ){ // a completely arbitrary limit
		return;
	}

 	if ( ! boost::filesystem::exists( this->path() ) )
 		return;

	result_set d_rs = this->children();
	dirs_list_t dirs;
	d_rs.copy_to<dirs_list_t>( dirs );

	Song::result_set s_rs = this->songs();
	songs_list_t songs;
	s_rs.copy_to<songs_list_t>( songs );

 	boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end


	// loop through each filesystem entry
 	for ( boost::filesystem::directory_iterator itr( this->path() ); itr != end_itr; ++itr ){

		// is directory?
 		if ( boost::filesystem::is_directory( *itr ) ) {

			// do we already know about the directory?
			dirs_list_t::iterator iter = std::find_if( dirs.begin(), dirs.end(), name_eq<dirs_list_t>( itr->leaf() ) );
			MusicDir::ptr child;
			// No, so add it.
			if ( dirs.end() == iter ){
				MusicDir::ptr new_dir = this->add_child( itr->leaf() ); 
				child.swap( new_dir );
				child->save();
			} else {
				child.swap( *iter );
				dirs.remove( *iter );

			}
			// recursively sync the directory, passing the depth, so we
			// don't somehow travel to INFINITY & BEYOND
			child->sync( depth );
 		} else if ( Song::is_interesting( *itr ) ){
			songs_list_t::iterator song = std::find_if( songs.begin(), songs.end(), name_eq<songs_list_t>( itr->leaf() ) );
			if ( songs.end() == song ){
				try {
					Song::create_from_file( *this, itr->leaf() );
				}
				catch( Song::file_error & ){	}
			} else {
				songs.remove( *song );
			}

 		}
 	}


	// now loop through, and delete unused entries
	std::for_each( dirs.begin(), dirs.end(), Konan_The_Destroyer<MusicDir::ptr>() );
	std::for_each( songs.begin(), songs.end(), Konan_The_Destroyer<Song::ptr>() );
}


void
MusicDir::destroy(){

	result_set d_rs = this->children();
	dirs_list_t dirs;
	d_rs.copy_to<dirs_list_t>( dirs );

	std::for_each( dirs.begin(), dirs.end(), Konan_The_Destroyer<MusicDir::ptr>() );

	Song::result_set s_rs = this->songs();
	songs_list_t songs;
	s_rs.copy_to<songs_list_t>( songs );
	std::for_each( songs.begin(), songs.end(), Konan_The_Destroyer<Song::ptr>() );

	sqlite::table::destroy();
}

