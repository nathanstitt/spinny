
#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H


#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <map>

//#include <pme.h>
//#include "ehs.h"
//#include "ehstypes.h"
#include "ews/formvalue.hpp"

namespace ews {

	class Request {

	public:

		typedef std::map < std::string, std::string > cookie_map_t;

		/// constructor
		Request( connection *conn );


		/// destructor
		virtual ~Request ( );

		/// parse out a request
		bool parse_request( std::string &data );

		/// Enumeration for the state of the current HTTP parsing
		enum HttpParseStates { HTTPPARSESTATE_INVALID = 0,
				       HTTPPARSESTATE_REQUEST,
				       HTTPPARSESTATE_HEADERS,
				       HTTPPARSESTATE_BODY,
				       HTTPPARSESTATE_COMPLETEREQUEST,
				       HTTPPARSESTATE_INVALIDREQUEST,
		};

		/// Enumeration of error codes for ParseSubbody
		enum ParseSubbodyResult {
			PARSESUBBODY_INVALID = 0,
			PARSESUBBODY_SUCCESS,
			PARSESUBBODY_INVALIDSUBBODY, // no blank line?
			PARSESUBBODY_FAILED // other reason

		};

		/// UNKNOWN must be the last one, as these must match up with RequestMethodStrings *exactly*
		enum Method { REQUESTMETHOD_OPTIONS, /* not implemented */
			      REQUESTMETHOD_GET,
			      REQUESTMETHOD_HEAD,
			      REQUESTMETHOD_POST,
			      REQUESTMETHOD_PUT, ///< not implemented
			      REQUESTMETHOD_DELETE, ///< not implemented
			      REQUESTMETHOD_TRACE, ///< not implemented
			      REQUESTMETHOD_CONNECT, ///< not implemented
			      REQUESTMETHOD_LAST, ///< must be the last valid entry
			      REQUESTMETHOD_UNKNOWN, ///< used until we find the method
			      REQUESTMETHOD_INVALID ///< must be the last entry
		};


		/// this is the request method from the client
		Method method;
	
		/// the clients requested URI
		std::string uri;

		/// the HTTP version of the request
		std::string http_version;

		/// headers from the client request
		string_map_t request_headers;

		/// Data specified by the client.  The 'name' field is mapped to a FormValue object which has the value and any metadata
		FormValue::Map form_values;

		/// cookies that come in from the client
		cookie_map_t cookies;

		/// returns the address this request came from
		std::string remote_address();
	
		/// returns the port this request came from
		int remote_port();

	private:

		/// treats the body as if it's a multipart form data
		bool parse_multipart_form_data();

		/// Body data, not NULL terminated
		std::string body_;

		/// goes through a subbody and parses out elements
		ParseSubbodyResult parse_sub_body ( std::string sub_body );

		/// takes the cookie header and breaks it down into usable chunks -- returns number of name/value pairs found
		int parse_cookies( const std::string &data );

		/// interprets the given string as if it's name=value pairs and puts them into oFormElements
		void parse_form_elements( const std::string &element );

		/// connection object from which this request came
		connection * connection_;


	};




} // ews

#endif // HTTPREQUEST_H
