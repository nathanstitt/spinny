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
		std::string headers;
	};

	Page get( const std::string &url ){
		Page ret;
		asio::ip::tcp::resolver::query query( "localhost", "3000" );
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
		request_stream << "GET " << url << " HTTP/1.0\r\n";
		request_stream << "Host: localhost\r\n";
		request_stream << "Accept: */*\r\n";
		request_stream << "Connection: close\r\n\r\n";

		// Send the request.
		asio::write(socket, request);

		// Read the response status line.
		asio::streambuf response;
		asio::read_until(socket, response, boost::regex("\r\n"));

		// Check that response is OK.
		std::istream response_stream(&response);
		std::string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			std::string err("Invalid Response: ");
			err += http_version.substr(0, 5);
			throw EWSTestClient::error( err );
		}

		ret.status=status_code;

		// Read the response headers, which are terminated by a blank line.
		asio::read_until(socket, response, boost::regex("\r\n\r\n"));


		// Process the response headers.
		std::string header;
		while (std::getline(response_stream, header) && header != "\r")
			ret.headers += header + "\n";
//			std::cout << header << "\n";
//		std::cout << "\n";

		// Write whatever content we already have to output.
//		if (response.size() > 0)
			//ret.body=&response;

		std::string body;
		while (std::getline(response_stream, body) && header != "\r"){
			ret.body += body + "\n";
		}

// 		// Read until EOF, writing data to output as we go.
// 		while (asio::read(socket, response,
// 				  asio::transfer_at_least(1),
// 				  asio::assign_error(error)))
// 			ret.body += &response;
// //			std::cout << &response;
// 		if (error != asio::error::eof)
// 			throw EWSTestClient::error( error.what() );

		return ret;

	}
};

#endif /* _EWS_CLIENT_H */

