
#ifndef _STREAMING_LAME_HPP_
#define _STREAMING_LAME_HPP_

#include "lame/lame.h"

#include "spinny/playlist.hpp"
#include "boost/log/log.hpp"
#include "boost/asio.hpp"
#include "boost/array.hpp"
#include "boost/thread/thread.hpp"
#include "boost/filesystem/fstream.hpp"
#include "streaming/server.hpp"
#include <string>


namespace Streaming {

	class Lame {
	public:
		Lame( Spinny::PlayList::ptr pl );

		bool start();
		bool is_running();
		bool stop();

		asio::const_buffer get_chunk();

		~Lame();
	private:
		boost::mutex buffer_mutex;
		boost::condition buffer_condition;

		typedef boost::array<unsigned char,LAME_MAXMP3BUFFER> buff_t;
		struct buffer {
			buff_t data;
			int data_length;
			buffer *next;
		};
		buffer *read_buffer;
		buffer *write_buffer;

		bool transcode_song( Spinny::Song::ptr song );

		Spinny::PlayList::ptr pl;

		mp3data_struct mp3input_data;
		lame_global_flags *lgf;

		FILE   *musicin;

		bool transcode();

		int get_audio( int buffer[2][1152] );

		int read_samples_mp3( short int mpg123pcm[2][1152], int );

		int init_decode_file();

		boost::thread *lame_thread;

	};

}

#endif // _STREAMING_LAME_HPP_
