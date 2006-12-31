#include "network/connection.hpp"
#include "network/connection_manager.hpp"
#include "network/server.hpp"

#include <vector>
#include <boost/bind.hpp>

namespace network {

	connection::connection(asio::io_service& io_service,
			       connection_manager& manager )
		: socket_(io_service),
		  connection_manager_(manager)
	{
		BOOST_LOGL(netwk,debug) << "NEW CONNECTION: " << (int)this << std::endl;
	}

	asio::ip::tcp::socket& connection::socket() {
		return socket_;
	}

	void connection::start() {
		socket_.async_read_some(asio::buffer(buffer_),
					boost::bind(&connection::handle_read, shared_from_this(),
						    asio::placeholders::error,
						    asio::placeholders::bytes_transferred)); 
	}

	void connection::stop() {
		socket_.close();
	}

	void connection::handle_read( const asio::error& e,
				      std::size_t bytes_transferred )
	{
		BOOST_LOGL(netwk,debug) << "Server Handle Read: " << e;
		if ( !e )	{
			bytes_transferred=0; // fix for unused warning
		} else if (e != asio::error::operation_aborted) {
			connection_manager_.stop(shared_from_this());
		}
	}


	void connection::handle_write(const asio::error& e, std::size_t bytes_transferred )
	{
		if (e != asio::error::operation_aborted) {
			connection_manager_.stop(shared_from_this());
		}
		BOOST_LOGL( netwk,debug )
			<< "Wrote " << bytes_transferred << " bytes on connection "
			<< (int)this << " result: " << e.what();
	}

} // namespace ews
