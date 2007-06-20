/* @(#)xspf.hpp
 */

#ifndef _HANDLERS_XSPF_H
#define _HANDLERS_XSPF_H 1

#include "handlers/shared.hpp"

namespace handlers {

	class Xspf
		: public ews::request_handler {
	public:
		Xspf();

		virtual request_handler::RequestStatus
		handle( const ews::request& req, ews::reply& rep ) const;
	};

} // handlers



#endif /* _HANDLERS_XSPF_H */

