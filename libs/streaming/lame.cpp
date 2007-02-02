
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
	    BOOST_LOGL(strm,err ) << "Error: number of channels has changed in mp3 file - not supported";

    if ( lame_get_in_samplerate( lgf ) != mp3input_data.samplerate )
	    BOOST_LOGL(strm,err ) << "Error: sample frequency has changed in mp3 file - not supported";

    return out;
}



int
Lame::get_audio( int buffer[2][1152] )
{
	int     num_channels = lame_get_num_channels( lgf );

	short   buf_tmp16[2][1152];
	int     samples_read;
	int     i;

	samples_read = read_samples_mp3( buf_tmp16, num_channels );
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
Lame::transcode_song( Spinny::Song::ptr song )
{

	boost::mutex::scoped_lock lock(buffer_mutex);

	int     Buffer[2][1152];
	int     iread;


	std::string in_path = song->path().string();
			
	if ( (musicin = fopen(in_path.c_str(), "rb" ) ) == NULL ) {
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
		
	BOOST_LOGL( strm, info ) << "Encoding "  <<  lame_get_brate(lgf)
				 << "kbs " << mode_names[lame_get_force_ms(lgf)][lame_get_mode(lgf)] 
				 << " (qval=" << lame_get_quality(lgf) << ")"
				 << "\n" << in_path
				 << "\nTitle:   " << song->title() 
				 << "\nArtist:  " << song->artist()->name()
				 << "\nAlbum:   " << song->album()->name();

						 


	/* encode until we hit eof */
	do {

		while (  ! write_buffer && this->is_running() ){
			buffer_condition.wait(lock);
		}

		iread = get_audio( Buffer );

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
				BOOST_LOGL( strm, err ) << "mp3 buffer is not big enough";
			else
				BOOST_LOGL( strm, err ) << "mp3 internal error:  error code=" << write_buffer->data_length; 
			return false;
		}

		write_buffer = NULL;
		buffer_condition.notify_one();

	} while ( iread && this->is_running() );



	if (fclose(this->musicin) != 0) {
		BOOST_LOGL( strm, err ) << "Could not close audio input file";
		throw std::runtime_error("Could not close audio input file");
	}


	return true;



}







  

Lame::Lame(Spinny::PlayList::ptr p) :
	pl(p),
	lame_thread(0)
{
	BOOST_LOGL( strm, info ) << "New transcoder " << this;
	this->start();
}

bool
Lame::start(){
	return ( lame_thread=new boost::thread( boost::bind(&Lame::transcode,boost::ref(*this) ) ) );
}

bool
Lame::stop(){
	boost::thread *t=lame_thread;
	lame_thread=NULL;
	buffer_condition.notify_all();
	t->join();
	delete t;
	return true;
}

bool
Lame::is_running(){
	return lame_thread;
}

Lame::~Lame(){
	BOOST_LOGL( strm, info ) << "Transcoder exit " << this;
	this->stop();
}

void
log_debug( const char* fmt, va_list args ) {
	static char buff[300];
	snprintf( buff, 300, fmt, args );
	BOOST_LOGL( strm, debug ) << buff;
}
void
log_info( const char* fmt, va_list args ) {
	static char buff[300];
	snprintf( buff, 300, fmt, args );
	BOOST_LOGL( strm, info ) << buff;
}void
log_err( const char* fmt, va_list args ) {
	static char buff[300];
	snprintf( buff, 300, fmt, args );
	BOOST_LOGL( strm, err ) << buff;
}

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

	while ( this->is_running() ){
		Spinny::Song::result_set songs = pl->songs();
		for ( Spinny::Song::result_set::iterator song = songs.begin(); this->is_running() && song != songs.end(); ++song ){

			try {
				this->transcode_song( song.shared_ptr() );
			}
			catch ( const std::exception &ex ){
				BOOST_LOGL( strm, err ) << "Caught exception: " << ex.what() << " moving on to next file";
			}
		}
	}

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


	while ( write_buffer && this->is_running() ){
		buffer_condition.wait(lock);
	}

	write_buffer = read_buffer;
	read_buffer=read_buffer->next;
	buffer_condition.notify_one();

	return asio::buffer( read_buffer->data, read_buffer->data_length );
}


