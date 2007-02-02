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

	enum RequestStatus {
		Continue,      // All ok, continue passing the request to other handlers
		Stop,          // All ok, but don't continue passing the request to other handlers
	};
	enum RequestPhase {
		Beginning,
		Middle,
		End,
		Last
	};

	/// Construct with a directory containing files to be served.
	explicit request_handler( const char *name, RequestPhase phase );


	/// Handle a request and produce a reply.
	virtual RequestStatus handle( const request &req, reply &rep ) const = 0;

	const char* name() const;
	RequestPhase phase() const;

	virtual ~request_handler();
private:
	const char *name_;
	RequestPhase phase_;
};



} // namespace ews

#endif // HTTP_REQUEST_HANDLER_HPP
