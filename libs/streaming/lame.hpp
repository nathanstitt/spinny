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
#include <string>


BOOST_DECLARE_LOG(lame);


class Lame {
public:
	Lame( Spinny::PlayList::ptr pl );

	asio::const_buffer next_block();

	bool transcode( const std::string &in_path, const std::string &out_path );

	bool doall( const std::string &out_path );

	bool to_buffers();

private:

	typedef boost::array<short int,4096> buff_t;
	struct buffer_link {
		buff_t buffer;
		buffer_link *next;
	};
	buffer_link *cur_link;

	int perform_encoding();

	Spinny::PlayList::ptr pl;

	//sound_file_format input_format;   
	int enc_delay;             /* if decoder finds a Xing header */ 
	int enc_padding;           /* if decoder finds a Xing header */ 
	unsigned int num_samples_read;

	mp3data_struct mp3input_data; /* used by MP3 */


	lame_global_flags *lgf;

	FILE   *musicin;
	FILE   *musicout;

	int
	get_audio( int buffer[2][1152] );

	int read_samples_mp3( short int mpg123pcm[2][1152], int );

	int decode_initfile();

};

#endif // _LAME_HPP_
