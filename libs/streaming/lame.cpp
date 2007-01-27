/*
 *      Command line frontend program
 *
 *      Copyright (c) 1999 Mark Taylor
 *                    2000 Takehiro TOMINAGA
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* $Id: main.c,v 1.92.2.1 2005/12/18 18:49:29 robert Exp $ */

# include "lame/config.h"

#include <assert.h>
#include <stdio.h>

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char   *strchr(), *strrchr();
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#include <sys/stat.h>

#ifdef __sun__
/* woraround for SunOS 4.x, it has SEEK_* defined here */
#include <unistd.h>
#endif

#if defined(_WIN32)
# include <windows.h>
#endif

#define         MAX_U_32_NUM            0xFFFFFFFF

#include "lame.hpp"

/* PLL 14/04/2000 */
#if macintosh
#include <console.h>
#endif

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif


#include <boost/thread/xtime.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/bind.hpp>

BOOST_DEFINE_LOG( lame, "lame" )

int
is_syncword_mp123( const void *const headerptr)
{
	const unsigned char *const p = (const unsigned char *const)headerptr;
	static const char abl2[16] =
		{ 0, 7, 7, 7, 0, 7, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8 };

	if ((p[0] & 0xFF) != 0xFF)
		return 0;       /* first 8 bits must be '1' */
	if ((p[1] & 0xE0) != 0xE0)
		return 0;       /* next 3 bits are also */
	if ((p[1] & 0x18) == 0x08)
		return 0;       /* no MPEG-1, -2 or -2.5 */        
	if ((p[1] & 0x06) == 0x00)
		return 0;       /* no Layer I, II and III */
#ifndef USE_LAYER_1
	if ((p[1] & 0x06) == 0x03*2)
		return 0; /* layer1 is not supported */
#endif
#ifndef USE_LAYER_2
	if ((p[1] & 0x06) == 0x02*2)
		return 0; /* layer1 is not supported */
#endif
	if (! (p[1] & 0x06) == 0x01*2 )
		return 0; /* imcompatible layer with input file format */
	if ((p[2] & 0xF0) == 0xF0)
		return 0;       /* bad bitrate */
	if ((p[2] & 0x0C) == 0x0C)
		return 0;       /* no sample frequency with (32,44.1,48)/(1,2,4)     */
	if ((p[1] & 0x18) == 0x18 && (p[1] & 0x06) == 0x04
	    && abl2[p[2] >> 4] & (1 << (p[3] >> 6)))
		return 0;
	if ((p[3] & 3) == 2)
		return 0;       /* reserved enphasis mode */
	return 1;
}


off_t  lame_get_file_size( const std::string &filename )
{
	struct stat sb;

	if ( 0 == stat ( filename.c_str(), &sb ) )
		return sb.st_size;
	return (off_t) -1;
}



int
lame_decode_fromfile(FILE * fd, short pcm_l[], short pcm_r[],
                     mp3data_struct * mp3data)
{
    int     ret = 0, len=0;
    unsigned char buf[1024];

    /* first see if we still have data buffered in the decoder: */
    ret = lame_decode1_headers(buf, len, pcm_l, pcm_r, mp3data);
    if (ret!=0) return ret;


    /* read until we get a valid output frame */
    while (1) {
        len = fread(buf, 1, 1024, fd);
        if (len == 0) {
	    /* we are done reading the file, but check for buffered data */
	    ret = lame_decode1_headers(buf, len, pcm_l, pcm_r, mp3data);
	    if (ret<=0) {
                lame_decode_exit(); /* release mp3decoder memory */
                return -1;  /* done with file */
            }
	    break;
	}

        ret = lame_decode1_headers(buf, len, pcm_l, pcm_r, mp3data);
        if (ret == -1) {
            lame_decode_exit();  /* release mp3decoder memory */
            return -1;
        }
	if (ret >0) break;
    }
    return ret;
}


