
/* @(#)exceptions.hpp
 */

#ifndef _EWS_EXCEPTIONS_H
#define _EWS_EXCEPTIONS_H 1

#include <stdexcept>

namespace ews {

	class error : public std::runtime_error {
	public:
		error( const char *msg );
	};

	class not_found_error : public error {
	public:
		not_found_error( const char *msg );
	};

	class forbidden_error : public error {
	public:
		forbidden_error( const char *msg );
	};



} // ews namespace


#endif /* _EWS_EXCEPTIONS_H */

