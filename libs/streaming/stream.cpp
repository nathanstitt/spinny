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
Stream::add_connection( Connection::ptr c, bool icy_taint  ) {
	BOOST_LOGL( strm, info ) << "Stream " << this
				 << " adding " << (icy_taint ? "" : "un" ) << "tainted connection "
				 << c->socket()->remote_endpoint().address().to_string();


	connections_.insert(c);
	c->set_stream( this );

	if ( ! controller_thread_ ) {
		BOOST_LOGL( strm, info ) << "Starting transcoder thread";
		running_ = true;
		controller_thread_=new boost::thread( boost::bind(&Stream::parcel,boost::ref(*this) ) );
	}

	return true;
}

std::list<asio::const_buffer>
Stream::history(){
	return lame_->history();
}

void
Stream::parcel(){

	BOOST_LOGL( strm, debug ) << this << " Stream::parcel()";

	while ( running_ ){

		Chunk chunk = lame_->get_chunk();

//		BOOST_LOGL( strm, debug ) << "Stream got chunk, sleep for " << chunk.milliseconds() << "ms";

		// Construct a timer without setting an expiry time.
		asio::deadline_timer timer(io_service_);

		// Set an expiry time relative to now.
		timer.expires_from_now( boost::posix_time::milliseconds( chunk.milliseconds() ) );

		std::for_each(connections_.begin(), connections_.end(),
			      boost::bind(&Connection::write, _1, chunk ) );


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
	BOOST_LOGL( strm, info ) << "Stopped connection " << &(*c) << " " << connections_.size() << " remaining";
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

void
Stream::song_order_changed( sqlite::id_t song_id, unsigned int new_position ){
	BOOST_LOGL( strm, debug ) << "stream " << this << " song " << song_id << " moved to " << new_position;
	lame_->song_order_changed( song_id, new_position );
}

bool
Stream::select_song( Spinny::Song::ptr song ){
	BOOST_LOGL( strm, debug ) << "stream " << this << " streaming " << song->db_id();
	return lame_->select_song( song );
}


Spinny::PlayList::ptr
Stream::playlist(){
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
	BOOST_LOGL( strm, info ) << "Stopped sending new data";

	io_service_.interrupt();
	networking_thread_->join();
	delete networking_thread_;

	BOOST_LOGL( strm, info ) << "Stopped listening";

	connections_.clear();

	BOOST_LOGL( strm, info ) << "Sockets closed";

	delete lame_;
}


void
Stream::handle_accept(const asio::error& e) {
	if (!e) {
		this->add_connection( new_connection_, false );

		new_connection_.reset( new Connection( io_service_, this ) );

		acceptor_.async_accept( *(new_connection_->socket()),
					boost::bind(&Stream::handle_accept, this, asio::placeholders::error));
	}
}
