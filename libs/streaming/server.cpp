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
Server::add_client( Spinny::PlayList::ptr pl, Connection::ptr conn ){

	BOOST_LOGL(strm,info)<< "Adding Streaming client";

	bool found=false;
	for (streams_t::iterator stream=streams_.begin(); streams_.end() != stream; ++stream ){
		if ( (*stream)->playlist()->db_id() == pl->db_id() ){
			(*stream)->add_connection( conn );
			found=true;
		}
	}

	if ( ! found ){
		BOOST_LOGL(strm,info) << "Failed to find stream for playlist id " << pl->db_id() << " creating one";
		Stream::ptr stream = this->add_stream( pl );
		return stream->add_connection( conn );
	}
	return true;
}
