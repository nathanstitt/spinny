
#ifndef _STREAMING_LAME_HPP_
#define _STREAMING_LAME_HPP_

#include "lame/lame.h"

#include "spinny/playlist.hpp"
#include "boost/log/log.hpp"
#include "boost/asio.hpp"
#include "boost/array.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/recursive_mutex.hpp"
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

		void song_order_changed( sqlite::id_t song_id, unsigned int index );

		~Lame();

		bool select_song( Spinny::Song::ptr song );

	private:
		void set_song( Spinny::Song::ptr song );

		void select_song( unsigned int index );

		enum MessageType {
			AwaitingRead,
			FillBuffer,
			SongChanged,
			Quitting,
		};

		typedef boost::array<unsigned char,LAME_MAXMP3BUFFER> buff_t;
		struct Buffer {
			Buffer();
			void destroy( int buffer_num=0 );
			void create();
			buff_t data;
			int data_length;
			Buffer *prev;
			Buffer *next;
//			bool writable;
		};

		bool fill_buffer( Buffer *buff );
		void next_song();

		boost::recursive_mutex buffer_mutex;
		boost::condition buffer_condition;


//		Buffer *write_buffer;

		Spinny::PlayList::ptr pl_;
		unsigned int current_pos_;
		sqlite::id_t current_song_id_;

		mp3data_struct mp3input_data;
		lame_global_flags *lgf;

		FILE   *musicin;

		void transcode();

		int get_audio( int buffer[2][1152] );

		int init_decode_file();

		boost::thread *lame_thread;

		MessageType action_;

		Buffer *buffer_;
	};

	struct Chunk {
		friend class Lame;
		Chunk( Lame::Buffer *b,unsigned short int br );
		unsigned int milliseconds() const;
		std::size_t size() const;
		asio::const_buffer data;
		unsigned short int bitrate;
	};

}

#endif // _STREAMING_LAME_HPP_
