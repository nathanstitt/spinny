
#ifndef CONTENTDISPOSITION_H
#define CONTENTDISPOSITION_H

#include <string>
#include "ews/server.hpp"


namespace ews {
/// This stores the content disposition of a subbody
/**
 * This stores the content disposition of a subbody.  This is used for
 *   multi-part form data.
 */
	class ContentDisposition {

	public:
	
		/// constructor
		ContentDisposition ( ) {
#ifdef EHS_MEMORY
			fprintf ( stderr, "[EHS_MEMORY] Allocated: ContentDisposition\n" );
#endif		
		}

		/// destructor
		~ContentDisposition ( ) {
#ifdef EHS_MEMORY
			fprintf ( stderr, "[EHS_MEMORY] Deallocated: ContentDisposition\n" );
#endif		
		}

#ifdef EHS_MEMORY
		/// this is only for watching memory allocation
		ContentDisposition ( const ContentDisposition & iroContentDisposition ) {
			*this = iroContentDisposition;
			fprintf ( stderr, "[EHS_MEMORY] Allocated: ContentDisposition (Copy Constructor)\n" );
		}
#endif		

		ews::string_map_t oContentDispositionHeaders; ///< map of headers for this content disposition
		std::string sContentDisposition; ///< content disposition string for this object

	};

} // ews

#endif // CONTENTDISPOSITION_H
