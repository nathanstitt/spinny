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
	const char* fields();

	static inline
	const char* table();
public:
	string name();

	MusicDir find_by_id( sqlite::auto_id_t id );

	MusicDir parent();

	bool is_root();

	boost::filesystem::path path();

	void sync();
};




#endif /* _MUSIC_DIR_H */