int
Lame::decode_initfile()
{
	/*  VBRTAGDATA pTagData; */
	/* int xing_header,len2,num_frames; */
	unsigned char buf[100];
	int     ret;
	unsigned int     len, aid_header;
	short int pcm_l[1152], pcm_r[1152];
	int freeformat = 0;

	memset(&mp3input_data, 0, sizeof(mp3data_struct));
	lame_decode_init();

    
	if ( fread(buf, 1, len, musicin) != len )
		return -1;      /* failed */
	if (buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3') {
	    
		BOOST_LOGL( lame, info ) << "ID3v2 found, be aware that the ID3 tag is currently lost when transcoding";

		len = 6;
		if (fread(&buf, 1, len, musicin) != len)
			return -1;      /* failed */
		buf[2] &= 127; buf[3] &= 127; buf[4] &= 127; buf[5] &= 127;
		len = (((((buf[2] << 7) + buf[3]) << 7) + buf[4]) << 7) + buf[5];

		fseek(musicin, len, SEEK_CUR);

		len = 4;
		if (fread(&buf, 1, len, musicin) != len)
			return -1;      /* failed */
	}
	aid_header = ( 0 == memcmp(buf, "AiD\1", 4) );

	if (aid_header) {
		if (fread(&buf, 1, 2, musicin) != 2)
			return -1;  /* failed */
		aid_header = (unsigned char) buf[0] + 256 * (unsigned char) buf[1];

		BOOST_LOGL( lame, info ) << "Album ID found.  length=" << aid_header;

		/* skip rest of AID, except for 6 bytes we have already read */
		fseek(musicin, aid_header - 6, SEEK_CUR);

		/* read 4 more bytes to set up buffer for MP3 header check */
		if (fread(&buf, 1, len, musicin) != len)
			return -1;      /* failed */
	}
	len = 4;
	while (!is_syncword_mp123(buf)) {
		unsigned int     i;
		for (i = 0; i < len - 1; i++)
			buf[i] = buf[i + 1];
		if (fread(buf + len - 1, 1, 1, musicin) != 1)
			return -1;  /* failed */
	}

	if ((buf[2] & 0xf0)==0) {
		BOOST_LOGL( lame, info ) << "Input file is freeformat.";
		freeformat = 1;
	}
	/* now parse the current buffer looking for MP3 headers.    */
	/* (as of 11/00: mpglib modified so that for the first frame where  */
	/* headers are parsed, no data will be decoded.   */
	/* However, for freeformat, we need to decode an entire frame, */
	/* so mp3data->bitrate will be 0 until we have decoded the first */
	/* frame.  Cannot decode first frame here because we are not */
	/* yet prepared to handle the output. */
	ret = lame_decode1_headersB( buf, len, pcm_l, pcm_r, &mp3input_data,&(enc_delay),&(enc_padding));
	if (-1 == ret)
		return -1;

	/* repeat until we decode a valid mp3 header.  */
	while (!mp3input_data.header_parsed) {
		len = fread(buf, 1, sizeof(buf), musicin);
		if (len != sizeof(buf))
			return -1;
		ret = lame_decode1_headersB( buf, len, pcm_l, pcm_r, &mp3input_data,&(enc_delay),&(enc_padding) );
		if (-1 == ret)
			return -1;
	}

	if (mp3input_data.bitrate==0 && !freeformat) {
		BOOST_LOGL( lame, info ) << "fail to sync...";
		return decode_initfile();
	}

	if ( mp3input_data.totalframes > 0) {
		/* mpglib found a Xing VBR header and computed nsamp & totalframes */
	}
	else {
		/* set as unknown.  Later, we will take a guess based on file size
		 * ant bitrate */
		mp3input_data.nsamp = MAX_U_32_NUM;
	}

	return 0;
}

