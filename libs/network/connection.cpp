#include "network/connection.hpp"
#include "network/connection_manager.hpp"
#include "network/server.hpp"

#include <vector>
#include <boost/bind.hpp>

namespace network {

	connection::connection(asio::io_service& io_service,
			       connection_manager& manager )
		: socket_(io_service),
		  manager_(manager)
	{
		BOOST_LOGL(netwk,debug) << "NEW CONNECTION: " << (int)this << std::endl;
	}


	void connection::start() {
		// noop by default
	}

	void connection::stop() {
		socket_.close();
	}

						     
	connection::~connection(){

	}

	asio::ip::tcp::socket&
	connection::socket() {
		return socket_;
	}

	connection_manager & 
	connection::manager(){
		return manager_;
	}

} // namespace ews
