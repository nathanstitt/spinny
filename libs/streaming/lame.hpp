/*
 *	Command line frontend program
 *
 *	Copyright (c) 1999 Mark Taylor
 *                    2000 Takehiro TOMIANGA
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _LAME_HPP_
#define _LAME_HPP_

#include "lame/lame.h"

#include "spinny/playlist.hpp"
#include "boost/log/log.hpp"
#include "boost/asio.hpp"
#include "boost/array.hpp"
#include "boost/thread/thread.hpp"
#include <string>


BOOST_DECLARE_LOG(lame);


class Lame {
public:
	Lame( Spinny::PlayList::ptr pl );

	asio::const_buffer get_chunk();


	bool transcode();

	bool to_buffers();

	~Lame();
private:
//	typedef boost::mutex::scoped_lock scoped_lock;
	boost::mutex buffer_mutex;
	boost::condition buffer_condition;
	

	typedef boost::array<unsigned char,LAME_MAXMP3BUFFER*2> buff_t;
	struct buffer {
		buff_t data;
		int data_length;
		buffer *next;
	};
	buffer *read_buffer;
	buffer *write_buffer;

	bool transcode_song();

	Spinny::PlayList::ptr pl;
	bool running;

	//sound_file_format input_format;   
	int enc_delay;             /* if decoder finds a Xing header */ 
	int enc_padding;           /* if decoder finds a Xing header */ 
//	unsigned int num_samples_read;

	mp3data_struct mp3input_data; /* used by MP3 */

	lame_global_flags *lgf;

	FILE   *musicin;

	int
	get_audio( int buffer[2][1152] );

	int read_samples_mp3( short int mpg123pcm[2][1152], int );

	int decode_initfile();

	boost::thread lame_thread;

};

#endif // _LAME_HPP_
