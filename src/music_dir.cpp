

#include "music_dir.hpp"
#include <vector>

namespace sqlite{

 	template <> inline
 	MusicDir
	reader::get() const {
		MusicDir md;
		md._id=this->get<id_t>(0);
		md._name=this->get<string>(1);
		md._parent_id=this->get<id_t>(2);
		return md;
	}

	template <> inline
	void
	command::bind( const MusicDir &md ){
		this->bind( 0, md._id );
		this->bind( 1, md._name );
		this->bind( 2, md._parent_id );
	};
}

class md_desc : public sqlite::table_desc {
public:
	virtual const char* table_name() const {
		return "music_dir";
	};
	virtual int num_fields() const {
		return 3;
	}
	virtual const char** fields() const {
		static const char *fields[] = {
			"id",
			"parent_id",
			"name",
		};
		return fields;
	}
	virtual const char** field_types() const {
		static const char *field_types[] = {
			"int",
			"int",
			"string",
		};
		return field_types;
	};
};

static md_desc table_desc;

const sqlite::table_desc*
MusicDir::table_description(){
	return &table_desc;
}



MusicDir::MusicDir() {

}


MusicDir
MusicDir::find_by_id( sqlite::id_t id ) {
	return Spinny::db()->find_by_field<MusicDir>( "id", id );
}

MusicDir
MusicDir::parent() const {
	return Spinny::db()->find_by_field<MusicDir>( "parent_id", _parent_id );
}

void
MusicDir::set_as_root(){
	_parent_id=_id;
}

bool
MusicDir::is_root() const {
	return ( _id == _parent_id );
}

string
MusicDir::name() const {
	return _name; 
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
	boost::filesystem::path p("");
	for ( vector<string>::reverse_iterator ri = dirs.rbegin(); ri != dirs.rend(); ++ri ){
		p /= *ri;
	}
	return p;
}
