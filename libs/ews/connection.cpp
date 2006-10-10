#include "ews/connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include "ews/connection_manager.hpp"
#include "ews/request_handler.hpp"
#include "ews/request_parser.hpp"
#include "ews/server.hpp"

namespace ews {

	connection::connection(asio::io_service& io_service,
			       connection_manager& manager,
			       const boost::filesystem::path &doc_root,
			       const boost::filesystem::path &tmpl_root )
		: socket_(io_service),
		  connection_manager_(manager),
		  doc_root_( doc_root ),
		  tmpl_root_( tmpl_root )
	{
		BOOST_LOGL(ewslog,info) << "NEW CONNECTION: " << (int)this << " : " << (int)&reply_ << std::endl;
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
				      std::size_t bytes_transferred) 
	{
//		std::cout << "Server Handle Read: " << e << std::endl;
		BOOST_LOGL(ewslog,info) << "Server Handle Read: " << e;
		if (!e)	{

			boost::tribool result;
			boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
				request_, buffer_.data(), buffer_.data() + bytes_transferred );
			
			if (result) {
//				std::cout << "Begin Write: " <<  (int)this << " : "  << (int)&reply_ << std::endl;
				BOOST_LOGL(ewslog,info) << "Begin Write: " <<  (int)this << " : "  << (int)&reply_ << std::endl;

				request_handler *handler = request_handler::find_handler( request_ );

				handler->handle_request( this );

				BOOST_LOGL( ewslog, info ) << request_.uri;
				for( reply::headers_t::const_iterator header=reply_.headers.begin();
				     reply_.headers.end() != header;
				     ++header )
				{
					BOOST_LOGL( ewslog, info ) << "Outgoing Header: " 
								   << header->name << " => " << header->value;
				}

				asio::async_write(socket_, reply_.to_buffers(),
						  boost::bind(&connection::handle_write, shared_from_this(),
							      asio::placeholders::error,
							      asio::placeholders::bytes_transferred ) );

			} else if ( ! result ) {
				reply_.set_to( reply::bad_request );
				asio::async_write(socket_, reply_.to_buffers(),
						  boost::bind(&connection::handle_write, shared_from_this(),
							      asio::placeholders::error,
							      asio::placeholders::bytes_transferred ));
			} else {
				socket_.async_read_some(asio::buffer(buffer_),
							boost::bind(&connection::handle_read, shared_from_this(),
								    asio::placeholders::error,
								    asio::placeholders::bytes_transferred ));
			}
		} else if (e != asio::error::operation_aborted) {
			connection_manager_.stop(shared_from_this());
		}
	}

	void connection::handle_write(const asio::error& e, std::size_t bytes_transferred )
	{
		if (e != asio::error::operation_aborted) {
			connection_manager_.stop(shared_from_this());
		}
		BOOST_LOGL(ewslog,info)
			<< "Wrote " << bytes_transferred << " bytes on connection "
			<< (int)this << " result: " << e.what();
	}

} // namespace ews