int
Lame::read_samples_mp3( short int mpg123pcm[2][1152], int )
{
    int     out;

    out = lame_decode_fromfile( musicin, mpg123pcm[0], mpg123pcm[1],
			       &(mp3input_data));

    if (out < 0) {
        memset(mpg123pcm, 0, sizeof(**mpg123pcm) * 2 * 1152);
        return 0;
    }

    if ( lame_get_num_channels( lgf ) != mp3input_data.stereo )
	    BOOST_LOGL(lame,err ) << "Error: number of channels has changed in mp3 file - not supported";

    if ( lame_get_in_samplerate( lgf ) != mp3input_data.samplerate )
	    BOOST_LOGL(lame,err ) << "Error: sample frequency has changed in mp3 file - not supported";

    return out;
}



int
Lame::get_audio( int buffer[2][1152] )
{
	int     num_channels = lame_get_num_channels( lgf );

	short   buf_tmp16[2][1152];
	int     samples_read;
	unsigned int     framesize;
	int     samples_to_read;
//	unsigned int tmp_num_samples;
	int     i;


	/* 
	 * NOTE: LAME can now handle arbritray size input data packets,
	 * so there is no reason to read the input data in chuncks of
	 * size "framesize".  EXCEPT:  the LAME graphical frame analyzer 
	 * will get out of sync if we read more than framesize worth of data.
	 */

	samples_to_read = framesize = lame_get_framesize(lgf);
	assert(framesize <= 1152);

	/* get num_samples */
//	tmp_num_samples = lame_get_num_samples( lgf );


	samples_read = read_samples_mp3( buf_tmp16, num_channels );
	for( i = samples_read; --i >= 0; )
		buffer[0][i] = buf_tmp16[0][i] << (8 * sizeof(int) - 16);
	if( num_channels == 2 ) {
		for( i = samples_read; --i >= 0; )
			buffer[1][i] = buf_tmp16[1][i] << (8 * sizeof(int) - 16);
	} else if( num_channels == 1 ) {
		memset( buffer[1], 0, samples_read * sizeof(int) );
	} else
		assert(0);





	return samples_read;
}


bool
Lame::transcode_song()
{
	boost::mutex::scoped_lock lock(buffer_mutex);

	int     Buffer[2][1152];
	int     iread;
//	int     frames;

	/* encode until we hit eof */
	do {

		while (  ! write_buffer && running ){
			buffer_condition.wait(lock);
		}

		/* read in 'iread' samples */
		iread = get_audio( Buffer );
//		frames = lame_get_frameNum(lgf);

		if (iread){
			/* encode */
			write_buffer->data_length = lame_encode_buffer_int(lgf, Buffer[0], Buffer[1], iread,
								   write_buffer->data.c_array(), LAME_MAXMP3BUFFER );
		} else {
			/* may return one more mp3 frame */
			write_buffer->data_length = lame_encode_flush_nogap( lgf,
									     write_buffer->data.c_array(),
									     LAME_MAXMP3BUFFER );
		}
		/* was our output buffer big enough? */
		if ( write_buffer->data_length < 0) {
			if ( -1 == write_buffer->data_length  )
				BOOST_LOGL( lame, err ) << "mp3 buffer is not big enough";
			else
				BOOST_LOGL( lame, err ) << "mp3 internal error:  error code=" << write_buffer->data_length; 
			return false;
		}

		write_buffer = NULL;
		buffer_condition.notify_one();

	} while ( iread && running );

	return true;
}







  

Lame::Lame(Spinny::PlayList::ptr p) :
	pl(p),
	running( true ),
	lame_thread( boost::bind(&Lame::transcode,boost::ref(*this) ) )
{


	
}

Lame::~Lame(){
	BOOST_LOGL(lame,info)<< "stopping transcoder";
	running=false;
	buffer_condition.notify_all();
	lame_thread.join();
}

bool
Lame::to_buffers(){


	return true;
}

static const char *mode_names[2][4] = {
	{"stereo", "j-stereo", "dual-ch", "single-ch"},
	{"stereo", "force-ms", "dual-ch", "single-ch"}
};

