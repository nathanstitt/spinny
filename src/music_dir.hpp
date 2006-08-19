/* @(#)music_dir.hpp
 */

#ifndef _MUSIC_DIR_H
#define _MUSIC_DIR_H 1

#include "spinny.hpp"
#include <boost/filesystem/operations.hpp>
#include <vector>

class MusicDir : public sqlite::table {
	GRANT_SQLITE_FRIENDSHIP;

	sqlite::id_t _id;
	string _name;	
	sqlite::id_t _parent_id;
	MusicDir();

	virtual void table_insert_values( std::ostream &str ) const;
	virtual void table_update_values( std::ostream &str ) const;
	void initialize_from_db( const sqlite::reader *reader );
 	static const sqlite::table::description* table_description();
 	virtual const description* m_table_description() const;
public:
	static MusicDir
	create_root( const boost::filesystem::path &path );

	static 
	std::vector<MusicDir>
	roots();

 	bool
 	is_root() const;

	bool save();

 	string name() const;

  	MusicDir
  	parent() const;


// 	boost::filesystem::path
// 	path() const;

// 	void
// 	sync();
};



#endif /* _MUSIC_DIR_H */

