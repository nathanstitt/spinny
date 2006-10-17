/* @(#)index.hpp
 */

#ifndef _INDEX_H
#define _INDEX_H 1

#include "ews/server.hpp"
#include "ews/request_handler.hpp"

namespace ews {

	namespace handlers {

		class Index
			: public ews::request_handler {
		public:
			Index();
			virtual request_handler::result
			handle( const ews::request& req, ews::reply& rep );
			std::string name() const;
	
		};

	} // handlers

} // ews

#endif /* _INDEX_H */
