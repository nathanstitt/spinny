#include "streaming/server.hpp"


BOOST_DEFINE_LOG( strm, "strm" )


using namespace Streaming;


Server::Server(const std::string& address, unsigned int starting_port ) :
	address_(address),
	starting_port_(starting_port)

{
	BOOST_LOGL( strm,info ) << "Streaming on: " 
			       << address
			       << " starting_port: " << starting_port;

	Spinny::PlayList::result_set rs = Spinny::PlayList::all();
	for ( Spinny::PlayList::result_set::iterator pl = rs.begin(); rs.end() != pl; ++pl ){
		BOOST_LOGL( strm, info ) << "Examining playlist " << pl->name() << " ( " << pl->size() << " songs )";
		if ( pl->size() ){
			this->add_stream( pl.shared_ptr() );
		}
	}
}


Server::~Server() {
	BOOST_LOGL( strm, info ) << "Stopping Streaming Server";
	streams_.clear();
}

void
Server::stop( Spinny::PlayList::ptr pl ) {
	for (streams_t::iterator stream=streams_.begin(); streams_.end() != stream; ++stream ){
		if ( (*stream)->playlist() == pl ){
			streams_.erase( *stream );
			break;
		}
	}
}

void
Server::song_order_changed( Spinny::PlayList::ptr pl, sqlite::id_t song_id, unsigned int new_position ){
	BOOST_LOGL( strm, info ) << "PL " << pl->db_id() << " song " << song_id << " moved to " << new_position;

	for (streams_t::iterator stream=streams_.begin(); streams_.end() != stream; ++stream ){
		BOOST_LOGL( strm, info ) << "PL " << pl->db_id() << " == " << (*stream)->playlist()->db_id();
		if ( (*stream)->playlist()->db_id() == pl->db_id() ){
			BOOST_LOGL( strm, debug ) << "PL match";
			(*stream)->song_order_changed( song_id, new_position );
			break;
		}
	}
}


Stream::ptr
Server::add_stream( Spinny::PlayList::ptr pl ){
	Stream::ptr s( new Stream( pl, address_, this->next_port() ) );
	streams_.insert( s );
	return s;
}


unsigned int
Server::next_port(){
	unsigned int port=starting_port_;
	bool found=true;
	while ( found ){
		found = false;
		for (streams_t::iterator stream=streams_.begin(); streams_.end() != stream; ++stream ){
			if ( (*stream)->port() == port ){
				found = true;
				port++;
			}
		}
	}
	return port;
}


bool
Server::add_client( Spinny::PlayList::ptr pl, boost::shared_ptr<asio::ip::tcp::socket> socket ){

	BOOST_LOGL(strm,info)<< "Adding Streaming client";

	if ( ! pl->size() ){
		BOOST_LOGL(strm,info)<< "NOT adding streaming client "
 				    << socket->remote_endpoint().address().to_string()
 				    << " as playlist is empty";
		return false;
	}

	boost::shared_ptr<Streaming::Connection> client( new Streaming::Connection( socket ) );
	Stream::ptr s;
	for (streams_t::iterator stream=streams_.begin(); streams_.end() != stream; ++stream ){
		if ( (*stream)->playlist()->db_id() == pl->db_id() ){
			s = *stream;
			break;
		}
	}

	if ( ! s ){
		BOOST_LOGL(strm,info) << "Failed to find stream for playlist id " << pl->db_id() << " creating one";
		
		s = this->add_stream( pl );
	}
	return s->add_connection( client );
}
