/* @(#)song.hpp
 */

#ifndef _HANDLERS_SONG_H
#define _HANDLERS_SONG_H 1

#include "handlers/shared.hpp"

namespace handlers {

	class Song
		: public ews::request_handler {
	public:
		Song();

		virtual request_handler::RequestStatus
		handle( const ews::request& req, ews::reply& rep ) const;
	};

} // handlers



#endif /* _HANDLERS_SONG_H */

