/* @(#)ews_client.hpp
 */

#ifndef _EWS_CLIENT_H
#define _EWS_CLIENT_H 1

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <map>

//using ;

struct EWSTestClient {


	asio::io_service io_service;
	asio::ip::tcp::resolver resolver;

	struct error : public std::runtime_error {
		error( const std::string &msg ) : std::runtime_error( msg ) { }
	};

	EWSTestClient() : resolver(io_service) {

	}

	struct Page {
		unsigned int status;
		std::string body;
		typedef std::map<std::string,std::string> headers_t;
		headers_t headers;
	};

	Page post( const std::string &url, const std::string &body ){
		std::string req = "POST ";
		req +=url;
		req +=" HTTP/1.0\r\n";
		return request( req, body );
	}


	Page request( const std::string req_line, const std::string &req_body ){
		Page ret;
		asio::ip::tcp::resolver::query query( "localhost", "3001" );
		asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		asio::ip::tcp::resolver::iterator end;


		// Try each endpoint until we successfully establish a connection.
		asio::ip::tcp::socket socket(io_service);
		asio::error error = asio::error::host_not_found;
		while (error && endpoint_iterator != end)
		{
			socket.close();
			socket.connect(*endpoint_iterator++, asio::assign_error(error));
		}
		if (error) {
			throw EWSTestClient::error( error.what());
		}

		// Form the request. We specify the "Connection: close" header so that the
		// server will close the socket after transmitting the response. This will
		// allow us to treat all data up until the EOF as the content.
		asio::streambuf request;
		std::ostream request_stream(&request);
		request_stream << req_line
			       << "Host: localhost\r\n"
			       << "Accept: */*\r\n"
			       << "Connection: close\r\n"
			       << "Content-Length: " << req_body.size() << "\r\n"
			       << "\r\n"
			       << req_body;
		cout  << "Sending: " << req_line
		     << "Host: localhost\r\n"
		     << "Accept: */*\r\n"
		     << "Connection: close\r\n"
		     << "Content-Length: " << req_body.size() << "\r\n"
		     << "\r\n"
		     << req_body << endl;


		// Send the request.
		asio::write(socket, request);

		// Read the response status line.67
		asio::streambuf response;
		asio::read_until(socket, response, boost::regex("\r\n"));

		// Check that response is OK.
		std::istream response_stream(&response);
		std::string http_version;
		response_stream >> http_version;

		response_stream >> ret.status;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			std::string err("Invalid Response: ");
			err += http_version.substr(0, 5);
			throw EWSTestClient::error( err );
		}

		// Read the response headers, which are terminated by a blank line.
		asio::read_until(socket, response, boost::regex("\r\n\r\n"));

		typedef vector< string > split_vector_type;
		// Process the response headers.
		std::string header;
		while (std::getline(response_stream, header) && header != "\r"){
			split_vector_type record;
			boost::split( record, header, boost::is_any_of(": \r\n"),token_compress_on );
			ret.headers[ record[0] ] = record[1];
			cout << "Recd Header: " << record[0] << " => " << record[1] << endl;
		}

		std::stringstream rep_body;

		// Write whatever content we already have to output.
 		if ( response.size() > 0 )
 			rep_body << &response;

		cout << "Got: " << rep_body.str() << endl;

 		// Read until EOF, writing data to output as we go.
 		while (asio::read(socket, response,
 				  asio::transfer_at_least(1),
 				  asio::assign_error(error))){
			cout << "Reading more " << endl;
			rep_body << &response;
		}

 		if (error != asio::error::eof)
 			throw EWSTestClient::error( error.what() );

		ret.body = rep_body.str();

		return ret;
	}

	Page get( const std::string &url ){
		std::string req = "GET ";
		req += url;
		req +=" HTTP/1.0\r\n";
		return request( req, "" );
	}
};

#endif /* _EWS_CLIENT_H */

