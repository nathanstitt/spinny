#ifndef NETWORK_CONNECTION_HPP
#define NETWORK_CONNECTION_HPP

#include "boost/asio.hpp"
#include "boost/array.hpp"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/enable_shared_from_this.hpp"

namespace network {


	class connection_manager;

	/// Represents a single connection from a client.
	class connection
		: private boost::noncopyable
	{
	public:
		/// Construct a connection with the given io_service.
		explicit connection(asio::io_service& io_service,
				    connection_manager& manager
			);


		/// Start the first asynchronous operation for the connection.
		virtual void start();

		/// Stop all asynchronous operations associated with the connection.
		virtual void stop();

		/// Get the socket associated with the connection.
		asio::ip::tcp::socket& socket();

		/// Get the manager associated with the connection.
		connection_manager& manager();

		virtual ~connection();
	private:

		/// Socket for the connection.
		asio::ip::tcp::socket socket_;

		/// The manager for this connection.
		connection_manager& manager_;
	};

	typedef boost::shared_ptr<connection> connection_ptr;

} // namespace network

#endif // NETWORK_CONNECTION_HPP
