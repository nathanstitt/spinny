#ifndef STREAMING_CONNECTION_HPP
#define STREAMING_CONNECTION_HPP

#include "boost/asio.hpp"
#include "boost/array.hpp"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/filesystem/path.hpp"


namespace Streaming {
	class Chunk;
	class Stream;

	/// Represents a single connection from a client.
	class Connection
		: public boost::enable_shared_from_this<Connection>,
		  private boost::noncopyable
	{
	public:
		typedef boost::shared_ptr<Connection> ptr;

		explicit Connection(asio::io_service& io_service, Stream *stream );

		explicit Connection(asio::io_service& io_service );

		/// Stop all asynchronous operations associated with the connection.
		void stop();

		void set_stream( Stream *s );

		asio::ip::tcp::socket& socket();
		
		bool send_finished();

		void write( const Chunk &c );
	private:
		/// Handle completion of a write operation.
		void handle_write(const asio::error& e, std::size_t bytes_transferred );

		/// Socket for the connection.
		asio::ip::tcp::socket socket_;

		/// The manager for this connection.
		Stream *stream_;

		bool send_finished_;

	};



} // namespace Streaming

#endif // STREAMING_CONNECTION_HPP
