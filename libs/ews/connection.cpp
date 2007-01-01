#include "ews/connection.hpp"

#include "ews/request_handler.hpp"
#include "ews/request_parser.hpp"
#include "ews/server.hpp"
#include <vector>
#include <boost/bind.hpp>

namespace ews {

	connection::connection( asio::io_service& io_service,
				network::connection_manager& manager,
				const boost::filesystem::path &d_root,
				const boost::filesystem::path &t_root )
		: network::connection( io_service, manager ),
		  doc_root( d_root ),
		  tmpl_root( t_root ),
		  request_(this),
		  reply_(this)
	{
		BOOST_LOGL(www,debug) << "NEW CONNECTION: " << (int)this << std::endl;
	}


	void connection::start() {
		this->socket().async_read_some( asio::buffer(buffer_),
					boost::bind(&connection::handle_read, shared_from_this(),
						    asio::placeholders::error,
						    asio::placeholders::bytes_transferred) ); 
	}


	void connection::handle_read( const asio::error& e,
				      std::size_t bytes_transferred) 
	{
//		std::cout << "Server Handle Read: " << e << std::endl;
		BOOST_LOGL(www,debug) << "Server Handle Read: " << e;
		if (!e)	{

			boost::tribool result;
			boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
				request_, buffer_.data(), buffer_.data() + bytes_transferred );
			
			if (result) {
				BOOST_LOGL(www,debug) << "Begin Write: " <<  (int)this
							<< " : " << request_.uri << std::endl;
 	
				if ( ! request_handler::handle_request( request_, reply_ ) ){
					reply_.set_to( reply::internal_server_error );
				}
				BOOST_LOGL( www, info ) << (int)reply_.status << " " << request_.method << " " << request_.uri;
				for( reply::headers_t::const_iterator header=reply_.headers.begin();
				     reply_.headers.end() != header;
				     ++header ) {
					BOOST_LOGL( www, debug ) << "Outgoing Header: "
								   << header->first << " => " << header->second;
				}

				asio::async_write( this->socket(), reply_.to_buffers(),
						   boost::bind(&connection::handle_write, shared_from_this(),
							       asio::placeholders::error,
							       asio::placeholders::bytes_transferred ) );
			} else if ( ! result ) {
				reply_.set_to( reply::bad_request );
				asio::async_write( this->socket(), reply_.to_buffers(),
						  boost::bind(&connection::handle_write, shared_from_this(),
							      asio::placeholders::error,
							      asio::placeholders::bytes_transferred ));
			} else {
				this->socket().async_read_some(asio::buffer(buffer_),
							boost::bind(&connection::handle_read, shared_from_this(),
								    asio::placeholders::error,
								    asio::placeholders::bytes_transferred ));
			}
		} else if (e != asio::error::operation_aborted) {
			this->manager().stop( shared_from_this() );
		}
	}

	void connection::handle_write(const asio::error& e, std::size_t bytes_transferred )
	{
		if (e != asio::error::operation_aborted) {
			this->manager().stop(shared_from_this());
		}
		BOOST_LOGL(www,debug)
			<< "Wrote " << bytes_transferred << " bytes on connection "
			<< (int)this << " result: " << e.what();
	}

} // namespace ews
