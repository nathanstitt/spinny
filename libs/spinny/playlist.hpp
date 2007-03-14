/* @(#)playlist.hpp
 */

#ifndef _SPINNY_PLAYLIST_H
#define _SPINNY_PLAYLIST_H 1

#include "sqlite/sqlite.hpp"
#include "spinny/config.h"
#include "spinny/song.hpp"
#include "boost/enable_shared_from_this.hpp"

#include <vector>

namespace Spinny {

class PlayList : public sqlite::table,
		 public boost::enable_shared_from_this<PlayList>
{
public:
	typedef boost::shared_ptr<PlayList> ptr;
	typedef ::sqlite::result_set<PlayList> result_set;
private:
	GRANT_NEEDED_FRIENDSHIP(PlayList);

	PlayList();

	virtual void table_insert_values( std::ostream &str ) const;
	virtual void table_update_values( std::ostream &str ) const;
	void initialize_from_db( const sqlite::reader *reader );
 	virtual const description* m_table_description() const;

        int bitrate_;
	string name_;
	string description_;
	int present_order_;
public:
 	static const sqlite::table::description* table_description();

	static ptr
	load( sqlite::id_t db_id );

	static result_set
	all();

	static ptr
	create( int bitrate, const std::string &name, const std::string &desc );

	static void
	set_order( sqlite::id_t db_id, int order );

	void
	set_song_order( sqlite::id_t db_id, int order );

	std::string
	name() const;

	std::string
	set_name( const std::string& name );

	std::string
	description() const;

	std::string
	set_description( const std::string& desc );

	int
	bitrate() const;

	int
	set_bitrate( int br );

	int
	present_order() const;

	int
	set_present_order( int br );

	Song::result_set
	songs() const;

	void
	insert( const Song::ptr s, int position );

	void
	insert( const PlayList::ptr pl, int position );

	bool
	save() const;

	unsigned int
	size() const;

	Song::ptr
	load_song( sqlite::id_t db_id );


	Song::ptr
	at( unsigned int pos );

	void
	clear();

	void
	remove( sqlite::id_t song_id );
};

} // namespace Spinny

#endif /* _SPINNY_PLAYLIST_H */

