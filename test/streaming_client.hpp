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

	struct error : public std::runtime_error {
		error( const std::string &msg ) : std::runtime_error( msg ) { }
	};

	StreamingTestClient() : resolver(io_service), socket(io_service) {

		asio::ip::tcp::resolver::query query( "localhost", "6000" );
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

	}


	std::vector<unsigned short int>
	read( unsigned int size ){
		std::vector<unsigned short int> ret;
		while ( ret.size() < size ){
			asio::read( socket, asio::buffer( ret ) );
		}
	}

	~StreamingTestClient(){
		socket.close();
	}
};

#endif /* _EWS_CLIENT_H */

