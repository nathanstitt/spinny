#ifndef NETWORK_CONNECTION_MANAGER_HPP
#define NETWORK_CONNECTION_MANAGER_HPP

#include <set>
#include <boost/noncopyable.hpp>
#include "network/connection.hpp"

namespace network {

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
	class connection_manager
		: private boost::noncopyable
	{
	public:
		/// Add the specified connection to the manager and start it.
		void start( connection_ptr c );

		/// Stop the specified connection.
		void stop(connection_ptr c);

		/// Stop all connections.
		void stop_all();

	private:
		/// The managed connections.
		std::set<connection_ptr> connections_;
	};


} // namespace network

#endif // NETWORK_CONNECTION_MANAGER_HPP