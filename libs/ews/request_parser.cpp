#include "ews/request_parser.hpp"
#include "ews/request.hpp"
#include "ews/server.hpp"
#include <sstream>
#include "boost/algorithm/string/case_conv.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

#include "boost/regex.hpp"
#include <algorithm>
#include <vector>

std::string
decode_str(  const std::string& in ){
	std::string out;
	out.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i) {
		if (in[i] == '%') {
			if ( ( i + 2 ) < in.size() ) {
				int value;
				std::istringstream is(in.substr(i + 1, 2));
				if (is >> std::hex >> value) {
					out += static_cast<char>(value);
				}
				i+=2;
			} else {
				i = in.size();
			}
		} else {
			out += in[i];
		}
	}
	return out;
}


namespace ews {

	request_parser::request_parser()
		: state_(method_start)
	{
	}

	void request_parser::reset()
	{
		state_ = method_start;
	}

	boost::tribool 
	request_parser::consume(request& req, char input) {
		switch (state_)
		{
		case method_start:
			if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
				BOOST_LOGL( www, err ) << "malformed request, invalid starting char";
				return false;
			} else {
				state_ = method;
				req.method.push_back(input);
				return boost::indeterminate;
			}
		case method:
//			std::cout << "Parsing: " << __LINE__ << std::endl;
			if (input == ' ') {
				state_ = uri;
				return boost::indeterminate;
			} else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
				BOOST_LOGL( www, err ) << "malformed request, invalid method char";
				return false;
			} else {
				req.method.push_back(input);
				return boost::indeterminate;
			}
		case uri_start:
			if (is_ctl(input)) {
				BOOST_LOGL( www, err ) << "malformed request, invalid uri char";
				return false;
			} else {
				state_ = uri;
				req.uri.push_back(input);
				return boost::indeterminate;
			}
		case uri:
			if (input == ' ') {
				state_ = http_version_h;
				return boost::indeterminate;
			} else if (is_ctl(input)) {
				BOOST_LOGL( www, err ) << "malformed request, invalid uri char";
				return false;
			} else {
				req.uri.push_back(input);
				return boost::indeterminate;
			}
		case http_version_h:
			if (input == 'H'){
				state_ = http_version_t_1;
				return boost::indeterminate;
			} else {
				BOOST_LOGL( www, err ) << "malformed request, invalid (H)TTP char";
				return false;
			}
		case http_version_t_1:
			if (input == 'T') {
				state_ = http_version_t_2;
				return boost::indeterminate;
			} else {
				BOOST_LOGL( www, err ) << "malformed request, invalid H(T)TP char";
				return false;
			}
		case http_version_t_2:
			if (input == 'T') {
				state_ = http_version_p;
				return boost::indeterminate;
			} else {
				BOOST_LOGL( www, err ) << "malformed request, invalid HT(T)P char";
				return false;
			}
		case http_version_p:
			if (input == 'P') {
				state_ = http_version_slash;
				return boost::indeterminate;
			} else {
				BOOST_LOGL( www, err ) << "malformed request, invalid HTT(P) char";
				return false;
			}
		case http_version_slash:
			if (input == '/') {
				req.http_version_major = 0;
				req.http_version_minor = 0;
				state_ = http_version_major_start;
				return boost::indeterminate;
			} else {
				BOOST_LOGL( www, err ) << "malformed request, invalid HTTP slash char";
				return false;
			}
		case http_version_major_start:
			if (is_digit(input)){
				req.http_version_major = req.http_version_major * 10 + input - '0';
				state_ = http_version_major;
				return boost::indeterminate;
			} else {
				BOOST_LOGL( www, err ) << "malformed request, HTTP version not digit";
				return false;
			}
		case http_version_major:
			if (input == '.') {
				state_ = http_version_minor_start;
				return boost::indeterminate;
			} else if (is_digit(input)) {
				req.http_version_major = req.http_version_major * 10 + input - '0';
				return boost::indeterminate;
			} else {
				BOOST_LOGL( www, err ) << "malformed request, HTTP version not digit then dot";
				return false;
			}
		case http_version_minor_start:
			if (is_digit(input)) {
				req.http_version_minor = req.http_version_minor * 10 + input - '0';
				state_ = http_version_minor;
				return boost::indeterminate;
			} else {
				BOOST_LOGL( www, err ) << "malformed request, HTTP version not digit dot digit";
				return false;
			}
		case http_version_minor:
			if (input == '\r') {
				state_ = expecting_newline_1;
				return boost::indeterminate;
			} else if (is_digit(input)) {
				req.http_version_minor = req.http_version_minor * 10 + input - '0';
				return boost::indeterminate;
			} else {
				BOOST_LOGL( www, err ) << "malformed request, HTTP version did not end as expected";
				return false;
			}
		case expecting_newline_1:
			if (input == '\n') {
				BOOST_LOGL( www, debug ) << "---------------------------------------------------";
				BOOST_LOGL( www, debug ) << req.method << " " << req.uri;
				BOOST_LOGL( www, debug ) << "---------------------------------------------------";
				state_ = header_line_start;
				return boost::indeterminate;
			} else {
				BOOST_LOGL( www, err ) << "malformed request, HTTP version did not end as expected";
				return false;
			}
		case header_line_start:
			if (input == '\r') {
				state_ = expecting_newline_3;
				return boost::indeterminate;
			} else if ( ! req.headers.empty() && ( input == ' ' || input == '\t') ) {
				state_ = header_lws;
				return boost::indeterminate;
			} else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
				BOOST_LOGL( www, err ) << "malformed request, HTTP header " << req.current_header_name << " is invalid";
				return false;
			} else {
				if ( ! req.current_header_name.empty() ){
					req.headers[ boost::to_upper_copy( decode_str( req.current_header_name ) ) ] 
						= decode_str( req.current_header_value );
					req.clear_current_header();
				}
 				req.current_header_name.push_back(input);
				state_ = header_name;
				return boost::indeterminate;
			}
		case header_lws:
			if (input == '\r') {
				state_ = expecting_newline_2;
				return boost::indeterminate;
			} else if (input == ' ' || input == '\t') {
				return boost::indeterminate;
			} else if (is_ctl(input)) {
				BOOST_LOGL( www, err ) << "malformed request, HTTP header " << req.current_header_name
						       << " value " << req.current_header_value << " is invalid";
				return false;
			} else {
				state_ = header_value;
				req.current_header_value.push_back(input);
				return boost::indeterminate;
			}
		case header_name:
			if (input == ':') {
				state_ = space_before_header_value;
				return boost::indeterminate;
			}else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
				BOOST_LOGL( www, err ) << "malformed request, HTTP header " << req.current_header_name
						       << " did not have : after name";
				return false;
			} else {
				req.current_header_name.push_back(input);
				return boost::indeterminate;
			}
		case space_before_header_value:
			state_ = header_value;
			if (input == ' ') {
				return boost::indeterminate;
			} else { // here we give the benifet of the doubt, as
				 // often time streaming clients don't bother with a space
				req.current_header_value.push_back(input);
				return boost::indeterminate;
			}
		case header_value:
			if (input == '\r') {
				state_ = expecting_newline_2;
				return boost::indeterminate;
			} else if (is_ctl(input)) {
				BOOST_LOGL( www, err ) << "malformed request, HTTP header " << req.current_header_name
						       << " value " << req.current_header_value << " didn't end in \\r";
				return false;
			} else {
				req.current_header_value.push_back(input);
				return boost::indeterminate;
			}
		case expecting_newline_2:
			if (input == '\n') {
				state_ = header_line_start;
				return boost::indeterminate;
			} else {
				BOOST_LOGL( www, err ) << "malformed request, HTTP header " << req.current_header_name
						       << " value " << req.current_header_value << " didn't end in \\n";
				return false;
			}
		case expecting_newline_3: // finished headers
