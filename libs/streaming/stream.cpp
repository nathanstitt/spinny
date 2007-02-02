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
	new_connection_( new Connection( io_service_, this ) )


{ 
	BOOST_LOGL( strm,info ) << "New Stream on: " << address	<< ":" << port;

	asio::ip::tcp::resolver resolver(io_service_);

	asio::ip::tcp::resolver::query query( address, boost::lexical_cast<std::string>(port) );
	asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();
	acceptor_.async_accept( new_connection_->socket(),
				boost::bind(&Stream::handle_accept, this, asio::placeholders::error));
}


bool
Stream::add_connection( Connection::ptr c  ) {
	connections_.insert(c);
	//c->start();
	c->set_stream( this );
	return true;
}



void
Stream::stop( Connection::ptr c) {
	c->stop();
	connections_.erase(c);
}

void
Stream::stop_all() {
	std::for_each(connections_.begin(), connections_.end(),
		      boost::bind(&Connection::stop, _1));
	connections_.clear();
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
	delete lame_;
}


void
Stream::handle_accept(const asio::error& e) {
	if (!e) {
		connections_.insert( new_connection_ );
		new_connection_.reset( new Connection( io_service_, this ) );
		acceptor_.async_accept(new_connection_->socket(),
				       boost::bind(&Stream::handle_accept, this, asio::placeholders::error));
	}
}
