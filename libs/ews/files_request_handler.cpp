#include "ews/files_request_handler.hpp"
#include "ews/server.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include "boost/lexical_cast.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/exception.hpp" 
#include "boost/filesystem/fstream.hpp"
#include "ews/mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include <iostream>

namespace ews {


	files_request_handler::files_request_handler()
		: request_handler( false )
	{

	}


	bool
	files_request_handler::handle_request(const request& req, reply& rep, const boost::filesystem::path &doc_root ){

		std::string request_path=req.url;

		// Request path must be absolute and not contain "..".
		if (request_path.empty() || request_path[0] != '/'
		    || request_path.find("..") != std::string::npos)
		{
			BOOST_LOGL( ewslog, err ) << "Requested file had .. in the path: "
						  << request_path;
			rep.set_to( reply::bad_request );
			return false;
		}

		// If path ends in slash (i.e. is a directory) then add "index.html".
		if (request_path[request_path.size() - 1] == '/')
		{
			request_path += "index.html";
		}

		// Determine the file extension.
		std::size_t last_slash_pos = request_path.find_last_of("/");
		std::size_t last_dot_pos = request_path.find_last_of(".");
		std::string extension;
		if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
		{
			extension = request_path.substr(last_dot_pos + 1);
		}

		// Open the file to send back.
		boost::filesystem::ifstream is;
		try {
			is.open( doc_root / request_path, std::ios::in | std::ios::binary);
		}
		catch ( boost::filesystem::filesystem_error &e ){
			BOOST_LOGL( ewslog, err ) << "Caught filesystem error: " 
						  << e.what() 
						  << " req file was: "
						  << (doc_root / request_path).string();
			rep.set_to( reply::not_found );
			return false;
		}
		if (!is) {
			rep.set_to( reply::not_found );
			BOOST_LOGL( ewslog, err ) << "File not opened successfully: " 
						  << (doc_root / request_path).string();
			return false;
		}

		// Fill out the reply to be sent to the client.
		rep.status = reply::ok;
		char buf[512];
		while (is.read(buf, sizeof(buf)).gcount() > 0)
			rep.content.write(buf, is.gcount());
	
// 		rep.headers.resize(2);
// 		rep.headers[0].name = "Content-Length";
// 		rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.str().size());
// 		rep.headers[1].name = "Content-Type";
// 		rep.headers[1].value = mime_types::extension_to_type(extension);

		rep.set_basic_headers( mime_types::extension_to_type(extension) );

		return true;
	}

	bool
	files_request_handler::can_handle( const request &req ){
		return true;
	}


	files_request_handler::~files_request_handler(){}



} // namespace ews
