/* @(#)stream.cpp
 */


#include "streaming/stream.hpp"
#include "streaming/server.hpp"
#include "streaming/lame.hpp"
#include "boost/bind.hpp"
#include <iostream>


using namespace Streaming;


Stream::Stream( Spinny::PlayList::ptr pl, const std::string& address, unsigned int port ) :
	pl_(pl),
	address_(address),
	port_(port),
	lame_( new Lame( pl ) ),
	acceptor_(io_service_),
	new_connection_( new Connection( io_service_, this ) ),
	controller_thread_(NULL),
	running_(false)
{ 
	BOOST_LOGL( strm,info ) << "New Stream for " << pl->name() << " on: " << address	<< ":" << port;

	asio::ip::tcp::resolver resolver(io_service_);

	asio::ip::tcp::resolver::query query( address, boost::lexical_cast<std::string>(port) );
	asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();
	acceptor_.async_accept( *(new_connection_->socket()),
				boost::bind(&Stream::handle_accept, this, asio::placeholders::error));

	networking_thread_=new boost::thread( boost::bind(&Stream::run,boost::ref(*this) ) );

}


bool
Stream::add_connection( Connection::ptr c  ) {
	connections_.insert(c);
	c->set_stream( this );

	if ( ! controller_thread_ ) {
		BOOST_LOGL( strm, info ) << "Starting transcoder thread";
		running_ = true;
		controller_thread_=new boost::thread( boost::bind(&Stream::parcel,boost::ref(*this) ) );
	}

	return true;
}

void
Stream::parcel(){

	BOOST_LOGL( strm, debug ) << this << " Stream::parcel()";

	while ( running_ ){

		Chunk chunk = lame_->get_chunk();

		BOOST_LOGL( strm, debug ) << "Stream got chunk, sleep for " << chunk.milliseconds() << "ms";

		std::for_each(connections_.begin(), connections_.end(),
			      boost::bind(&Connection::write, _1, chunk ) );

		// Construct a timer without setting an expiry time.
		asio::deadline_timer timer(io_service_);

		// Set an expiry time relative to now.
		timer.expires_from_now( boost::posix_time::milliseconds( chunk.milliseconds() ) );

		// Wait for the timer to expire.
		timer.wait();
	}
}



void
Stream::run(){
	io_service_.run();
}

void
Stream::stop( Connection::ptr c ) {
	connections_.erase(c);
	if ( connections_.empty() ){
		running_ = false;
		controller_thread_->join();
		delete controller_thread_;
		controller_thread_ = NULL;
	}
}

unsigned int
Stream::port(){
	return port_;
}

Spinny::PlayList::ptr
Stream::playlist(){
	BOOST_LOGL(strm,info) << __PRETTY_FUNCTION__ << " : " << __LINE__ << " " << (pl_);
	BOOST_LOGL(strm,info) << __PRETTY_FUNCTION__ << " : " << __LINE__ << " " << pl_->db_id();

	
	return pl_;
}

Stream::~Stream(){

	BOOST_LOGL( strm, info ) << "Shutting down stream on port " << port();

	if ( running_ ){
		running_ = false;
		controller_thread_->join();
		delete controller_thread_;
		controller_thread_ = NULL;
	}

	io_service_.interrupt();
	networking_thread_->join();
	delete networking_thread_;

	connections_.clear();

	delete lame_;
}


void
Stream::handle_accept(const asio::error& e) {
	if (!e) {
		this->add_connection( new_connection_ );

		new_connection_.reset( new Connection( io_service_, this ) );

		acceptor_.async_accept( *(new_connection_->socket()),
					boost::bind(&Stream::handle_accept, this, asio::placeholders::error));
	}
}
