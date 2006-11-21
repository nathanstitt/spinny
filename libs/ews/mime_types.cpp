	#include "ews/mime_types.hpp"
#include "ews/server.hpp"

namespace ews {
	namespace mime_types {

		struct mapping
		{
			const char* extension;
			const char* mime_type;
		} mappings[] =
		{
			 { "css",  "text/css" },
			 { "html", "text/html" },
			 { "png",  "image/png" },
			 { "gif",  "image/gif" },
			 { "xml",  "text/xml" },
			 { "json", "application/json" },
			 { "jpg",  "image/jpeg" },
			 { 0, 0 }	// Marks end of list.
		};

		std::string extension_to_type(const std::string& extension) {
			for (mapping* m = mappings; m->extension; ++m) {
				if ( m->extension == extension) {
					return m->mime_type;
				}
			}
			return "text/plain";
		}

	} // namespace mime_types
} // namespace ews

