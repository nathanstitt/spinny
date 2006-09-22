#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include "connection_manager.hpp"
#include "request_handler.hpp"


namespace ews {

connection::connection(asio::io_service& io_service,
		       connection_manager& manager,
		       const boost::filesystem::path &doc_root )
  : socket_(io_service),
    connection_manager_(manager),
    doc_root_( doc_root )
{

}

asio::ip::tcp::socket& connection::socket()
{
  return socket_;
}

void connection::start()
{
  socket_.async_read_some(asio::buffer(buffer_),
      boost::bind(&connection::handle_read, shared_from_this(),
        asio::placeholders::error,
        asio::placeholders::bytes_transferred));
}

void connection::stop()
{
  socket_.close();
}

void connection::handle_read(const asio::error& e,
    std::size_t bytes_transferred)
{
	if (!e)	{
		boost::tribool result;
		boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
			request_, buffer_.data(), buffer_.data() + bytes_transferred);

		if (result) {
			request_handler *handler = request_handler::find_handler( request_ );
			handler->handle_request( request_, reply_, doc_root_ );
			asio::async_write(socket_, reply_.to_buffers(),
					  boost::bind(&connection::handle_write, shared_from_this(),
						      asio::placeholders::error));
		} else if ( ! result ) {
			reply_ = reply::stock_reply(reply::bad_request);
			asio::async_write(socket_, reply_.to_buffers(),
					  boost::bind(&connection::handle_write, shared_from_this(),
						      asio::placeholders::error));
		} else {
			socket_.async_read_some(asio::buffer(buffer_),
						boost::bind(&connection::handle_read, shared_from_this(),
							    asio::placeholders::error,
							    asio::placeholders::bytes_transferred));
		}
	} else if (e != asio::error::operation_aborted) {
		connection_manager_.stop(shared_from_this());
	}
}

void connection::handle_write(const asio::error& e)
{
  if (e != asio::error::operation_aborted)
  {
    connection_manager_.stop(shared_from_this());
  }
}

} // namespace ews
