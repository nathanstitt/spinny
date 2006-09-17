/* @(#)music_dir.hpp
 */

#ifndef _MUSIC_DIR_H
#define _MUSIC_DIR_H 1

#include "spinny.hpp"

#include "boost/filesystem/path.hpp"
#include <vector>

class Song;

class MusicDir : public sqlite::table {
public:
	typedef boost::shared_ptr<MusicDir> ptr;
	typedef ::sqlite::result_set<MusicDir> result_set;
	typedef ::sqlite::result_set<MusicDir>::iterator iterator;
private:
	GRANT_NEEDED_FRIENDSHIP(MusicDir);
	friend class Song;

	sqlite::id_t _parent_id;
	string _name;	
	MusicDir();

	virtual void table_insert_values( std::ostream &str ) const;
	virtual void table_update_values( std::ostream &str ) const;
	void initialize_from_db( const sqlite::reader *reader );
 	static const sqlite::table::description* table_description();
 	virtual const description* m_table_description() const;

	MusicDir::ptr add_child( const std::string &name );

public:

	static MusicDir::ptr
	load( sqlite::id_t db_id );

	static MusicDir::ptr
	create_root( const boost::filesystem::path &path );

	static 
	result_set
	roots();

	sqlite::result_set<Song>
	songs();

 	bool
 	is_root() const;

	bool save() const;

 	string filesystem_name() const;

	MusicDir::ptr
  	parent() const;

	result_set
	children() const;

  	boost::filesystem::path
  	path() const;

	bool
	is_valid();

 	void
 	sync( unsigned char depth = 0 );

	virtual void destroy();
};









#endif /* _MUSIC_DIR_H */

