
#include "ews/server.hpp"
#include <boost/bind.hpp>

BOOST_DEFINE_LOG( www, "www" )



namespace ews {

	server::server(const std::string& address, const std::string& port,
		       const boost::filesystem::path & doc_root,
		       const boost::filesystem::path & tmpl_root
		)
		: io_service_(),
		  acceptor_(io_service_),
		  connection_manager_(),
		  new_connection_( new connection( io_service_,
						   connection_manager_, doc_root, tmpl_root ) ),
		  doc_root_(doc_root),
		  tmpl_root_(tmpl_root)

	{
		// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
		asio::ip::tcp::resolver resolver(io_service_);
		asio::ip::tcp::resolver::query query(address, port);
		asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
		acceptor_.open(endpoint.protocol());
		acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
		acceptor_.bind(endpoint);
		acceptor_.listen();
		acceptor_.async_accept( *(new_connection_->socket()),
				       boost::bind(&server::handle_accept, this,
						   asio::placeholders::error)) ;

		BOOST_LOGL( www,debug) << "EWS running on: " 
					 << address
					 << ":" << port
					 << " root: " << doc_root_.string()
					 << " tmpl: " << tmpl_root_.string();

	}

	void server::run()
	{
		BOOST_LOGL( www, info ) << "Accepting connections";
		// The io_service::run() call will block until all asynchronous operations
		// have finished. While the server is running, there is always at least one
		// asynchronous operation outstanding: the asynchronous accept call waiting
		// for new incoming connections.
		io_service_.run();
	}

	void server::stop()
	{
		// Post a call to the stop function so that server::stop() is safe to call
		// from any thread.
		io_service_.post(boost::bind(&server::handle_stop, this));
	}

	void server::handle_accept(const asio::error& e)
	{
		if (!e)
		{
			connection_manager_.start(new_connection_);
			new_connection_.reset( new connection(io_service_,
							      connection_manager_, doc_root_, tmpl_root_ ));
			acceptor_.async_accept( *(new_connection_->socket()),
					       boost::bind(&server::handle_accept, this,
							   asio::placeholders::error));
		}
	}

	void server::handle_stop()
	{
		// The server is stopped by cancelling all outstanding asynchronous
		// operations. Once all operations have finished the io_service::run() call
		// will exit.
		acceptor_.close();
		connection_manager_.stop_all();
	}

} // namespace ews

