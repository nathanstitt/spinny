/* @(#)info.hpp
 */

#ifndef _HANDLERS_INFO_H
#define _HANDLERS_INFO_H 1

#include "ews/server.hpp"
#include "ews/request_handler.hpp"


namespace handlers {

	class Info
		: public ews::request_handler {
	public:
		Info();

		virtual request_handler::RequestStatus
		handle( const ews::request& req, ews::reply& rep ) const;
	};

} // handlers



#endif /* _HANDLERS_INFO_H */

