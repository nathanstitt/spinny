#include "boost/bind.hpp"
#include "streaming/connection.hpp"
#include "streaming/server.hpp"
#include "streaming/lame.hpp"

using namespace Streaming;

Connection::Connection( asio::io_service& socket, Stream *stream ) :
	missed_count_(0),
	socket_( new asio::ip::tcp::socket( socket ) ),
	stream_( stream ),
	send_finished_(true),
	icy_count_( 0 ),
	wants_icy_(false)
{
	BOOST_LOGL(strm,debug) << "New Streaming Connection: " << this << std::endl;


}

Connection::Connection( boost::shared_ptr<asio::ip::tcp::socket> &socket ) :
	missed_count_(0),
	socket_(socket),
	stream_( NULL ),
	send_finished_(true),
	icy_count_( 0 ),
	wants_icy_(false)
{
	BOOST_LOGL(strm,debug) << "New Streaming Connection: " << this << std::endl;
}


void
Connection::set_socket_options(){
// 	asio::socket_base::linger option(true, 30);
// 	socket_->set_option(option);
}

void
Connection::write( const Chunk &c ){
	boost::recursive_mutex::scoped_lock lk(mutex_);

// 	BOOST_LOGL(strm,debug) << "Write requested on " << this 
// 			       << ( send_finished_ ? " sending" : " skipping due to lag" ) << " missed " << missed_count_;

	if ( send_finished_ ){
		missed_count_ = 0;
		asio::async_write( *socket_, asio::buffer(c.data),
				   boost::bind( &Connection::handle_write, shared_from_this(),
						asio::placeholders::error,
						asio::placeholders::bytes_transferred ) );
		send_finished_=false;
	} else if ( missed_count_ > 10 ){
		stream_->stop( shared_from_this() );	
	} else {
		missed_count_ += 1;
	}
}

void
Connection::write_history( const std::list<asio::const_buffer> &buffers ){

	if ( send_finished_ ){
		asio::async_write( *socket_, buffers,
				   boost::bind( &Connection::handle_write, shared_from_this(),
						asio::placeholders::error,
						asio::placeholders::bytes_transferred ) );
		send_finished_=false;
	}
}

bool
Connection::use_icy( bool val ){
	return wants_icy_ = val;
}

bool
Connection::using_icy(){
	return wants_icy_;
}

void
Connection::handle_write( const asio::error& e, std::size_t bytes_transferred ){
	boost::recursive_mutex::scoped_lock lk(mutex_);

  	BOOST_LOGL(strm,debug)
  		<< "Wrote " << bytes_transferred << " bytes on connection "
  		<< this << " result: " << e.what();

	if ( asio::error::success == e ){
		send_finished_=true;
	} else {
		BOOST_LOGL( strm, info ) << "Streaming connection disconnected";
		stream_->stop( shared_from_this() );
	}
}

std::string
Connection::remote_address(){
	return socket_->remote_endpoint().address().to_string();
}

void
close_error_handler( const asio::error& e ) {
	if ( e ){
		BOOST_LOGL( strm,err ) << "Socket close failed: " << e.what();
	} else {
		BOOST_LOGL( strm, debug ) << "Socket closed successfully";
	}
}

Connection::~Connection(){

	boost::recursive_mutex::scoped_lock lk(mutex_);

	BOOST_LOGL( strm,info )<< "Destroying Connection " << this;

	try {
		socket_->close( close_error_handler );
	}
	catch( const asio::error &e ){
		BOOST_LOGL( strm,info )<< "socket->close() raised " << e.what();
	}
}


void
Connection::set_stream( Stream *s ){
	stream_ = s;
}


boost::shared_ptr<asio::ip::tcp::socket>
Connection::socket(){
	return socket_;
}