//			std::cout << "Parsing: " << __LINE__ << std::endl;

			if ( input == '\n' ) {
				req.headers[ boost::to_upper_copy( decode_str( req.current_header_name ) ) ] 
					= decode_str( req.current_header_value );
				if ( req.method == "POST" ){
					req.content_length = req.get_header<unsigned int>( "CONTENT-LENGTH" );
				} else {
					req.content_length = 0;
				}

				BOOST_LOGL( www, debug ) << "Finished Header: "
							   << req.current_header_name << " => " 
							   << req.current_header_value;
				BOOST_LOGL( www, debug ) << "Headers Finished.  Body Length = " << req.content_length;
				req.clear_current_header();
				if ( req.content_length ){
					state_ = reading_body;
					return boost::indeterminate;
				} else {
					return true;
				}
			} else {
				BOOST_LOGL( www, err ) << "malformed request, HTTP headers did not end in \\r\\n\\r\\n";
				return false;
			}
		case reading_body:
			req.body.push_back( input );
			if ( req.content_length == req.body.size() ){
				return true;
			} else {
				return boost::indeterminate;
			}
		default:
			BOOST_LOGL( www, err ) << "malformed request, unknown parsing error";
			return false;
		}
	}


	bool request_parser::is_char(int c)
	{
		return c >= 0 && c <= 127;
	}

	bool request_parser::is_ctl(int c)
	{
		return c >= 0 && c <= 31 || c == 127;
	}

	bool request_parser::is_tspecial(int c)
	{
		switch (c)
		{
		case '(': case ')': case '<': case '>': case '@':
		case ',': case ';': case ':': case '\\': case '"':
		case '/': case '[': case ']': case '?': case '=':
		case '{': case '}': case ' ': case '\t':
			return true;
		default:
			return false;
		}
	}

	bool request_parser::is_digit(int c)
	{
		return c >= '0' && c <= '9';
	}


	
	bool parse_form_elements( request::varibles_t &map, const std::string &str ){
		static const boost::regex regex("[?]?([^?=]*)=([^&]*)&?", boost::regex_constants::perl );
		boost::smatch what;

		boost::sregex_iterator match( str.begin(), str.end(), regex );
		boost::sregex_iterator end;

		while ( match != end ){
			
			std::string name = decode_str( (*match)[ 1 ] );
			std::string value = decode_str( (*match)[ 2 ] );

			map[ name ].push_back( value );

			*match++;
		}
		return true;
	}

	bool
	request_parser::perform_final_decode( request &req ){
 		req.url=decode_str( req.uri );
		std::string::size_type pos = req.url.find_first_of('?');
		if ( std::string::npos != pos ){
			req.url.erase( pos, req.url.size() );
		}

		//req.url=trim_right_if(phone,is_any_of("0"));

		bool rv = ( parse_form_elements( req.varibles,  req.uri ) && 
			    parse_form_elements( req.varibles,  req.body ) );

		std::string cookies = req.get_header<std::string>( "COOKIE" );
		if ( ! cookies.empty() ){
			parse_form_elements( req.varibles, cookies );
			std::string ticket = req.single_value<std::string>("Ticket");
			if ( ! ticket.empty() ){
				try {
					req.user = Spinny::User::with_ticket( ticket );
				}
				catch( sqlite::not_found ){

				}
			}
		}

		std::vector< std::string > elements;
		boost::split( elements, req.url, boost::is_any_of("/"), boost::token_compress_on );
		req.num_url_elements=(char)elements.size()-1;
		if ( req.num_url_elements > 5 ){
			req.num_url_elements=5;
		}
		switch ( req.num_url_elements ){
		case 5:
			req.u5=elements[5];
		case 4:
			req.u4=elements[4];
		case 3:
			req.u3=elements[3];
		case 2:
			req.u2=elements[2];
		case 1:
			req.u1=elements[1];
		}

		if ( BOOST_IS_LOG_ENABLED( www, debug ) ) {
			BOOST_LOGL( www, debug ) << "found: " << (int)req.num_url_elements << " url elements: "
						   << req.u1 << "," 
						   << req.u2 << ","
						   << req.u3 << ","
						   << req.u4 << ","
						   << req.u5;
			BOOST_LOGL( www, debug ) << "Parsing out varibles " << ( rv ? "succeeded" : "failed" );
			for ( request::varibles_t::const_iterator rv=req.varibles.begin();
			      req.varibles.end() != rv;
				++rv )
			{
				BOOST_LOGL( www, debug ) << "    " << rv->first << ":";
				for ( request::varible_t::const_iterator vrb=rv->second.begin();
				      rv->second.end() != vrb;
				      ++vrb )
				{
					BOOST_LOGL( www, debug ) << "        " << *vrb;
				}
			}
			for ( request::headers_t::const_iterator header=req.headers.begin();
			      req.headers.end() != header;
			      ++header ) {

				BOOST_LOGL( www, err ) << "HTTP header " << header->first
						       << " = " << header->second;
			}

		}

		return rv;
	}


} // namespace ews

