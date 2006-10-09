/* @(#)request.cpp
 */


#include "ews/request.hpp"
#include "ews/server.hpp"
#include "boost/algorithm/string/case_conv.hpp"

namespace ews{

	void
	request::clear_current_header(){
		current_header_name.clear();
		current_header_value.clear();
	}
}
