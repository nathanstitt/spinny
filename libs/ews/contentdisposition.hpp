
#ifndef CONTENTDISPOSITION_H
#define CONTENTDISPOSITION_H

#include <string>
#include <map>
#include "ews/server.hpp"

namespace ews {
/// This stores the content disposition of a subbody
/**
 * This stores the content disposition of a subbody.  This is used for
 *   multi-part form data.
 */
	class ContentDisposition {

	public:
		typedef std::map<std::string, std::string> string_map_t;

		/// constructor
		ContentDisposition ( ) { }

		/// destructor
		~ContentDisposition ( ) { }


		string_map_t headers;
		std::string disposition;

	};

} // ews

#endif // CONTENTDISPOSITION_H
