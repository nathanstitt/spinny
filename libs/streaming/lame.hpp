
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
		friend class Chunk;
		typedef boost::array<unsigned char,LAME_MAXMP3BUFFER> buff_t;
		struct buffer {
			buff_t data;
			int data_length;
			buffer *next;
		};
	public:


		Lame( Spinny::PlayList::ptr pl );

		Chunk get_chunk();

//		asio::const_buffer next_seconds( int seconds );
		
		~Lame();
	private:
		bool fill_buffer();
		void next_song();

		boost::mutex buffer_mutex;
		boost::condition buffer_condition;


		buffer *read_buffer;
		buffer *write_buffer;

		Spinny::PlayList::ptr pl;
		unsigned int cur_pos;

		mp3data_struct mp3input_data;
		lame_global_flags *lgf;

		FILE   *musicin;

		bool transcode();

		int get_audio( int buffer[2][1152] );

		int init_decode_file();

		boost::thread *lame_thread;

		bool running_;

	};

	struct Chunk {
		friend class Lame;
		Chunk( Lame::buffer *b,
		       unsigned short int br );
		unsigned int milliseconds();
		std::size_t size();
		asio::const_buffer data;
		unsigned short int bitrate;
	};

}

#endif // _STREAMING_LAME_HPP_
