#ifndef _NETWORK_STREAM_HPP
#define _NETWORK_STREAM_HPP

#include "boost/asio.hpp"
#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/log/log.hpp>
#include "network/connection_manager.hpp"

BOOST_DECLARE_LOG( netwk );

namespace network {

	class server
		: private boost::noncopyable
	{
	public:
		explicit server( const std::string& address, const std::string& port );

		void run();

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
	};


} // namespace network

#endif // _NETWORK_STREAM_HPP
