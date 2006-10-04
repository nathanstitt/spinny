#include "ews/header.hpp"


ews::header::header(  const std::string &n, const std::string& v ) :
	name(n),
	value(v)
{ }


ews::header::header()
{ }
