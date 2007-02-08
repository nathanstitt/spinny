#ifndef _STREAMING_STREAM_H
#define _STREAMING_STREAM_H 1


#include "spinny/playlist.hpp"
#include "boost/asio.hpp"
#include "streaming/buffer.hpp"
#include <boost/log/log.hpp>

#include "boost/thread/thread.hpp"
#include "streaming/connection.hpp"


namespace Streaming {

	class Lame;

	class Stream {

	public:
		typedef boost::shared_ptr<Stream> ptr;

		Stream( Spinny::PlayList::ptr pl, const std::string& address, unsigned int port );

		bool add_connection( Connection::ptr conn );

		/// Stop the specified connection.
		void stop( Connection::ptr c);

		unsigned int port();

		Spinny::PlayList::ptr playlist();

		~Stream();

		void run();

		/// Handle completion of an asynchronous accept operation.
		void handle_accept(const asio::error& e);
	private:
		
		void parcel();

		Spinny::PlayList::ptr pl_;
		std::string address_;
		unsigned int port_;
		Lame *lame_;

		/// The io_service used to perform asynchronous operations.
		asio::io_service io_service_;

		/// Acceptor used to listen for incoming connections.
		asio::ip::tcp::acceptor acceptor_;

		Connection::ptr new_connection_;

		/// The connection manager which owns all live connections.
		typedef std::set<Connection::ptr> connections_t;
		connections_t connections_;

		boost::thread *controller_thread_;

		boost::thread *networking_thread_;

		bool running_;

	};

};


#endif /* _STREAMING_STREAM_H */

