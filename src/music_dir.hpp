/* @(#)music_dir.hpp
 */

#ifndef _MUSIC_DIR_H
#define _MUSIC_DIR_H 1

#include "spinny.hpp"
#include <boost/filesystem/operations.hpp>


class MusicDir : public sqlite::table {
	SQLITE_GRANT_FRIENDSHIP;

	sqlite::auto_id_t _id;
	string _name;	
	sqlite::auto_id_t _parent_id;
public:
	static const sqlite::table_desc* description();

	MusicDir();

	string
	name() const;

	static MusicDir 
	find_by_id( sqlite::auto_id_t id );

	MusicDir
	parent() const;

	void
	set_as_root();

	bool
	is_root() const;

	boost::filesystem::path
	path() const;

	void
	sync();
};



#endif /* _MUSIC_DIR_H */

