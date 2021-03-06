#ifndef HTTP_MIME_TYPES_HPP
#define HTTP_MIME_TYPES_HPP

#include <string>

namespace ews {
	namespace mime_types {

		/// Convert a file extension into a MIME type.
		std::string extension_to_type(const std::string& extension);

	} // namespace mime_types
} // namespace ews


#endif // HTTP_MIME_TYPES_HPP
