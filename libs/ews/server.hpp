#ifndef _EWS_SERVER_HPP
#define _EWS_SERVER_HPP

#include "boost/asio.hpp"
#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include "connection.hpp"
#include "connection_manager.hpp"
#include "ews/request_handler.hpp"
#include <boost/log/log.hpp>
#include "ews/mime_types.hpp"

BOOST_DECLARE_LOG(www);

namespace ews {

	typedef std::map< std::string, std::string > string_map_t;

/// The top-level class of the HTTP server.
	class server
		: private boost::noncopyable
	{
	public:
		/// Construct the server to listen on the specified TCP address and port, and
		/// serve up files from the given directory.
		explicit server(const std::string& address, const std::string& port,
				const boost::filesystem::path& doc_root,
				const boost::filesystem::path& tmpl_root
			);

		/// Run the server's io_service loop.
		void run();

		/// Stop the server.
		void stop();

	private:
		/// Handle completion of an asynchronous accept operation.
		void handle_accept(const asio::error& e);

		/// Handle a request to stop the server.
		void handle_stop();

		/// The io_service used to perform asynchronous operations.
		asio::io_service io_service_;

		/// Acceptor used to listen for incoming connections.
		asio::ip::tcp::acceptor acceptor_;

		/// The connection manager which owns all live connections.
		connection_manager connection_manager_;

		/// The next connection to be accepted.
		connection_ptr new_connection_;

		boost::filesystem::path doc_root_;
		boost::filesystem::path tmpl_root_;
	};


} // namespace ews

#endif // _EWS_SERVER_HPP
