
#include "ews/httprequest.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <iterator>
#include <functional>
#include "boost/regex.hpp"
#include "boost/log/log.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/find_iterator.hpp"
#include "boost/algorithm/string/case_conv.hpp"

#include <assert.h>



using namespace std;
namespace ews {


	/// gets the RequestMethod enumeration based on isRequestMethod
	Request::Method GetRequestMethodFromString ( const std::string & isRequestMethod );

	void Request::parse_form_elements( const std::string &str ) {
		static const boost::regex regex("[?]?([^?=]*)=([^&]*)&?", boost::regex_constants::perl );
		boost::smatch what;

		boost::sregex_iterator match( str.begin(), str.end(), regex );
		boost::sregex_iterator end;

		while ( match != end ){
			ContentDisposition oContentDisposition;
			std::string name = (*match)[ 1 ];
			std::string value = (*match)[ 2 ];

			BOOST_LOGL( ewslog, info ) << "Got param data: " << name << " => " << value;

			form_values[ name ] =
				FormValue ( value, oContentDisposition );

			*match++;
		}

	}






// this parses a single piece of a multipart form body
// here are two examples -- first is an uploaded file, second is a
//   standard text box html form
/*
  -----------------------------7d2248207500d4
  Content-Disposition: form-data; name="DFWFilename2"; filename="C:\Documents and Settings\administrator\Desktop\contacts.dat"
  Content-Type: application/octet-stream

  Default Screen Name
  -----------------------------7d2248207500d4


  -----------------------------7d2248207500d4
  Content-Disposition: form-data; name="foo"

  asdfasdf
  -----------------------------7d2248207500d4
*/


	Request::ParseSubbodyResult
	Request::parse_sub_body ( std::string sub_body ) {
		boost::smatch what;

		// find the spot after the headers in the body
		std::string::size_type blank_line_pos = sub_body.find ( "\r\n\r\n" );
	
		// if there's no double blank line, then this isn't a valid subbody
		if ( blank_line_pos == std::string::npos ) {
			BOOST_LOGL(ewslog, err ) << "[EHS_DEUBG] Invalix sub body, couldn't find double blank line";
			return PARSESUBBODY_INVALIDSUBBODY;
		}

		// create a string from the beginning to the blank line -- OFF BY ONE?
		std::string headers ( sub_body, 0, blank_line_pos - 1 );


		// first line MUST be the content-disposition header line, so that
		//   we know what the name of the field is.. otherwise, we're in trouble
		static const boost::regex content_disposition_regex ( "Content-Disposition:[ ]?([^;]+);[ ]?(.*)",boost::regex_constants::perl );

		if ( boost::regex_match( headers, what, content_disposition_regex ) ){

			std::string content_disposition ( what[ 0 ] );
			std::string name_value_pairs ( what[ 1 ] );
			
			BOOST_LOGL( ewslog, info ) << "cont. disp: " << content_disposition << ", name/value pairs: " << name_value_pairs;

			string_map_t string_map;

			static const boost::regex name_value_pairs_regex("[ ]?([^= ]+)=\"([^\"]+)\"[;]?",boost::regex_constants::perl);
			boost::sregex_iterator match( name_value_pairs.begin(), name_value_pairs.end(), name_value_pairs_regex );
			boost::sregex_iterator end;

			while ( match != end ){
				std::string name  = (*match)[0];
				std::string value = (*match)[1];

				BOOST_LOGL( ewslog, info ) << "Header found: " << name << " => " << value;
				
				string_map[ name ] = value;
				*match++;
			}

			// take oStringMap and actually fill the right object with its data
			FormValue & roFormValue = form_values[ string_map[ "name" ] ];
		
			// copy the headers in now that we know the name
			roFormValue.oContentDisposition.oContentDispositionHeaders = string_map;
		
			// grab out the body
			roFormValue.sBody = sub_body.substr ( blank_line_pos + 4 );
		} else {
			// couldn't find content-disposition line -- FATAL ERROR
			BOOST_LOGL( ewslog, err ) << "Couldn't find content-disposition line";
			return PARSESUBBODY_INVALIDSUBBODY;
		
		}
		return PARSESUBBODY_SUCCESS;
	}





