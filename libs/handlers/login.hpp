/* @(#)login.hpp
 */

#ifndef _HANDLERS_LOGIN_H
#define _HANDLERS_LOGIN_H 1

#include "handlers/shared.hpp"

namespace handlers {

	class Login
		: public ews::request_handler {
	public:
		Login();

		virtual request_handler::RequestStatus
		handle( const ews::request& req, ews::reply& rep ) const;
		std::string name() const;
	
	};

} // handlers



#endif /* _HANDLERS_LOGIN_H */

