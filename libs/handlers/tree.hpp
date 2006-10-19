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

		virtual request_handler::result
		handle( const ews::request& req, ews::reply& rep );
		std::string name() const;
	};

} // handlers



#endif /* _HANDLERS_TREE_H */

