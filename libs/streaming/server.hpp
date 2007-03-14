/* @(#)server.hpp
 */

#ifndef _STREAMING_SERVER_H
#define _STREAMING_SERVER_H 1

#include "streaming/stream.hpp"


BOOST_DECLARE_LOG(strm);

namespace Streaming {


	class Server
		: private boost::noncopyable
	{
	public:
		/// Construct the server to listen on the specified TCP address and port, and
		/// serve up files from the given directory.
		explicit Server( const std::string& address, unsigned int starting_port );

		/// Stop a stream with the playlist
		void stop(Spinny::PlayList::ptr);

		bool add_client( Spinny::PlayList::ptr pl, boost::shared_ptr<asio::ip::tcp::socket>, bool use_icy );

		Stream::ptr add_stream( Spinny::PlayList::ptr pl );

		void song_order_changed( Spinny::PlayList::ptr pl, sqlite::id_t song_id, unsigned int new_position );
		
		bool select_song( Spinny::PlayList::ptr pl, Spinny::Song::ptr song );

		~Server();
	private:
		unsigned int next_port();

		std::string address_;
		unsigned int starting_port_;

		typedef std::set<Stream::ptr> streams_t;
		streams_t streams_;


	};


} // namespace ews



#endif /* _STREAMING_SERVER_H */

