

/* @(#)shared.hpp
 */

#ifndef _HANDLERS_INSTANTIATE_H
#define _HANDLERS_INSTANTIATE_H 1


#include "ews/server.hpp"


namespace handlers {
	
	void link_up();

	enum JsonReqType {
		JsonDir		     = 'a',
		JsonArtistsFirstChar = 'b', 
		JsonArtistsAlbum     = 'd',
		JsonAlbumsFirstChar  = 'e',
		JsonAlbum	     = 'f',
		JsonArtistsSongs     = 'g',
		JsonAlbumsSongs	     = 'h',
		JsonDirSongs	     = 'i',
		JsonSong	     = 's',
		JsonUnknown	     = 'z' 
	};

	void
	requireAuth( Spinny::User::ptr, Spinny::User::Role );

	std::string
	json_q( const std::string& );

} // handlers

#endif /* _HANDLERS_INSTANTIATE_H */

