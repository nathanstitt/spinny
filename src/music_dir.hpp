/* @(#)music_dir.hpp
 */

#ifndef _MUSIC_DIR_H
#define _MUSIC_DIR_H 1

#include "spinny.hpp"
#include <boost/filesystem/operations.hpp>


class MusicDir : public sqlite::table {
	GRANT_SQLITE_FRIENDSHIP;

	sqlite::id_t _id;
	string _name;	
	sqlite::id_t _parent_id;
public:
	MusicDir();

	static
	const sqlite::table_desc*
	table_description();

	string
	name() const;

	static MusicDir 
	find_by_id( sqlite::id_t id );

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

