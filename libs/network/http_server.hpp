#ifndef _EWS_SERVER_HPP
#define _EWS_SERVER_HPP

#include "network/server.hpp"
#include "ews/connection.hpp"
#include "ews/exceptions.hpp"
#include "ews/connection_manager.hpp"
#include "ews/mime_types.hpp"
#include "ews/request_handler.hpp"
#include <boost/log/log.hpp>

BOOST_DECLARE_LOG(www);

namespace ews {

	typedef std::map< std::string, std::string > string_map_t;

/// The top-level class of the HTTP server.
	class server
		: public network::server
	{
	public:
		/// Construct the server to listen on the specified TCP address and port, and
		/// serve up files from the given directory.
		explicit server(const std::string& address, const std::string& port,
				const boost::filesystem::path& doc_root,
                                const boost::filesystem::path& tmpl_root );

		virtual
		::network::connection*
		new_connection( asio::io_service& io_service,
				network::connection_manager& manager ) const;
	private:

		boost::filesystem::path doc_root_;
		boost::filesystem::path tmpl_root_;
	};


} // namespace ews

#endif // _EWS_SERVER_HPP
