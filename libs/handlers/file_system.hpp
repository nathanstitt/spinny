#ifndef _HANDLERS_FILESYSTEM_HPP_
#define _HANDLERS_FILESYSTEM_HPP_ 1

#include "ews/request_handler.hpp"

namespace handlers {

class Files
	: public ews::request_handler {
public:
	explicit Files();

	virtual ews::request_handler::RequestStatus
	handle(const ews::request& req, ews::reply& rep ) const;


};


} // namespace handlers

#endif // _HANDLERS_FILESYSTEM_HPP_
