/* @(#)stream.cpp
 */


#include "streaming/stream.hpp"
#include "streaming/server.hpp"
#include "streaming/lame.hpp"

#include <iostream>


using namespace Streaming;


Stream::Stream( Spinny::PlayList::ptr pl, const std::string& address, unsigned int port ) :
	address_(address),
	port_(port),
	lame_( new Lame( pl ) ),
	acceptor_(io_service_),
	new_connection_( new Connection( io_service_, this ) ),
	running_(true)
{ 
	BOOST_LOGL( strm,info ) << "New Stream for " << pl->name() << " on: " << address	<< ":" << port;

	asio::ip::tcp::resolver resolver(io_service_);

	asio::ip::tcp::resolver::query query( address, boost::lexical_cast<std::string>(port) );
	asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();
	acceptor_.async_accept( new_connection_->socket(),
				boost::bind(&Stream::handle_accept, this, asio::placeholders::error));


	networking_thread_=new boost::thread( boost::bind(&Stream::run,boost::ref(*this) ) );

	controller_thread_=new boost::thread( boost::bind(&Stream::parcel,boost::ref(*this) ) );

}


bool
Stream::add_connection( Connection::ptr c  ) {
	connections_.insert(c);
	//c->start();
	c->set_stream( this );
	return true;
}

void
Stream::parcel(){
	BOOST_LOGL( strm, debug ) << this << " Stream::parcel()";
	while ( running_ ){
		BOOST_LOGL( strm, debug ) << this << " getting new chunk";

		Chunk chunk = lame_->get_chunk();

BOOST_LOGL( strm, debug ) << __LINE__ << " getting new chunk";

		std::for_each(connections_.begin(), connections_.end(),
			      boost::bind(&Connection::write, _1, chunk ) );

BOOST_LOGL( strm, debug ) << __LINE__ << " getting new chunk";

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
Stream::stop( Connection::ptr c) {
	c->stop();
	connections_.erase(c);

}



unsigned int
Stream::port(){
	return port_;
}

Spinny::PlayList::ptr
Stream::playlist(){
	return pl_;
}

Stream::~Stream(){

	std::for_each(connections_.begin(), connections_.end(),
		      boost::bind(&Connection::stop, _1));

	connections_.clear();

	delete lame_;

	running_=false;

	controller_thread_->join();

	delete networking_thread_;
	networking_thread_=0;

	delete controller_thread_;
	controller_thread_=0;
}


void
Stream::handle_accept(const asio::error& e) {
	if (!e) {
		connections_.insert( new_connection_ );
		new_connection_->write( lame_->get_chunk() );

		new_connection_.reset( new Connection( io_service_, this ) );
		acceptor_.async_accept(new_connection_->socket(),
				       boost::bind(&Stream::handle_accept, this, asio::placeholders::error));
	}
	
}
