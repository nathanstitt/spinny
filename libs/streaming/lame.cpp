
# include "lame/config.h"

#include <assert.h>
#include <stdio.h>


#if defined(_WIN32)
# include <windows.h>
#endif

#define         MAX_U_32_NUM            0xFFFFFFFF

#include "streaming/lame.hpp"
#include "streaming/server.hpp"

#include <boost/thread/xtime.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/bind.hpp>
#include "spinny/album.hpp"
#include "spinny/artist.hpp"

using namespace Streaming;

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
Lame::init_decode_file()
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
	    
		BOOST_LOGL( strm, info ) << "ID3v2 found, be aware that the ID3 tag is currently lost when transcoding";

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

		BOOST_LOGL( strm, info ) << "Album ID found.  length=" << aid_header;

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
		BOOST_LOGL( strm, info ) << "Input file is freeformat.";
		freeformat = 1;
	}
	/* now parse the current buffer looking for MP3 headers.    */
	/* (as of 11/00: mpglib modified so that for the first frame where  */
	/* headers are parsed, no data will be decoded.   */
	/* However, for freeformat, we need to decode an entire frame, */
	/* so mp3data->bitrate will be 0 until we have decoded the first */
	/* frame.  Cannot decode first frame here because we are not */
	/* yet prepared to handle the output. */
	ret = lame_decode1_headers( buf, len, pcm_l, pcm_r, &mp3input_data );
	if (-1 == ret)
		return -1;

	/* repeat until we decode a valid mp3 header.  */
	while (!mp3input_data.header_parsed) {
		len = fread(buf, 1, sizeof(buf), musicin);
		if (len != sizeof(buf))
			return -1;
		ret = lame_decode1_headers( buf, len, pcm_l, pcm_r, &mp3input_data );
		if (-1 == ret)
			return -1;
	}

	if (mp3input_data.bitrate==0 && !freeformat) {
		BOOST_LOGL( strm, info ) << "fail to sync...";
		return init_decode_file();
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
Lame::get_audio( int buffer[2][1152] )
{
	int     num_channels = lame_get_num_channels( lgf );

	short   buf_tmp16[2][1152];
	int     samples_read;
	int     i;
 
	samples_read = lame_decode_fromfile( musicin, buf_tmp16[0], buf_tmp16[1],
					     &(mp3input_data));
	if ( samples_read < 0) {
		memset(buf_tmp16, 0, sizeof(**buf_tmp16) * 2 * 1152);
		return 0;
	}

	if ( lame_get_num_channels( lgf ) != mp3input_data.stereo )
		BOOST_LOGL(strm,err ) << "Error: number of channels has changed in mp3 file - not supported";

	if ( lame_get_in_samplerate( lgf ) != mp3input_data.samplerate )
		BOOST_LOGL(strm,err ) << "Error: sample frequency has changed in mp3 file - not supported";



	for( i = samples_read; --i >= 0; )
		buffer[0][i] = buf_tmp16[0][i] << (8 * sizeof(int) - 16);
	if( num_channels == 2 ) {
		for( i = samples_read; --i >= 0; )
			buffer[1][i] = buf_tmp16[1][i] << (8 * sizeof(int) - 16);
	} else if( num_channels == 1 ) {
		memset( buffer[1], 0, samples_read * sizeof(int) );
	}

	return samples_read;
}

static const char *mode_names[2][4] = {
	{"stereo", "j-stereo", "dual-ch", "single-ch"},
	{"stereo", "force-ms", "dual-ch", "single-ch"}
};


bool
Lame::fill_buffer(){
	int     Buffer[2][1152];
	int iread;
	int encoded=0;
	write_buffer->data_length=0;
	do {
		iread = get_audio( Buffer );
		if ( iread ){
			/* encode */
			encoded = lame_encode_buffer_int( lgf, Buffer[0], Buffer[1], iread,
							  write_buffer->data.c_array()+write_buffer->data_length,
							  LAME_MAXMP3BUFFER-write_buffer->data_length );
		} else {
			/* may return one more mp3 frame */
			encoded = lame_encode_flush_nogap( lgf, write_buffer->data.c_array()+write_buffer->data_length,
							   LAME_MAXMP3BUFFER-write_buffer->data_length );
			this->next_song();
		}
		/* was our output buffer big enough? */
		if ( encoded < 0) {
			if ( -1 == encoded  )
				BOOST_LOGL( strm, err ) << "mp3 buffer is not big enough";
			else
				BOOST_LOGL( strm, err ) << "mp3 internal error:  error code=" << encoded;
			throw std::runtime_error("Error filling buffer");
		}
		write_buffer->data_length += encoded;
	} while( ( write_buffer->data_length + encoded + 500 ) < LAME_MAXMP3BUFFER );

	BOOST_LOGL( strm, debug ) << "Filled buffer with " << write_buffer->data_length << " / " << LAME_MAXMP3BUFFER << " bytes";

	return true;
}



Lame::Lame(Spinny::PlayList::ptr p) :
	pl(p),
	cur_pos( pl->size() ),
	musicin(0),
	lame_thread(0),
	running_(true)
{
	BOOST_LOGL( strm, info ) << "New transcoder " << this;
	lame_thread=new boost::thread( boost::bind(&Lame::transcode,boost::ref(*this) ) );
}




Lame::~Lame(){
	BOOST_LOGL( strm, info ) << "Transcoder exit " << this;
	running_=false;
	buffer_condition.notify_all();
	lame_thread->join();
	delete lame_thread;
}

void
log_debug( const char* fmt, va_list args ) {
	static char buff[300];
	snprintf( buff, 300, fmt, args );
	BOOST_LOGL( strm, debug ) << " internal lame debug msg: " << buff;
}
void
log_info( const char* fmt, va_list args ) {
	static char buff[300];
	snprintf( buff, 300, fmt, args );
	BOOST_LOGL( strm, info ) << " internal lame info msg : " << buff;
}

void
log_err( const char* fmt, va_list args ) {
	static char buff[300];
	snprintf( buff, 300, fmt, args );
	BOOST_LOGL( strm, err ) << " internal lame error: " << buff;
}

bool
Lame::transcode(){
	BOOST_LOGL( strm, debug ) << __PRETTY_FUNCTION__ << " called";

	read_buffer=new buffer;
	write_buffer=new buffer;
	read_buffer->next=write_buffer;
	write_buffer->next=read_buffer;
	

	/* initialize libmp3lame */

	if (NULL == ( lgf = lame_init())) {
		throw std::runtime_error("fatal error during initialization");
	}


	if( -1 == lame_set_num_channels( lgf, 2 ) ) {
		BOOST_LOGL( strm, err ) << "Unsupported number of channels: " << mp3input_data.stereo;
		throw std::runtime_error( "Unsupported number of channels" );
	}

	if ( -1 == lame_set_brate(lgf, pl->bitrate() ) ){
		BOOST_LOGL( strm, err ) << "Unsupported bitrate: " << pl->bitrate();
		throw std::runtime_error( "Unsupported bitrate" );

	}

	lame_set_errorf(lgf,log_err );
 	lame_set_debugf(lgf,log_debug);
 	lame_set_msgf(lgf,log_info);

	lame_set_quality(lgf,7);

	lame_init_params(lgf);

	BOOST_LOGL( strm, debug ) << __PRETTY_FUNCTION__ << " init portion finished-" << running_;

	this->next_song();

	while ( running_ ){
		boost::mutex::scoped_lock lock(buffer_mutex);

		while (  ! write_buffer && running_ ){
			buffer_condition.wait(lock);
		}

		try {
			this->fill_buffer();
			write_buffer = NULL;
			buffer_condition.notify_one();
		}
		catch ( const std::exception &ex ){
			BOOST_LOGL( strm, err ) << "Caught exception: " << ex.what() << " moving on to next file";
			this->next_song();
		}
	}

	lame_close(lgf);

	delete read_buffer;
	delete write_buffer;

	return true;

}

void
Lame::next_song(){
	unsigned int size = pl->size();
	++cur_pos;

	if ( this->cur_pos > size ){
		this->cur_pos = 0;
	}
	Spinny::Song::ptr song = pl->at( cur_pos );
	std::string in_path = song->path().string();

	BOOST_LOGL( strm, debug ) << __PRETTY_FUNCTION__ << " " << song->title();
	
	
	if ( this->musicin && fclose( this->musicin ) ) {
		BOOST_LOGL( strm, err ) << "Could not close audio input file";
		throw std::runtime_error("Could not close audio input file");
	}
	
	if ( ( musicin = fopen(in_path.c_str(), "rb" ) ) == NULL ) {
		BOOST_LOGL( strm, err ) << "Could not open " << in_path;
		throw std::runtime_error( "file open for read failed" );
	}

	
	if (-1 == init_decode_file() ) {
		BOOST_LOGL( strm, err ) << "Error reading headers in mp3 input file " << in_path;
		throw std::runtime_error( "Error reading headers in mp3 input file" );
	}

	id3tag_init( lgf );
	id3tag_add_v2(lgf);
	id3tag_v2_only(lgf);
	id3tag_set_title( lgf, song->title().c_str() );
	id3tag_set_artist( lgf, song->artist()->name().c_str() );
	id3tag_set_album( lgf, song->album()->name().c_str() );
	id3tag_set_year( lgf, boost::lexical_cast<std::string>( song->year() ).c_str()  );


	(void) lame_set_in_samplerate( lgf, mp3input_data.samplerate );
	(void) lame_set_num_samples( lgf, mp3input_data.nsamp );

	lame_init_bitstream(lgf);

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
		
	BOOST_LOGL( strm, info ) << this<< " encoding "  <<  lame_get_brate(lgf)
				 << "kbs " << mode_names[lame_get_force_ms(lgf)][lame_get_mode(lgf)] 
				 << " (qval=" << lame_get_quality(lgf) << ")"
				 << "\n    " << in_path
				 << "\n    Title:   " << song->title() 
				 << "\n    Artist:  " << song->artist()->name()
				 << "\n    Album:   " << song->album()->name();


}
/*

fire off thread to encode next block, returning current (already encoded block)

 */


Chunk
Lame::get_chunk(){
	boost::mutex::scoped_lock lock(buffer_mutex);

	while ( write_buffer && running_ ){
		buffer_condition.wait(lock);
	}

	write_buffer = read_buffer;
	read_buffer=read_buffer->next;
	buffer_condition.notify_one();
//	BOOST_LOGL( strm, info ) << __PRETTY_FUNCTION__ << " " <<  this << " returned " << read_buffer->data_length << " chunk";
	Chunk c( read_buffer, pl->bitrate() );
	return c;
}

Chunk::Chunk( Lame::buffer *b,
	      unsigned short int br  ) : 
	data( asio::buffer( b->data, b->data_length ) ),
	bitrate( br ) 
{ }

unsigned int
Chunk::milliseconds(){
	return ( asio::buffer_size( data ) * 1000 ) / bitrate / 125;
}

std::size_t
Chunk::size(){
	return asio::buffer_size( data );
}
