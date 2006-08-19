

#include "music_dir.hpp"
#include <vector>
#include <algorithm>
#include <iterator>

class md_desc : public sqlite::table::description {
public:
	virtual const char* table_name() const {
		return "music_dir";
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
	str << _parent_id << ",'" << sqlite::q(_name) << "'";
}

void
MusicDir::table_update_values( std::ostream &str ) const {
	str << "parent_id=" << _parent_id << "," << "name='" << sqlite::q(_name) << "'";
}


void
MusicDir::initialize_from_db( const sqlite::reader *reader ) {
	_parent_id = reader->get<int>(0);
	_name	   = reader->get<std::string>(1);
}

MusicDir::MusicDir() {

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



std::vector<MusicDir>
MusicDir::roots(){
	boost::shared_ptr<sqlite::command> cmd = Spinny::db()->load_many<MusicDir>( "parent_id", 0 );
	sqlite::slurp< std::vector<MusicDir> > sp;
	std::for_each( cmd->begin(), cmd->end(), sp );
	return sp.container;
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





// boost::filesystem::path
// MusicDir::path() const {
// 	vector<string> dirs;
// 	dirs.push_back( _name );
// 	MusicDir md = *this;
// 	while ( ! md.is_root() ) {
// 		md=md.parent();
// 		dirs.push_back( md._name );
// 	}
// 	boost::filesystem::path p("");
// 	for ( vector<string>::reverse_iterator ri = dirs.rbegin(); ri != dirs.rend(); ++ri ){
// 		p /= *ri;
// 	}
// 	return p;
// }
