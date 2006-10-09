
#ifndef FORMVALUE_H
#define FORMVALUE_H

#include <string>

#include "ews/server.hpp"
#include "ews/contentdisposition.hpp"

namespace ews {

/// This is how data is passed in from the client has to be more complicated because of metadata associated with MIME-style attachments
/**
 * This is how data is passed in from the client has to be more complicated because of metadata 
 *   associated with MIME-style attachments.  Each element of a form is put into a ContentDisposition
 *   object which can be looked at in HandleRequest to see what data might have been sent in.
 */

	class FormValue {
	
	public:
		typedef std::map < std::string, FormValue > map;
		typedef std::map < std::string, std::string > string_map_t;

		/// for MIME attachments only, normal header information like content-type
		// -- everything except content-disposition, which is in oContentDisposition
		string_map_t headers;

		/// everything in the content disposition line
		ContentDisposition content_disposition; 
	
		/// the body of the value.  For non-MIME-style attachments, this is the only part used.
		std::string body; 
	
		/// Default constructor
		FormValue ( ) { }
	
		/// Constructor 
		FormValue ( std::string &b, ContentDisposition &cd ) :
			content_disposition ( cd ),
			body ( b ) 
			{ }


		/// destructor
		virtual ~FormValue ( ) { }
   
	};

} // ews 

#endif // FORMVALUE_H
