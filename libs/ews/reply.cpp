#include "reply.hpp"
#include <string>
#include "boost/lexical_cast.hpp"
#include "ews/mime_types.hpp"
#include "ews/server.hpp"

namespace ews {

	namespace status_strings {

		const std::string ok =
			"HTTP/1.0 200 OK\r\n";
		const std::string created =
			"HTTP/1.0 201 Created\r\n";
		const std::string accepted =
			"HTTP/1.0 202 Accepted\r\n";
		const std::string no_content =
			"HTTP/1.0 204 No Content\r\n";
		const std::string multiple_choices =
			"HTTP/1.0 300 Multiple Choices\r\n";
		const std::string moved_permanently =
			"HTTP/1.0 301 Moved Permanently\r\n";
		const std::string moved_temporarily =
			"HTTP/1.0 302 Moved Temporarily\r\n";
		const std::string not_modified =
			"HTTP/1.0 304 Not Modified\r\n";
		const std::string bad_request =
			"HTTP/1.0 400 Bad Request\r\n";
		const std::string unauthorized =
			"HTTP/1.0 401 Unauthorized\r\n";
		const std::string forbidden =
			"HTTP/1.0 403 Forbidden\r\n";
		const std::string not_found =
			"HTTP/1.0 404 Not Found\r\n";
		const std::string internal_server_error =
			"HTTP/1.0 500 Internal Server Error\r\n";
		const std::string not_implemented =
			"HTTP/1.0 501 Not Implemented\r\n";
		const std::string bad_gateway =
			"HTTP/1.0 502 Bad Gateway\r\n";
		const std::string service_unavailable =
			"HTTP/1.0 503 Service Unavailable\r\n";

		asio::const_buffer to_buffer(reply::status_type status)
		{
			switch (status)
			{
			case reply::ok:
				return asio::buffer(ok);
			case reply::created:
				return asio::buffer(created);
			case reply::accepted:
				return asio::buffer(accepted);
			case reply::no_content:
				return asio::buffer(no_content);
			case reply::multiple_choices:
				return asio::buffer(multiple_choices);
			case reply::moved_permanently:
				return asio::buffer(moved_permanently);
			case reply::moved_temporarily:
				return asio::buffer(moved_temporarily);
			case reply::not_modified:
				return asio::buffer(not_modified);
			case reply::bad_request:
				return asio::buffer(bad_request);
			case reply::unauthorized:
				return asio::buffer(unauthorized);
			case reply::forbidden:
				return asio::buffer(forbidden);
			case reply::not_found:
				return asio::buffer(not_found);
			case reply::internal_server_error:
				return asio::buffer(internal_server_error);
			case reply::not_implemented:
				return asio::buffer(not_implemented);
			case reply::bad_gateway:
				return asio::buffer(bad_gateway);
			case reply::service_unavailable:
				return asio::buffer(service_unavailable);
			default:
				return asio::buffer(internal_server_error);
			}
		}

	} // namespace status_strings

	namespace misc_strings {

		const char name_value_separator[] = { ':', ' ' };
		const char crlf[] = { '\r', '\n' };

	} // namespace misc_strings


	namespace stock_replies {

		static const char ok[] = "";
	
