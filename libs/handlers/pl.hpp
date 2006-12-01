/* @(#)pl.hpp
 */

#ifndef _HANDLERS_PL_H
#define _HANDLERS_PL_H 1


#include "handlers/shared.hpp"

namespace handlers {

	class PL
		: public ews::request_handler {
	public:
		PL();

		virtual request_handler::RequestStatus
		handle( const ews::request& req, ews::reply& rep ) const;

	
	};

} // handlers



#endif /* _HANDLERS_PL_H */

