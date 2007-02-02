#ifndef EWS_CONNECTION_MANAGER_HPP
#define EWS_CONNECTION_MANAGER_HPP

#include <set>
#include <boost/noncopyable.hpp>
#include "ews/connection.hpp"

namespace ews {

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
	class connection_manager
		: private boost::noncopyable
	{
	public:
		/// Add the specified connection to the manager and start it.
		void start(connection_ptr c);

		/// remove the connection from management
		void remove(connection_ptr c);

		/// Stop the specified connection.
		void stop(connection_ptr c);

		/// Stop all connections.
		void stop_all();

	private:
		/// The managed connections.
		std::set<connection_ptr> connections_;
	};


} // namespace ews

#endif // EWS_CONNECTION_MANAGER_HPP
