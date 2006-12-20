/* @(#)music_dir.hpp
 */

#ifndef _MUSIC_DIR_H
#define _MUSIC_DIR_H 1

#include "spinny/spinny.hpp"
#include "boost/filesystem/path.hpp"
#include <vector>

namespace Spinny {

class Song;

class MusicDir : public sqlite::table {
public:
	typedef boost::shared_ptr<MusicDir> ptr;
	typedef ::sqlite::result_set<MusicDir> result_set;
	typedef ::sqlite::result_set<MusicDir>::iterator iterator;
private:
	sqlite::id_t num_children_;
	sqlite::id_t num_songs_;
	bool loaded_counts_;
	GRANT_NEEDED_FRIENDSHIP(MusicDir);
	friend class Song;

	sqlite::id_t parent_id_;
	string name_;
	MusicDir();

	virtual void table_insert_values( std::ostream &str ) const;
	virtual void table_update_values( std::ostream &str ) const;
	void initialize_from_db( const sqlite::reader *reader );
 	virtual const description* m_table_description() const;

	MusicDir::ptr add_child( const std::string &name );

public:
 	static const sqlite::table::description* table_description();

	static MusicDir::ptr
	load( sqlite::id_t db_id );

	static MusicDir::ptr
	create_root( const boost::filesystem::path &path );

	static 
	result_set
	roots();

	static
	result_set
	children_of( sqlite::id_t db_id );

	sqlite::result_set<Song>
	songs();

 	bool
 	is_root() const;

	bool
	save() const;

	std::string
	filesystem_name() const;

	MusicDir::ptr
  	parent() const;

	result_set
	children() const;

  	boost::filesystem::path
  	path() const;

	bool
	is_valid();

	sqlite::id_t
	num_children();

	sqlite::id_t
	num_songs();

 	void
 	sync( unsigned char depth = 0 );

	virtual void destroy();

};



} // namespace Spinny





#endif /* _MUSIC_DIR_H */

