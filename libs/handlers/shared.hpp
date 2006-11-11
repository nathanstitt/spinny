

/* @(#)shared.hpp
 */

#ifndef _HANDLERS_INSTANTIATE_H
#define _HANDLERS_INSTANTIATE_H 1


#include "ews/request_handler.hpp"



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


} // handlers

#endif /* _HANDLERS_INSTANTIATE_H */

