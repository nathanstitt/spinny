#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include "boost/asio.hpp"
#include "boost/array.hpp"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/filesystem/path.hpp"

#include "reply.hpp"
#include "request.hpp"
#include "request_parser.hpp"

namespace ews {


	class connection_manager;

/// Represents a single connection from a client.
	class connection
		: public boost::enable_shared_from_this<connection>,
		  private boost::noncopyable
	{
	public:
		/// Construct a connection with the given io_service.
		explicit connection(asio::io_service& io_service,
				    connection_manager& manager,
				    const boost::filesystem::path &doc_root );

		/// Get the socket associated with the connection.
		asio::ip::tcp::socket& socket();

		/// Start the first asynchronous operation for the connection.
		void start();

		/// Stop all asynchronous operations associated with the connection.
		void stop();

	private:
		/// Handle completion of a read operation.
		void handle_read(const asio::error& e, std::size_t bytes_transferred);

		/// Handle completion of a write operation.
		void handle_write(const asio::error& e, std::size_t bytes_transferred );

		/// Socket for the connection.
		asio::ip::tcp::socket socket_;

		/// The manager for this connection.
		connection_manager& connection_manager_;

		/// Buffer for incoming data.
		boost::array<char, 8192> buffer_;

		/// The incoming request.
		request request_;

		/// The parser for the incoming request.
		request_parser request_parser_;

		/// The reply to be sent back to the client.
		reply reply_;

		boost::filesystem::path doc_root_;
	};

	typedef boost::shared_ptr<connection> connection_ptr;


} // namespace ews

#endif // HTTP_CONNECTION_HPP
