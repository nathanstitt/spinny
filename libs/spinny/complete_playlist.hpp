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
public:
	typedef boost::shared_ptr<CompletePlayList> ptr;

	static
	ptr
	instance();

	Song::result_set
	virtual songs( std::string order="", unsigned int start=0, unsigned int limit=0 ) const;

	unsigned int
	virtual size() const;

};


} // namespace Spinny


#endif //_SPINNY_COMPLETE_PLAYLIST_H_