	bool
	Request::parse_multipart_form_data(){
	
		//assert( request_headers.count( "content-type" ] );

		boost::smatch what;
		// find the boundary string
		int nMatchResult = 0;

		static const boost::regex content_type_regex( "multipart/[^;]+;[ ]*boundary=([^\"]+)$",boost::regex_constants::perl );
		BOOST_LOGL( ewslog, info ) << "Looking for boundary in " << request_headers[ "content-type" ];

		
		if ( boost::regex_match( request_headers [ "content-type" ], what, content_type_regex ) ){

			std::string boundary_string = what[0];

			// the actual boundary has two dashes prepended to it
			std::string actual_boundary = std::string ("--") + boundary_string;

			BOOST_LOGL( ewslog, info ) << "Found boundary of " << boundary_string;
			BOOST_LOGL( ewslog, info ) << "Looking for boundary to match (" << body_.length()
						   << ") " 
						   << body_.substr ( 0, actual_boundary.length ( ) ).c_str ( );

			// check to make sure we started at the boundary
			if ( body_.substr ( 0, actual_boundary.length ( ) ) != actual_boundary ) {
				BOOST_LOGL( ewslog, err ) << "Misparsed multi-part form data for unknown reason - first bytes weren't the boundary string";
				return false;
			}

			// go past the initial boundary
			std::string remaining_body = body_.substr ( actual_boundary.length() );

			// while we're at a boundary after we grab a part, keep going
			std::string::size_type next_part_position;

			while ( ( next_part_position = remaining_body.find( std::string( "\r\n" ) + actual_boundary )
					) != std::string::npos ) 
			{
				BOOST_LOGL( ewslog, info ) << "Found subbody from pos 0 to " << next_part_position;

				//assert( (unsigned int) remaining_body.length ( ) >= actual_boundary.length ( ) );
			
				parse_sub_body ( remaining_body.substr ( 0, next_part_position ) );
			
				// skip past the boundary at the end and look for the next one
				next_part_position += actual_boundary.length ( );

				remaining_body = remaining_body.substr ( next_part_position );

			}
			
		} else {
			BOOST_LOGL( ewslog, err ) << "Couldn't find boundary specification in content-type header";
			return false;
		}
		return true;
	}

	// A cookie header looks like: Cookie: username=xaxxon; password=comoesta
	// everything after the : gets passed in in irsData
	int
	Request::parse_cookies( const std::string &data ) {

		BOOST_LOGL( ewslog, info ) << "looking for cookies in " << data;

		static const boost::regex cookie_regex( "\\s*([^=]+)=([^;]+)(;|$)*",boost::regex_constants::perl );

		int name_value_pairs_found = 0;
		

		boost::sregex_iterator match( data.begin(), data.end(), cookie_regex );
		boost::sregex_iterator end;

		while ( match != end ){
			cookies[ (*match)[0] ] = (*match)[1];
			*match++;
			name_value_pairs_found++;
		}
		return name_value_pairs_found;

	}


	// takes data and tries to figure out what it is. 
	bool
	Request::parse_request( std::string &data ){
		BOOST_LOGL( ewslog, info ) << "Parsing new request:\n" << data;

		typedef boost::split_iterator<std::string::iterator> sections_iterator_t;
		sections_iterator_t sections=
			boost::make_split_iterator( data, boost::first_finder("\r\n\r\n", boost::is_iequal() ) );

		if ( sections_iterator_t() == sections ){
			BOOST_LOGL( ewslog, err ) << "Invalid Request! Failed to find \\r\\n\\r\\n mark to split headers & body";
			return false;
		}
		std::string headers_str=boost::copy_range<std::string>(*sections);
		BOOST_LOGL( ewslog,info ) << "Headers:\n" << headers_str;

		typedef std::vector<std::string> headers_t;
		headers_t headers;
		boost::split( headers, headers_str, boost::is_any_of("\r\n"), boost::token_compress_on );

		if ( headers.empty() ){
			BOOST_LOGL( ewslog, err ) << "Invalid Request! Failed to find any headers.";
			return HTTPPARSESTATE_INVALIDREQUEST;
		} else {
			// everything must be uppercase according to rfc2616
			static const boost::regex reqline_regex(
				"^(OPTIONS|GET|HEAD|POST|PUT|DELETE|TRACE|CONNECT) ([^ ]*) HTTP/([^ ]+)",
				boost::regex_constants::perl  );
			boost::smatch what;

			if ( boost::regex_match( headers[0], what, reqline_regex ) ) {
				BOOST_LOGL(ewslog,info) << "Looks like we have a HTTP/" << 
					what[3] << " " << what[1] << " for " << what[2];
				// get the info from the request line
				this->method = GetRequestMethodFromString( what[1] );
				this->uri = what[2];
				this->http_version = what[3];

				// check to see if the uri appeared to have req parameter data in it
				parse_form_elements( this->uri );
			} else {
				BOOST_LOGL( ewslog, err ) << "Invalid Request! First line was: " << headers[0];
				return HTTPPARSESTATE_INVALIDREQUEST;
			} // end match on request line
		}


		for( unsigned int index = 1; index < headers.size(); ++index ){

			typedef vector< string > vec_t;
			vec_t record;
			boost::split( record, headers[index],
				      boost::is_any_of(": "), boost::token_compress_on );

			if ( record.size() != 2 ){
				BOOST_LOGL( ewslog, err ) << "Invalid Header #" << index << ": " << headers[index];
				return HTTPPARSESTATE_INVALIDREQUEST;
			} else {	
				this->request_headers[ boost::to_upper_copy( record[0] ) ] = record[1];
			}
		}


		// if this is an HTTP/1.1 request, then it had better have a Host: header
		if ( this->http_version == "1.1" ) {
			if ( ! this->request_headers.count("HOST") ) {
				BOOST_LOGL( ewslog, err ) << "HTTP/1.1 request made, but host header not found";
				return HTTPPARSESTATE_INVALIDREQUEST;
			}

			string_map_t::const_iterator te = request_headers.find("TRANSFER-ENCODING");
			if ( request_headers.end() != te && boost::to_lower_copy(te->second )== "chunked" ){
				BOOST_LOGL( ewslog, err ) << "NO SUPPORT FOR CHUNKED ENCODING";
				return HTTPPARSESTATE_INVALIDREQUEST;
			}
		}
		string_map_t::const_iterator cookie = request_headers.find("COOKIE");
		if ( request_headers.end() != cookie ){
			parse_cookies( cookie->second );
		}

		string_map_t::const_iterator cl = request_headers.find( "CONTENT-LENGTH" );
		if ( request_headers.end() == cl || ! boost::lexical_cast<unsigned int>( cl->second ) ) {
			return HTTPPARSESTATE_COMPLETEREQUEST;
		} else {
			++sections;

			this->body_ = boost::copy_range<std::string>(*sections);

			// if we're dealing with multi-part form attachments
			string_map_t::const_iterator ct=request_headers.find("CONTENT-TYPE");

			if ( request_headers.end() != ct && boost::to_lower_copy( ct->second.substr ( 0, 9 ) ) == "multipart" ) {
				// handle the body as if it's multipart form data
				if ( parse_multipart_form_data() ) {
					return HTTPPARSESTATE_COMPLETEREQUEST;
				} else {
					BOOST_LOGL( ewslog, err ) << "Mishandled multi-part form data for unknown reason";
					return HTTPPARSESTATE_INVALIDREQUEST;
				}
			} else {
				// the body is just one piece
				
				// check for any form data
				parse_form_elements( this->body_ );
				BOOST_LOGL( ewslog, info ) << "Request Completed";
				return HTTPPARSESTATE_COMPLETEREQUEST;
			}
		}
		return HTTPPARSESTATE_INVALID;
	}





////////////////////////////////////////////////////////////////////
//  HTTP REQUEST
//
////////////////////////////////////////////////////////////////////


