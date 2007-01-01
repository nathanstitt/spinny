/* @(#)request.cpp
 */


#include "network/request.hpp"
#include "network/server.hpp"
#include "boost/algorithm/string/case_conv.hpp"

namespace ews{

	void
	request::clear_current_header(){
		current_header_name.clear();
		current_header_value.clear();
	}

	request::request( const connection *c ) :
		conn( c ),
		http_version_major(0),
		http_version_minor(0),
		content_length(0)
	{ }

}
