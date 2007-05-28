/* @(#)complete_playlist.hpp
 */

#ifndef _SPINNY_COMPLETE_PLAYLIST_H_
#define _SPINNY_COMPLETE_PLAYLIST_H_ 1

#include "sqlite/sqlite.hpp"
#include "spinny/config.h"
#include "spinny/playlist.hpp"


namespace Spinny {
	class Artist;
	class Album;

class CompletePlayList : public PlayList
{
	std::string order_;
public:
	typedef boost::shared_ptr<CompletePlayList> ptr;

	static
	ptr
	instance();

	void
	virtual
	set_order( std::string order, bool descending=true );

	Song::result_set
	virtual songs( unsigned int start=0, unsigned int limit=0 ) const;

	unsigned int
	virtual size() const;

	Song::ptr
	at( unsigned int pos );



	void
	set_song_order( sqlite::id_t db_id, int order );

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

	void
	clear();

	void
	remove( sqlite::id_t song_id );

	
	//as_zip();

};


} // namespace Spinny


#endif //_SPINNY_COMPLETE_PLAYLIST_H_
