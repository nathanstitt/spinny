#include "boost/bind.hpp"
#include "streaming/connection.hpp"
#include "streaming/server.hpp"
#include "streaming/lame.hpp"

using namespace Streaming;

Connection::Connection( asio::io_service& socket, Stream *stream ) :
	socket_( new asio::ip::tcp::socket( socket ) ),
	stream_( stream ),
	send_finished_(true)
{
	BOOST_LOGL(strm,debug) << "New Streaming Connection: " << this << std::endl;
}

Connection::Connection( boost::shared_ptr<asio::ip::tcp::socket> &socket ) :
	socket_(socket),
	stream_( NULL ),
	send_finished_(true)
{
	BOOST_LOGL(strm,debug) << "New Streaming Connection: " << this << std::endl;
}


void
Connection::write( const Chunk &c ){
	boost::mutex::scoped_lock lk(mutex_);

	BOOST_LOGL(strm,debug) << "Write requested on " << this 
			       << ( send_finished_ ? " sending" : " skipping due to lag" );
	if ( send_finished_ ){
		
		asio::async_write( *socket_, asio::const_buffer_container_1(c.data),
				   boost::bind( &Connection::handle_write, shared_from_this(),
						asio::placeholders::error,
						asio::placeholders::bytes_transferred ) );
		send_finished_=false;
	}
}

void
Connection::handle_write( const asio::error& e, std::size_t bytes_transferred ){
	boost::mutex::scoped_lock lk(mutex_);

	BOOST_LOGL(strm,debug)
		<< "Wrote " << bytes_transferred << " bytes on connection "
		<< this << " result: " << e.what();
	if ( e ){
		stream_->stop( shared_from_this() );
	} else {
		send_finished_=true;
	}
}


Connection::~Connection(){
	socket_->close();
}


void
Connection::set_stream( Stream *s ){
	stream_ = s;
}


boost::shared_ptr<asio::ip::tcp::socket>
Connection::socket(){
	return socket_;
}
