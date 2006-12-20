/* @(#)users.hpp
 */

#ifndef _HANDLERS_USERS_H
#define _HANDLERS_USERS_H 1

#include "handlers/shared.hpp"

namespace handlers {

	class Users
		: public ews::request_handler {
	public:
		Users();

		virtual request_handler::RequestStatus
		handle( const ews::request& req, ews::reply& rep ) const;

	
	};

} // handlers



#endif /* _HANDLERS_USERS_H */