bool
Lame::transcode(){

	read_buffer=new buffer;
	write_buffer=new buffer;
	read_buffer->next=write_buffer;
	write_buffer->next=read_buffer;
	

#if defined(_WIN32)
	/* set affinity back to all CPUs.  Fix for EAC/lame on SMP systems from
	   "Todd Richmond" <todd.richmond@openwave.com> */
	typedef BOOL (WINAPI *SPAMFunc)(HANDLE, DWORD);
	SPAMFunc func;
	SYSTEM_INFO si;

	if ((func = (SPAMFunc)GetProcAddress(GetModuleHandle("KERNEL32.DLL"),
					     "SetProcessAffinityMask")) != NULL) {
		GetSystemInfo(&si);
		func(GetCurrentProcess(), si.dwActiveProcessorMask);
	}
#endif


	/* initialize libmp3lame */

	if (NULL == ( lgf = lame_init())) {
		throw std::runtime_error("fatal error during initialization");
	}

	lame_init_params(lgf);

	if( -1 == lame_set_num_channels( lgf, 2 ) ) {
		BOOST_LOGL( lame, err ) << "Unsupported number of channels: " << mp3input_data.stereo;
		throw std::runtime_error( "Unsupported number of channels" );
	}


	while ( running ){
		Spinny::Song::result_set songs = pl->songs();
		for ( Spinny::Song::result_set::iterator song = songs.begin(); running && song != songs.end(); ++song ){
			std::string in_path = song->path().string();
			if ( (musicin = fopen(in_path.c_str(), "rb" ) ) == NULL ) {
				BOOST_LOGL( lame, err ) << "Could not open " << in_path;
				throw std::runtime_error( "file open for read failed" );
			}
	
			if (-1 == decode_initfile() ) {
				BOOST_LOGL( lame, err ) << "Error reading headers in mp3 input file " << in_path;
				throw std::runtime_error( "Error reading headers in mp3 input file" );
			}

			(void) lame_set_in_samplerate( lgf, mp3input_data.samplerate );
			(void) lame_set_num_samples( lgf, mp3input_data.nsamp );

			if ( lame_get_num_samples( lgf ) == MAX_U_32_NUM ) {

				double  flen = lame_get_file_size(in_path); /* try to figure out num_samples */
				if (flen >= 0) {
					/* try file size, assume 2 bytes per sample */
					if ( mp3input_data.bitrate > 0) {
						double  totalseconds =
							(flen * 8.0 / (1000.0 * mp3input_data.bitrate));
						unsigned long tmp_num_samples =
							(unsigned long) (totalseconds * lame_get_in_samplerate( lgf ));
				
						(void) lame_set_num_samples( lgf, tmp_num_samples );
						mp3input_data.nsamp = tmp_num_samples;
					} else {
						(void) lame_set_num_samples( lgf,
									     (unsigned long)(flen / (2 * lame_get_num_channels( lgf ))) );
					}
				}
			}
		
			BOOST_LOGL( lame, info ) << "Encoding " << in_path
						 << " as " <<  lame_get_brate(lgf)
						 << "kbs " << mode_names[lame_get_force_ms(lgf)][lame_get_mode(lgf)] 
						 << " (qval=" << lame_get_quality(lgf) << ")";

			this->transcode_song();

			if (fclose(this->musicin) != 0) {
				BOOST_LOGL( lame, err ) << "Could not close audio input file";
				throw std::runtime_error("Could not close audio input file");
			}

			lame_init_bitstream(lgf);
		}
	}
	BOOST_LOGL( lame, info ) << "Encoding completed";

//	lame_mp3_tags_fid(lgf, musicout); /* add VBR tags to mp3 file */

//	fclose(musicout); /* close the output file */

	lame_close(lgf);

	return true;

}

/*

fire off thread to encode next block, returning current (already encoded block)

 */

asio::const_buffer
Lame::get_chunk(){
	
	boost::mutex::scoped_lock lock(buffer_mutex);


	while ( write_buffer && running ){
		buffer_condition.wait(lock);
	}

	write_buffer = read_buffer;
	read_buffer=read_buffer->next;
	buffer_condition.notify_one();

	return asio::buffer( read_buffer->data, read_buffer->data_length );
}


