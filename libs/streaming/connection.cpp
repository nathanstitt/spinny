#include "boost/bind.hpp"
#include "streaming/connection.hpp"
#include "streaming/server.hpp"
#include "streaming/lame.hpp"

using namespace Streaming;

Connection::Connection(asio::io_service& io_service, Stream *stream ) :
	socket_(io_service),
	stream_( stream ),
	send_finished_(true)
{
	BOOST_LOGL(strm,debug) << "NEW CONNECTION: " << (int)this << std::endl;
}


void
Connection::write( const Chunk &c ){
	if ( send_finished_ ){
		BOOST_LOGL(strm,debug) << "Begin write on " << this;
		asio::async_write( socket_, asio::const_buffer_container_1(c.data),
				   boost::bind( &Connection::handle_write, shared_from_this(),
						asio::placeholders::error,
						asio::placeholders::bytes_transferred ) );
	}
	send_finished_=false;
}

void
Connection::handle_write(const asio::error& e, std::size_t bytes_transferred ){
	send_finished_=true;
// 	if (e != asio::error::operation_aborted) {
// 		stream_->stop(shared_from_this());
// 	}
// //	this->
	BOOST_LOGL(strm,debug)
		<< "Wrote " << bytes_transferred << " bytes on connection "
		<< this << " result: " << e.what();
}

void
Connection::stop(){
	socket_.close();
}


void
Connection::set_stream( Stream *s ){
	stream_ = s;
}


asio::ip::tcp::socket&
Connection::socket(){
	return socket_;
}