		const char created[] =
			"<html>"
			"<head><title>Created</title></head>"
			"<body><h1>201 Created</h1></body>"
			"</html>";
		const char accepted[] =
			"<html>"
			"<head><title>Accepted</title></head>"
			"<body><h1>202 Accepted</h1></body>"
			"</html>";
		const char no_content[] =
			"<html>"
			"<head><title>No Content</title></head>"
			"<body><h1>204 Content</h1></body>"
			"</html>";
		const char multiple_choices[] =
			"<html>"
			"<head><title>Multiple Choices</title></head>"
			"<body><h1>300 Multiple Choices</h1></body>"
			"</html>";
		const char moved_permanently[] =
			"<html>"
			"<head><title>Moved Permanently</title></head>"
			"<body><h1>301 Moved Permanently</h1></body>"
			"</html>";
		const char moved_temporarily[] =
			"<html>"
			"<head><title>Moved Temporarily</title></head>"
			"<body><h1>302 Moved Temporarily</h1></body>"
			"</html>";
		const char not_modified[] =
			"<html>"
			"<head><title>Not Modified</title></head>"
			"<body><h1>304 Not Modified</h1></body>"
			"</html>";
		const char bad_request[] =
			"<html>"
			"<head><title>Bad Request</title></head>"
			"<body><h1>400 Bad Request</h1></body>"
			"</html>";
		const char unauthorized[] =
			"<html>"
			"<head><title>Unauthorized</title></head>"
			"<body><h1>401 Unauthorized</h1></body>"
			"</html>";
		const char forbidden[] =
			"<html>"
			"<head><title>Forbidden</title></head>"
			"<body><h1>403 Forbidden</h1></body>"
			"</html>";
		const char not_found[] =
			"<html>"
			"<head><title>Not Found</title></head>"
			"<body><h1>404 Not Found</h1></body>"
			"</html>";
		const char internal_server_error[] =
			"<html>"
			"<head><title>Internal Server Error</title></head>"
			"<body><h1>500 Internal Server Error</h1></body>"
			"</html>";
		const char not_implemented[] =
			"<html>"
			"<head><title>Not Implemented</title></head>"
			"<body><h1>501 Not Implemented</h1></body>"
			"</html>";
		const char bad_gateway[] =
			"<html>"
			"<head><title>Bad Gateway</title></head>"
			"<body><h1>502 Bad Gateway</h1></body>"
			"</html>";
		const char service_unavailable[] =
			"<html>"
			"<head><title>Service Unavailable</title></head>"
			"<body><h1>503 Service Unavailable</h1></body>"
			"</html>";

		const char*
		to_string(reply::status_type status)
		{
			switch (status)
			{
			case reply::ok:
				return ok;
			case reply::created:
				return created;
			case reply::accepted:
				return accepted;
			case reply::no_content:
				return no_content;
			case reply::multiple_choices:
				return multiple_choices;
			case reply::moved_permanently:
				return moved_permanently;
			case reply::moved_temporarily:
				return moved_temporarily;
			case reply::not_modified:
				return not_modified;
			case reply::bad_request:
				return bad_request;
			case reply::unauthorized:
				return unauthorized;
			case reply::forbidden:
				return forbidden;
			case reply::not_found:
				return not_found;
			case reply::internal_server_error:
				return internal_server_error;
			case reply::not_implemented:
				return not_implemented;
			case reply::bad_gateway:
				return bad_gateway;
			case reply::service_unavailable:
				return service_unavailable;
			default:
				return internal_server_error;
			}
		}

	} // namespace stock_replies

	static NEOERR *
	cs_renderer ( void *r, char *contents ){
		reply *rep = reinterpret_cast<reply*>( r );
//		BOOST_LOGL( ewslog, info ) << "Rendered TMPL to:\n" << contents;
		rep->content << contents;
		return STATUS_OK;
	}


	bool
	reply::parse_template(){
		NEOERR *cs_res;
//		BOOST_LOGL( ewslog, info ) << "Parsing HDF:\n" << content.str();
		BOOST_LOGL( ewslog, info ) << "Parsing TMPL File: " << template_.string();
		if ( STATUS_OK != ( cs_res =
				    hdf_read_string ( hdf_, const_cast<char*>( content.str().c_str() ) ) ) ){
			BOOST_LOGL( ewslog, err ) << "Error: " 
						  << cs_res->error << " : " << cs_res->desc
						  << " was encountered while parsing hdf: "
						  << content.str();
		} else if ( STATUS_OK != ( cs_res =
					   cs_parse_file( cs_parse_, template_.string().c_str() ) ) ) {
			BOOST_LOGL( ewslog, err ) << "Error: " 
						  << cs_res->error << " : " << cs_res->desc
						  << " was encountered while parsing tmpl file: "
						  << cs_res->file << ":" << cs_res->lineno;
			nerr_log_error(cs_res);
		}
		if ( STATUS_OK == cs_res ){
			content.str("");
			cs_res = cs_render( cs_parse_, this, cs_renderer );
		}
		if ( STATUS_OK != cs_res ){
			BOOST_LOGL( ewslog, err ) << "Error: " 
						  << cs_res->error << " : " << cs_res->desc
						  << " failed to render hdf to template";
		}
		return ( STATUS_OK == cs_res );
	}



