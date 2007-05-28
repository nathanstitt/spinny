/* @(#)playlist.hpp
 */

#ifndef _SPINNY_PLAYLIST_H
#define _SPINNY_PLAYLIST_H 1

#include "sqlite/sqlite.hpp"
#include "spinny/config.h"
#include "boost/enable_shared_from_this.hpp"
#include "spinny/artist.hpp"
#include "spinny/album.hpp"
#include "spinny/song.hpp"
#include "boost/shared_ptr.hpp"

namespace Spinny {
	class Artist;
	class Album;
	class CompletePlayList;

class PlayList : public sqlite::table,
		 public boost::enable_shared_from_this<PlayList>
{
public:
	typedef boost::shared_ptr<PlayList> ptr;
	typedef ::sqlite::result_set<PlayList> result_set;
private:
	GRANT_NEEDED_FRIENDSHIP(PlayList);
	friend class CompletePlayList;
	PlayList();

	virtual void table_insert_values( std::ostream &str ) const;
	virtual void table_update_values( std::ostream &str ) const;
	void initialize_from_db( const sqlite::reader *reader );
 	virtual const description* m_table_description() const;

	string name_;
	string description_;
	string order_by_;
	string limit_;
	int present_order_;
public:
 	static const sqlite::table::description* table_description();

	static ptr
	load( sqlite::id_t db_id );

	static result_set
	all();

	static ptr
	create( const std::string &name="New Playlist", const std::string &desc="" );

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
	present_order() const;

	int
	set_present_order( int br );

	void
	virtual
	set_order( std::string order, bool descending=true );

	Song::result_set
	virtual
	songs( unsigned int start=0, unsigned int limit=0 ) const;

	void
	insert( Spinny::Song::result_set songs, int position );

	void
	insert( Song::ptr s, int position );

	void
	insert( PlayList::ptr pl, int position );

	void
	insert( Spinny::MusicDir::ptr md, int position );

	void
	insert( Spinny::Artist::ptr artist, int position );

	void
	insert( boost::shared_ptr<Album> album, int position );

	bool
	save() const;

	unsigned int
	virtual size() const;

	static
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

