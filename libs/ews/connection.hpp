#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include "boost/asio.hpp"
#include "boost/array.hpp"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/enable_shared_from_this.hpp"
#include "boost/filesystem/path.hpp"

#include "ews/reply.hpp"
#include "ews/request.hpp"
#include "ews/request_parser.hpp"

#include "network/connection.hpp"

namespace ews {



/// Represents a single connection from a client.
	class connection
		: public network::connection,
		  public boost::enable_shared_from_this<connection>
	{
	public:
		/// Construct a connection with the given io_service.
		explicit connection( asio::io_service& io_service,
				     network::connection_manager& manager,
				     const boost::filesystem::path &doc_root,
				     const boost::filesystem::path& tmpl_root
			);

		/// Start the first asynchronous operation for the connection.
		void start();


		boost::filesystem::path doc_root;

		boost::filesystem::path tmpl_root;

	private:
		/// Handle completion of a read operation.
		void handle_read(const asio::error& e, std::size_t bytes_transferred);

		/// Handle completion of a write operation.
		void handle_write(const asio::error& e, std::size_t bytes_transferred );

		/// Buffer for incoming data.
		boost::array<char, 8192> buffer_;

		/// The incoming request.
		request request_;

		/// The reply to be sent back to the client.
		reply reply_;

		/// The parser for the incoming request.
		request_parser request_parser_;

	};

	typedef boost::shared_ptr<connection> connection_ptr;


} // namespace ews

#endif // HTTP_CONNECTION_HPP
