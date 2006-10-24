/* @(#)playlist.hpp
 */

#ifndef _PLAYLIST_H
#define _PLAYLIST_H 1

#include "spinny/spinny.hpp"
#include "spinny/song.hpp"

#include <vector>



class PlayList : public sqlite::table {
public:
	typedef boost::shared_ptr<PlayList> ptr;
	typedef ::sqlite::result_set<PlayList> result_set;
private:
	GRANT_NEEDED_FRIENDSHIP(PlayList);

	PlayList();

	virtual void table_insert_values( std::ostream &str ) const;
	virtual void table_update_values( std::ostream &str ) const;
	void initialize_from_db( const sqlite::reader *reader );
 	static const sqlite::table::description* table_description();
 	virtual const description* m_table_description() const;

        int bitrate_;
	string name_;

public:
	static ptr
	load( sqlite::id_t db_id );

	static result_set
	all();

	static ptr
	create( int bitrate, std::string name );

	std::string
	name() const;

	int
	bitrate() const;

	Song::result_set
	songs() const;

	void
	push_back( const Song &s );

	bool
	save() const;

	
};

#endif /* _PLAYLIST_H */

