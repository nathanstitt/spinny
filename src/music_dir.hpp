/* @(#)music_dir.hpp
 */

#ifndef _MUSIC_DIR_H
#define _MUSIC_DIR_H 1

#include "spinny.hpp"
#include <boost/filesystem/operations.hpp>

class MusicDir : public sqlite::table {
	friend class sqlite::reader;
	friend class sqlite::command;
	friend class sqlite::connection;

	sqlite::auto_id_t _id;
	string _name;	
	sqlite::auto_id_t _parent_id;

	static inline
	const char* fields(){
		return "id,parent_id,name";
	}

	static inline
	const char* table(){
		return "music_dir";
	}

public:
	string name(){ return _name; }
	MusicDir find_by_id( sqlite::auto_id_t id );
	MusicDir parent();
	bool is_root();

	boost::filesystem::path path();

	void sync();
};



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


#endif /* _MUSIC_DIR_H */

