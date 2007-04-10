/* @(#)streaming_client.hpp
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
#include "ews/server.hpp"

//using ;

struct StreamingTestClient {


	asio::io_service io_service;
	asio::ip::tcp::resolver resolver;
	asio::ip::tcp::socket socket;
	std::size_t header_length;
	std::string http_version;

	struct error : public std::runtime_error {
		error( const std::string &msg ) : std::runtime_error( msg ) { }
	};

	struct Headers {
		unsigned int status;
		std::string body;
		typedef std::map<std::string,std::string> line_t;
		line_t line;
	};
	Headers header;

	StreamingTestClient( int stream_id=1, bool taint=1 ) :
		resolver(io_service), 
		socket(io_service),
		header_length(0),
		http_version()
		
		{

//		asio::ip::tcp::resolver::query query( "209.51.162.163", "6048" );
		asio::ip::tcp::resolver::query query( "localhost", "3001" );
		asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		asio::ip::tcp::resolver::iterator end;

		// Try each endpoint until we successfully establish a connection.
		asio::error error = asio::error::host_not_found;
		while (error && endpoint_iterator != end)
		{
			socket.close();
			socket.connect(*endpoint_iterator++, asio::assign_error(error));
		}
		if (error) {
			throw StreamingTestClient::error( error.what());
		}

		asio::streambuf request;
		std::ostream request_stream(&request);
//  		request_stream << "GET /" << " HTTP/1.0\r\n"
//  			       << "Host: 64.236.34.97\r\n"
//  			       << "User-Agent: bmp/0.9.7\r\n";

 		request_stream << "GET /stream/" << stream_id << " HTTP/1.0\r\n"
 			       << "Host: localhost\r\n"
  			       << "User-Agent: bmp/0.9.7\r\n";

		if ( taint ){
			request_stream << "icy-metadata: 1\r\n";
		}
		request_stream << "\r\n";


		// Send the request.
		asio::write(socket, request);
		char last=' ';
		bool last_was_end=false;
		boost::array<char,1> c;
		std::string line;
		bool first=true;

		while ( true ){
			std::size_t read=asio::read( socket, asio::buffer(c) );
			header_length += read;
// 			if ( header_length > 200 ){
// 				break;
// 			}
//			cout << (int)c[0] << " : " << c[0] << "  |  ";
			if ( '\r' == last && '\n' == c[0] ){
//				cout << "BREAK : " << endl;
				if ( last_was_end ){
					break;
				}
				vector< string > record;
				boost::split( record, line, boost::is_any_of(": \r\n"), token_compress_on );

				if ( first ){
					first=false;
					header.status=boost::lexical_cast<unsigned int>( line.substr( 4, 3 ) );
				} else {
					cout << "ADDING HEADER: " << record[0] << "=>" << record[1] << endl;
					header.line[ boost::to_upper_copy(record[0]) ] = boost::to_upper_copy(record[1]);
				}
				line="";

				last_was_end=true;

			} else if ( '\r' != c[0] ) {
				last_was_end=false;
			}
			last=c[0];
			if ( '\n' != last ){
				line+=last;
			}
		}


	}


	~StreamingTestClient(){
		socket.close();
	}
};

#endif /* _EWS_CLIENT_H */

