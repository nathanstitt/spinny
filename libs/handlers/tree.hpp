/* @(#)tree.hpp
 */

#ifndef _HANDLERS_TREE_H
#define _HANDLERS_TREE_H 1

#include "ews/server.hpp"
#include "ews/request_handler.hpp"


namespace handlers {

	class Tree
		: public ews::request_handler {
	public:
		Tree();

		virtual request_handler::RequestStatus
		handle( const ews::request& req, ews::reply& rep ) const;
	};

} // handlers



#endif /* _HANDLERS_TREE_H */

