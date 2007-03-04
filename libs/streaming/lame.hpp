
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
#include <list>

namespace Streaming {

	struct buffer;

	class Lame {
		friend class Chunk;
	public:

		Lame( Spinny::PlayList::ptr pl );

		Chunk get_chunk();

		std::list<asio::const_buffer>
		history();

		~Lame();
		
	private:
		typedef boost::array<unsigned char,LAME_MAXMP3BUFFER> buff_t;
		struct Buffer {
			Buffer();
			void destroy( int buffer_num=0 );
			void create();
			buff_t data;
			int data_length;
			Buffer *prev;
			Buffer *next;
			bool writable;
		};

		bool fill_buffer( Buffer *buff );
		void next_song();

		boost::mutex buffer_mutex;
		boost::condition buffer_condition;


//		Buffer *write_buffer;

		Spinny::PlayList::ptr pl;
		unsigned int cur_pos;

		mp3data_struct mp3input_data;
		lame_global_flags *lgf;

		FILE   *musicin;

		void transcode();

		int get_audio( int buffer[2][1152] );

		int init_decode_file();

		boost::thread *lame_thread;

		bool running_;
		Buffer *buffer_;

	};

	struct Chunk {
		friend class Lame;
		Chunk( Lame::Buffer *b,
		       unsigned short int br );
		unsigned int milliseconds();
		std::size_t size();
		asio::const_buffer data;
		unsigned short int bitrate;
	};

}

#endif // _STREAMING_LAME_HPP_