	Request::Request ( connection *conn ) :
		method ( REQUESTMETHOD_UNKNOWN ),
		uri ( "" ),
		http_version( "" ),
		connection_ ( conn )
	{ }



	Request::~Request ( )
	{ }


// HELPER FUNCTIONS

// Takes a char* buffer and grabs a line off it, puts the new line in irsLine
//   and shrinks the buffer by the size of the line and sets ipnBufferLength to 
//   the new size

// returns 1 if it got a line, 0 otherwise
	bool GetNextLine ( std::string & irsLine, ///< line removed from *ippsBuffer
			  std::string & irsBuffer ///< buffer from which to remove a line
		)
	{

		typedef boost::split_iterator<string::iterator> string_find_iterator;
		string_find_iterator headers=
			boost::make_split_iterator( irsBuffer, boost::first_finder("\r\n\r\n", boost::is_iequal()  ) );

		
		BOOST_LOG(ewslog) << "Headers: " << *headers;
//std::string( irsBuffer, 0, irsBuffer.find_first_of( "\r\n\r\n" ) );

		typedef vector< boost::iterator_range<string::iterator> > find_vector_type;

		find_vector_type FindVec; // #1: Search for separators

		boost::split( FindVec, irsBuffer, boost::is_any_of("\r\n"), boost::token_compress_on ); // FindVec == { [abc],[ABC],[aBc] }
		BOOST_LOG( ewslog ) << "GetNextLine found: " << FindVec.size();
		for ( find_vector_type::iterator it=FindVec.begin(); it != FindVec.end(); ++it ){
			BOOST_LOG( ewslog ) << *it;
		}
		irsLine="";

		bool result = false;
		boost::smatch what;
		// can't use split, because we lose our \r\n
		static const boost::regex regex( "^([^\\r]*\\r\\n)(.*)$", boost::regex_constants::perl | boost::regex_constants::mod_s );
		if ( boost::regex_match( irsBuffer, what, regex ) ){
			irsLine = what[ 0 ];
			irsBuffer = what[ 1 ];
			result=true;
		} else {
			irsLine = "";
		}
		return result;
	}


/// List of possible HTTP request methods
	char * RequestMethodStrings [] = { "OPTIONS", "GET", "HEAD", "POST", 
					   "PUT", "DELETE", "TRACE", "CONNECT", "*" };


	Request::Method
	GetRequestMethodFromString ( const std::string & isRequestMethod ){

		int i = 0;

		for ( i = 0; i < Request::REQUESTMETHOD_LAST; i++ ) {

			if ( isRequestMethod == RequestMethodStrings [ i ] ) {

				break;

			}

		}

		return ( Request::Method ) i;

	}

	std::string & mytolower (std::string & s ///< string to make lowercase
		) 
	{ 

		// had to remove the std:: from std::tolower because vc++ 6 complained
		std::transform ( s.begin(), s.end(), s.begin(), std::ptr_fun<int, int>(tolower));
		return s;

	}


	std::string
	Request::remote_address(){
		return connection_->socket().remote_endpoint().address().to_string();
	}
	
	int Request::remote_port(){
		return connection_->socket().remote_endpoint().port();
	}




} // namespace ews
