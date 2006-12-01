#ifndef HTTP_FILES_REQUEST_HANDLER_HPP
#define HTTP_FILES_REQUEST_HANDLER_HPP

#include <string>
#include "ews/request_handler.hpp"
#include "boost/filesystem/path.hpp"

namespace ews {

class reply;
class request;

/// The common handler for all incoming requests.
class files_request_handler 
	: public request_handler
{
public:


	/// Construct with a directory containing files to be served.
	explicit files_request_handler();

	/// Handle a request and produce a reply.
	virtual request_handler::RequestStatus
	handle(const request& req, reply& rep ) const;

	virtual std::string name() const;

	virtual ~files_request_handler();
private:

};


} // namespace ews

#endif // HTTP_FIELS_REQUEST_HANDLER_HPP
