#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <vector>
#include <map>

#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string/find.hpp"
#include "ews/request_parser.hpp"
#include "ews/header.hpp"
#include "cs/cs.h"

namespace ews {
	class connection;

	/// A request received from a client.
	class request {
		friend class request_parser;

		std::string current_header_name;
		std::string current_header_value;
		void clear_current_header();
	public:
		request( const connection *conn );

		const connection *conn;

		typedef std::map< std::string, std::string > headers_t;
		headers_t headers;

		typedef std::vector<std::string> varible_t;
		typedef std::map< std::string, varible_t > varibles_t;
		varibles_t varibles;

		int http_version_major;
		int http_version_minor;
		std::string method;
		std::string uri;
		std::string url;
		std::string body;
		unsigned int content_length;

		template<typename T>
		T
		get_header( const std::string& name ) const {
			headers_t::const_iterator hv=headers.find( name );
			if ( headers.end() != hv ){
				return boost::lexical_cast<T>( hv->second );
			} else {
				return T();
			}
		}

		template<typename T>
		T
		single_value( const std::string &name ) const {
			varibles_t::const_iterator var=varibles.find( name );
			varible_t::iterator val;
			if ( varibles.end() != var && ! var->second.empty() ){
				return boost::lexical_cast<T>( var->second.front() );
			} else {
				return T();
			}

		}
	};

} // namespace server

#endif // HTTP_REQUEST_HPP
