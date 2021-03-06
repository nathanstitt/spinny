/* @(#)index.hpp
 */

#ifndef _HANDLERS_INDEX_H
#define _HANDLERS_INDEX_H 1

#include "ews/server.hpp"
#include "ews/request_handler.hpp"


namespace handlers {

	class Index
		: public ews::request_handler {
	public:
		Index();

		virtual request_handler::RequestStatus
		handle( const ews::request& req, ews::reply& rep ) const;
	
	};

} // handlers



#endif /* _HANDLERS_INDEX_H */