	std::vector<asio::const_buffer>
	reply::to_buffers() {
		std::vector<asio::const_buffer> buffers;
		buffers.push_back(status_strings::to_buffer(status));

		if ( ! template_.empty() ){
			if ( parse_template() ){
				headers[ "Content-Length" ] = boost::lexical_cast<std::string>( content.size() );
			} else {
				template_ = "";
				this->set_to( internal_server_error );
			}
		}

		for (headers_t::const_iterator header = headers.begin();
		     headers.end() != header; 
		     ++header )
		{
			buffers.push_back(asio::buffer( header->first ) );
			buffers.push_back(asio::buffer( misc_strings::name_value_separator));
			buffers.push_back(asio::buffer( header->second ) );
			buffers.push_back(asio::buffer( misc_strings::crlf ) );
		}
		buffers.push_back( asio::buffer(misc_strings::crlf ) );
		buffers.push_back( asio::buffer( content.str() ) );

		return buffers;
	}


	reply::reply( const connection *conn ) :
		conn_(conn),
		cs_parse_(0),
		hdf_(0),
		tmpl_results_(0)
	{ }


	reply::~reply(){

	}

	bool
	reply::set_template( const boost::filesystem::path &tmpl ){
		NEOERR *cs_res;
		bool ret=false;
		if ( template_.empty() ){

			if ( STATUS_OK != ( cs_res = hdf_init ( &hdf_ ) ) ){
				BOOST_LOGL( ewslog, err ) << "An error was encountered while initializeing hdf: "
							  << cs_res->error << " : " << cs_res->desc;
			} else if ( STATUS_OK != ( cs_res
						   = cs_init ( &cs_parse_, hdf_ ) ) ){
				BOOST_LOGL( ewslog, err ) << "An error was encountered while initializeing clearsilver: "
							  << cs_res->error << " : " << cs_res->desc;
			} else if ( STATUS_OK != ( cs_res 
						   = hdf_set_value ( hdf_, "hdf.loadpaths.def", conn_->tmpl_root.string().c_str() ) ) ){
				BOOST_LOGL( ewslog, err ) << "Failed to set hdf load paths"
							  << cs_res->error << " : " << cs_res->desc;
			}
			if ( STATUS_OK == cs_res ){
				ret=true;
				template_ = conn_->tmpl_root / tmpl;
			} else {
				template_="";
			}
		} else {
			template_ = conn_->tmpl_root / tmpl;
		}

		return ret;
	}

	void
	reply::set_to( status_type stat ){
		this->status=stat;
		this->headers.clear();
		content.str( stock_replies::to_string(stat) );
		this->set_basic_headers( "html" );
	}


	bool
	reply::set_basic_headers( const std::string &ext ) {
		add_header( "Content-Type", ews::mime_types::extension_to_type( ext ) );
		add_header("Content-Length", boost::lexical_cast<std::string>( content.str().size() ) );
		return true;
	}

	bool
	reply::add_header( const std::string &name, const std::string &value ) {
		headers.insert( headers_t::value_type( name,value) );
		return true;
	}


	int
	reply::stream::buffer::overflow(int c) {
		buffer_.push_back( static_cast<char>(c) );
		return c;
	}
	unsigned int
	reply::stream::size() const {
		return buf_.buffer_.size();

	}

	const std::string&
	reply::stream::str() const {
		return buf_.buffer_;

	}

	const std::string&
	reply::stream::str( const std::string &str ){
		return buf_.buffer_ = str;
	}

	reply::stream::stream() :
		std::ostream( &buf_ ){ }



} // namespace ews
