#include "handlers/file_system.hpp"
#include "ews/server.hpp"
#include "ews/mime_types.hpp"
#include "ews/reply.hpp"
#include "ews/request.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include "boost/lexical_cast.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/exception.hpp" 
#include "boost/filesystem/fstream.hpp"
#include <iostream>

namespace handlers {


Files::Files() : request_handler( "Filesystem", Last ) { }


ews::request_handler::RequestStatus
Files::handle( const ews::request& req, ews::reply& rep ) const {
	std::string request_path=req.url;

// 		std::cout << "Attempting to serve file: " 
// 			  << ( req.conn->doc_root / request_path).string()
// 			  << std::endl;

	BOOST_LOGL( www, debug ) << "Attempting to serve file: " 
					   << ( req.conn->doc_root / request_path).string();

	// Request path must be absolute and not contain "..".
	if (request_path.empty() || request_path[0] != '/'
	    || request_path.find("..") != std::string::npos)
	{
		BOOST_LOGL( www, err ) << "Requested file had .. in the path: "
					  << request_path;
		throw ews::error("Illegal (for us) character seq .. in request");
	}

	// If path ends in slash (i.e. is a directory) then add "index.html".
	if (request_path[request_path.size() - 1] == '/') {
		request_path += "index.html";
	}

	// Determine the file extension.
	std::size_t last_slash_pos = request_path.find_last_of("/");
	std::size_t last_dot_pos = request_path.find_last_of(".");
	std::string extension;
	if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)	{
		extension = request_path.substr(last_dot_pos + 1);
	}

	// Open the file to send back.
	boost::filesystem::ifstream is;
	try {
		is.open( req.conn->doc_root / request_path, std::ios::in | std::ios::binary);
	}
	catch ( boost::filesystem::filesystem_error &e ){
		BOOST_LOGL( www, err ) << "Caught filesystem error: " 
				       << e.what() 
				       << " req file was: "
				       << ( req.conn->doc_root / request_path).string();
		throw ews::not_found_error( req.url.c_str() );
	}
	if (!is) {
		BOOST_LOGL( www, debug ) << "File not opened successfully: " 
					 << (req.conn->doc_root / request_path).string();
		throw ews::not_found_error( req.url.c_str() );
	}

	// Fill out the reply to be sent to the client.
	rep.status = ews::reply::ok;
	char buf[512];
	while (is.read(buf, sizeof(buf)).gcount() > 0){
		rep.content.write(buf, is.gcount());
	}
	BOOST_LOGL( www, debug ) << "Serving file: " << (req.conn->doc_root / request_path).string();
	BOOST_LOGL( www, debug  ) << "EXT: " << extension;
	rep.set_basic_headers( extension );

	return Stop;
}




} // namespace handlers
