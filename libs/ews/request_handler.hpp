#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include <string>
#include "boost/noncopyable.hpp"
#include "boost/filesystem/path.hpp"


namespace ews {

struct reply;
struct request;

/// The common handler for all incoming requests.
class request_handler
	: private boost::noncopyable
{
public:
	static request_handler* find_handler( const request& req );

	/// Construct with a directory containing files to be served.
	explicit request_handler( bool should_register=true );

	/// Handle a request and produce a reply.
	virtual bool handle_request(const request& req, reply& rep, const boost::filesystem::path &doc_root ) = 0;

	virtual bool can_handle( const request& req ) = 0;
	
	virtual ~request_handler();
};


} // namespace ews

#endif // HTTP_REQUEST_HANDLER_HPP
