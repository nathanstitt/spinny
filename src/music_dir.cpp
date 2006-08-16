

#include "music_dir.hpp"
#include <vector>

namespace sqlite{

 	template <> inline
 	MusicDir
	reader::get(){
		MusicDir md;
		md._id=this->get<auto_id_t>(0);
		md._name=this->get<string>(1);
		md._parent_id=this->get<auto_id_t>(2);
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

const char*
MusicDir::fields(){
	return "id,parent_id,name";
}

	
const char*
MusicDir::table(){
	return "music_dir";
}


MusicDir
MusicDir::find_by_id( sqlite::auto_id_t id ){
	return Spinny::db()->find_by_field<MusicDir>( "id", _id );
}

MusicDir
MusicDir::parent(){
	return Spinny::db()->find_by_field<MusicDir>( "parent_id", _parent_id );
}


bool
MusicDir::is_root(){
	return ( _id == _parent_id );
}

string
MusicDir::name(){
	return _name; 
}

boost::filesystem::path
MusicDir::path(){
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
