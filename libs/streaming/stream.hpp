

#ifndef _STREAM_H
#define _STREAM_H 1

#include "spinny/playlist.hpp"
#include "boost/asio.hpp"
#include "streaming/buffer.hpp"
#include <boost/log/log.hpp>

BOOST_DECLARE_LOG(strm);

namespace Streaming {


	void Stop();


	class Stream {

	public:
		Stream( Spinny::PlayList::ptr pl );

		void decode_song( Spinny::Song::ptr song );

		char* mp3_data;
		unsigned int mp3_data_len;
	private:
		
		Spinny::PlayList::ptr pl_;
	};



};

#endif /* _STREAM_H */

