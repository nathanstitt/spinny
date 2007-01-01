
#include "ews/exceptions.hpp"

ews::error::error( const char *msg ) : runtime_error(msg){}


ews::not_found_error::not_found_error( const char *msg ) : error(msg){}
