

#include "music_dir.hpp"
#include <vector>

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
