/* @(#)pl.hpp
 */

#ifndef _PL_H
#define _PL_H 1


#include "handlers/shared.hpp"

namespace handlers {

	class PL
		: public ews::request_handler {
	public:

		virtual request_handler::result
		handle( const ews::request& req, ews::reply& rep ) const;
		std::string name() const;
	
	};

} // handlers



#endif /* _PL_H */

