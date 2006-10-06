#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <vector>
#include "header.hpp"


namespace ews {

/// A request received from a client.
struct request
{

// 	typedef std::list<std::string> varible_t;
// 	typedef std::map< std::string, varible_t > varibles_t;

	std::string method;
	std::string uri;
	std::string url;
	int http_version_major;
	int http_version_minor;
	std::vector<header> headers;

//	varibles_t varibles;

};

} // namespace server

#endif // HTTP_REQUEST_HPP
