#include "network/server.hpp"
#include <boost/bind.hpp>
#include <boost/log/log.hpp>

BOOST_DEFINE_LOG( www, "www" )



namespace ews {

	server::server(const std::string& address, const std::string& port,
		       const boost::filesystem::path & doc_root,
		       const boost::filesystem::path & tmpl_root
		)
		: network::server( address, port ),
		  doc_root_(doc_root),
		  tmpl_root_(tmpl_root)

	{

		BOOST_LOGL( www,debug) << "EWS running on: " 
					 << address
					 << ":" << port
					 << " root: " << doc_root_.string()
					 << " tmpl: " << tmpl_root_.string();
	}


	::network::connection*
	server::new_connection( asio::io_service& io_service, network::connection_manager& manager ) const {
		return new ews::connection( io_service, manager, doc_root_, tmpl_root_ );
	}


} // namespace ews

