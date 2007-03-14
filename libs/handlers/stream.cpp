#include "boost/bind.hpp"
#include "handlers/shared.hpp"
#include "handlers/stream.hpp"
#include "spinny/song.hpp"
#include "spinny/artist.hpp"
#include "sqlite/comma.hpp"
#include "streaming/lame.hpp"

using namespace handlers;

Stream::Stream() : ews::request_handler( "Stream", Middle ) {}


// void
// Stream::send_more( Lame *ls, asio::ip::tcp::socket *socket, const asio::error& e, std::size_t bytes_transferred ) const {

// 	if ( asio::error::operation_aborted == e ) {
// 		return;
// 	}

// 	BOOST_LOGL( www,info ) << "Streamed " << (int)ls << " bytes: " << bytes_transferred;
// //	ls->get_chunk();
// //	BOOST_LOGL( www, info ) << "Done";

// //	std::vector<asio::const_buffer> cbs;
// //	cbs.push_back( ls->get_chunk() );
// // 	asio::async_write( *socket, cbs,

// // 			   boost::bind(&Stream::send_more,
// // 				       boost::ref(*this),
// // 				       boost::ref(ls),
// // 				       boost::ref(socket),
// // 				       asio::placeholders::error,
// // 				       asio::placeholders::bytes_transferred )
// // 		);
// }


ews::request_handler::RequestStatus
Stream::handle( const ews::request& req, ews::reply& rep ) const { 
	
	if ( req.u1 != "stream" ) {
		return Continue;
	}

 	BOOST_LOGL( www, info ) << "About to attempt socket detach";

 	Spinny::PlayList::ptr pl = Spinny::PlayList::load( boost::lexical_cast<sqlite::id_t>( req.u2 ) );


	if ( Spinny::App::instance()->streaming->add_client( pl, req.conn->socket(), req.get_header<bool>("ICY-METADATA") ) ){
		req.conn->detach_socket();
	} else {
		rep.set_to ( ews::reply::internal_server_error );
	}

	return Stop;

}


