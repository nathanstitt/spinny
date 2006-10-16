#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include <string>
#include "boost/noncopyable.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/logic/tribool.hpp"

namespace ews {

class reply;
class request;

/// The common handler for all incoming requests.
class request_handler
	: private boost::noncopyable
{
public:
	static bool handle_request( const request& req, reply &rep );

	/// Construct with a directory containing files to be served.
	explicit request_handler( bool should_register=true );

	enum result {
		cont,    // All ok, continue passing the request to other handlers
		stop,    // All ok, but don't continue passing the request to other handlers
		error,   // Error occured, stop processing & set status to 500
		file,    // Handle request with the filesystem handler
	};

	/// Handle a request and produce a reply.
	virtual result handle( const request &req, reply &rep ) = 0;

	virtual std::string name() const = 0;

	virtual ~request_handler();
};


} // namespace ews

#endif // HTTP_REQUEST_HANDLER_HPP
