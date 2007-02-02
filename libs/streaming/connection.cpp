#include "boost/bind.hpp"
#include "streaming/connection.hpp"
#include "streaming/server.hpp"

using namespace Streaming;

Connection::Connection(asio::io_service& io_service, Stream *stream ) :
	socket_(io_service),
	stream_( stream )
{
	BOOST_LOGL(strm,debug) << "NEW CONNECTION: " << (int)this << std::endl;
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
