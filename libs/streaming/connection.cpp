#include "boost/bind.hpp"
#include "streaming/connection.hpp"
#include "streaming/server.hpp"
#include "streaming/lame.hpp"
#include "spinny/app.hpp"
#include "spinny/artist.hpp"

using namespace Streaming;

#define ICY_LENGTH 32768

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


/*

http://forums.radiotoolbox.com/viewtopic.php?t=74


icy-name:Unnamed Server\r\n
icy-genre:Unknown Genre\r\n
icy-pub:1\r\n
icy-br:56\r\n
icy-url:http://www.shoutcast.com\r\n
icy-irc:%23shoutcast\r\n
icy-icq:0\r\n
icy-aim:N%2FA\r\n
content-type: mime/type\r\n
icy-reset: 1\r\n
icy-prebuffer: ??\r\n 
icy-metaint:8192\r\n
\r\n 
*/

std::string&
Connection::icy_tags(){
	Spinny::Song::ptr song = stream_->current_song();
	icy_tags_ = "0";
	icy_tags_ += "StreamTitle='";
	icy_tags_ += song->artist()->name() + " - ";
	icy_tags_ += song->title() + "';";


	div_t sz = div( icy_tags_.size(), 16 );

	if ( sz.rem ){
		icy_tags_.append( 17-sz.rem, '\0' );
		sz.quot+=1;
	}
	icy_tags_[ 0 ] = sz.quot;
	BOOST_LOGL( strm, debug ) << "Icy tags size=" << sz.quot;
	return icy_tags_;
}

std::string&
Connection::icy_headers(){
	icy_tags_ = "ICY 200 OK\r\nContent-Type: audio/mpeg\r\nicy-name: ";
	icy_tags_ += stream_->playlist()->name() + "\r\nicy-url: http://";
	icy_tags_ += Spinny::App::instance()->config<std::string>("web_listen_address") + "\r\n";

	if ( this->using_icy() ){
		icy_tags_ += "icy-metaint: ";
		icy_tags_ += boost::lexical_cast<std::string>(ICY_LENGTH);
		icy_tags_ += "\r\n";
	}
	icy_tags_ += "\r\n";

	return icy_tags_;
}

void
Connection::write( const Chunk &c ){
	boost::recursive_mutex::scoped_lock lk(mutex_);

 	BOOST_LOGL(strm,debug) << "Write requested on " << this 
 			       << ( send_finished_ ? " sending" : " skipping due to lag" ) << " missed " << missed_count_
			       << " : ICY " << this->using_icy() << " / " << icy_count_;

	//asio::const_buffer_container_1 buffer;
	if ( send_finished_ ){

		missed_count_ = 0;
	
		if ( this->using_icy() && ( icy_count_ + c.size() > ICY_LENGTH ) ){
			std::string &tags = this->icy_tags();
			BOOST_LOGL(strm,debug) << "SENDING ICY after " << (ICY_LENGTH - icy_count_ )
					       << " bytes. Tag size: " << (int)tags[0] << " : " << tags;

			//std::size_t size = tags.size();

			boost::array<asio::const_buffer,3> bufs = { { 
					asio::buffer( c.data, ICY_LENGTH - icy_count_ ),
					asio::buffer( tags ),
					asio::buffer(  c.data+(ICY_LENGTH - icy_count_), c.size() )
				} };
			asio::async_write( *socket_, bufs,
					   boost::bind( &Connection::handle_write, shared_from_this(),
							asio::placeholders::error,
							asio::placeholders::bytes_transferred ) );

			icy_count_ = c.size() - ( ICY_LENGTH - icy_count_ );

		} else {
			asio::async_write( *socket_, asio::buffer(c.data ),
					   boost::bind( &Connection::handle_write, shared_from_this(),
							asio::placeholders::error,
							asio::placeholders::bytes_transferred ) );

			if ( this->using_icy() ){
				icy_count_ += c.size();
			}
		}

		send_finished_=false;
	} else if ( missed_count_ > 10 ){
		stream_->stop( shared_from_this() );	
	} else {
		missed_count_ += 1;
	}
}

void
Connection::write_buffers( const std::list<asio::const_buffer> &buffers ){
	boost::recursive_mutex::scoped_lock lk(mutex_);

 	for ( std::list<asio::const_buffer>::const_iterator buf = buffers.begin(); 
	      buf != buffers.end();
	      ++buf )
	{
		icy_count_ += asio::buffer_size( *buf  );

	}


	asio::async_write( *socket_, buffers,
			   boost::bind( &Connection::handle_write, shared_from_this(),
					asio::placeholders::error,
					asio::placeholders::bytes_transferred ) );
	send_finished_=false;
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
		asio::error error_close;
		socket_->close( asio::assign_error(error_close) );
		if (error_close)
		{
			BOOST_LOGL(strm,err ) << "Error " << error_close.what() << " closeing socket on connection " << this;
		}

	}
	catch( const asio::system_exception &e ){
		BOOST_LOGL( strm,info )<< "socket->close() raised " << e.what();
	}

	try {
		socket_.reset();

	}
	catch( const asio::system_exception &e ){
		BOOST_LOGL( strm,info )<< "~socket() raised " << e.what();
	}
}


void
Connection::set_stream( Stream *s ){
	stream_ = s;
	std::string &headers = this->icy_headers();

	BOOST_LOGL( strm,info )<< "New Connection " << this << " on stream " << s 
			       << "\nHeaders:\n" << headers;
	
	asio::async_write( *socket_, asio::buffer( headers ),
			   boost::bind( &Connection::handle_write, shared_from_this(),
					asio::placeholders::error,
					asio::placeholders::bytes_transferred ) );
	icy_count_=0;
	this->write_buffers( stream_->history() );
}


boost::shared_ptr<asio::ip::tcp::socket>
Connection::socket(){
	return socket_;
}
