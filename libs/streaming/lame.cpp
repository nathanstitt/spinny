

#include <assert.h>
#include <stdio.h>

#if defined(_WIN32)
# include <windows.h>
#endif

#define         MAX_U_32_NUM            0xFFFFFFFF

#include "streaming/lame.hpp"
#include "streaming/server.hpp"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/bind.hpp>
#include "spinny/album.hpp"
#include "spinny/artist.hpp"

using namespace Streaming;

static const int NUM_BUFFERS=3;


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
	unsigned int     len, aid_header=0;
	short int pcm_l[1152], pcm_r[1152];
	int freeformat = 0;

	memset(&mp3input_data, 0, sizeof(mp3data_struct));

	lame_decode_init();

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
Lame::fill_buffer( Buffer *buffer ){
	int read_buffer[2][1152];
	int iread=0;
	int encoded=0;
	buffer->data_length=0;
	do {
		if ( SongChanged == action_ ){
			BOOST_LOGL( strm, debug ) << "Song has changed to " << current_song_id_;
			iread = 0;
		} else {
			iread = get_audio( read_buffer );
		}
		if ( iread ){
			/* encode */
			encoded = lame_encode_buffer_int( lgf, read_buffer[0], read_buffer[1], iread,
							  buffer->data.c_array()+buffer->data_length,
							  LAME_MAXMP3BUFFER-buffer->data_length );
		} else {
// 			/* may return one more mp3 frame */
// 			encoded = lame_encode_flush_nogap( lgf, buffer->data.c_array()+buffer->data_length,
// 							   LAME_MAXMP3BUFFER-buffer->data_length );
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
		buffer->data_length += encoded;
	} while( ( buffer->data_length + encoded + 500 ) < LAME_MAXMP3BUFFER );

	BOOST_LOGL( strm, debug ) << "Filled buffer " << buffer << " with " << buffer->data_length << " / " << LAME_MAXMP3BUFFER << " bytes";

	return true;
}



Lame::Lame(Spinny::PlayList::ptr p) :
	pl_(p),
	current_pos_( pl_->size() ),
	musicin(0),
	lame_thread(0),
	action_( FillBuffer ),
	buffer_( new Buffer )
{
	buffer_->create();

	BOOST_LOGL( strm, info ) << "New transcoder " << this;

	lame_thread=new boost::thread( boost::bind(&Lame::transcode,boost::ref(*this) ) );
	

	for ( int i = 0 ; i <= NUM_BUFFERS; ++i ){

		boost::recursive_mutex::scoped_lock lock(buffer_mutex);

		while ( AwaitingRead != action_ ){
			buffer_condition.wait(lock);
		}

		action_= FillBuffer;
		buffer_condition.notify_one();
	}
}




Lame::~Lame(){
	{
		boost::recursive_mutex::scoped_lock lock(buffer_mutex);
		action_=Quitting;
		buffer_condition.notify_one();
	}


	lame_thread->join();

	delete lame_thread;

	buffer_->destroy();

	BOOST_LOGL( strm, info ) << "Transcoder exit " << this;
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

void
Lame::transcode(){
	BOOST_LOGL( strm, debug ) << __PRETTY_FUNCTION__ << " called";

//	write_buffer=new Buffer;
//	read_buffer->next=write_buffer;
//	write_buffer->next=read_buffer;
	

	/* initialize libmp3lame */

	if (NULL == ( lgf = lame_init())) {
		throw std::runtime_error("fatal error during initialization");
	}


	if( -1 == lame_set_num_channels( lgf, 2 ) ) {
		BOOST_LOGL( strm, err ) << "Unsupported number of channels: " << mp3input_data.stereo;
		throw std::runtime_error( "Unsupported number of channels" );
	}

	if ( -1 == lame_set_brate(lgf, pl_->bitrate() ) ){
		BOOST_LOGL( strm, err ) << "Unsupported bitrate: " << pl_->bitrate();
		throw std::runtime_error( "Unsupported bitrate" );
	}

	lame_set_errorf(lgf,log_err );
 	lame_set_debugf(lgf,log_debug);
 	lame_set_msgf(lgf,log_info);

	lame_set_quality(lgf,7);

	lame_init_params(lgf);

	this->select_song( 0 );
	Buffer *write_buffer = buffer_;

	// move the buffer back one
	buffer_ = buffer_->prev;

	while ( Quitting != action_ ){
		boost::recursive_mutex::scoped_lock lock(buffer_mutex);

		while ( Quitting != action_ && SongChanged != action_ &&  FillBuffer != action_ ) {
			buffer_condition.wait(lock);

		}

		try {
			if ( Quitting != action_ ){
				this->fill_buffer( write_buffer );
				write_buffer = write_buffer->next;
				action_ = AwaitingRead;
				buffer_condition.notify_one();
			}
		}
		catch ( const std::exception &ex ){
			BOOST_LOGL( strm, err ) << "Caught exception: " << ex.what() << " moving on to next file";
			this->next_song();
		}
	}
	fclose( musicin );

	lame_close(lgf);
}

Chunk
Lame::get_chunk(){
	boost::recursive_mutex::scoped_lock lock(buffer_mutex);
	while ( AwaitingRead != action_ ){
		buffer_condition.wait(lock);
	}
	action_= FillBuffer;
	buffer_ = buffer_->next;
	buffer_condition.notify_one();
//	BOOST_LOGL( strm, info ) << "Lame::get_chunk() " << this << " returned " << buffer_->data_length << " chunk from buffer " << buffer_;

	Chunk c( buffer_, pl_->bitrate() );

	return c;
}

std::list<asio::const_buffer>
Lame::history(){
	Buffer *begin = buffer_->next->next;
	Buffer *end = buffer_;
	std::list<asio::const_buffer> ret;
	while ( begin != end ){
		BOOST_LOGL( strm, info ) << "Lame::history() added " << begin;
		begin = begin->next;
		ret.push_back( asio::buffer( begin->data, begin->data_length ) );
	}
//	BOOST_LOGL( strm, info ) << "Lame::history() added " << buffer_;
//	ret.push_back( asio::buffer( buffer_->data, buffer_->data_length ) );
	
	BOOST_LOGL( strm, info ) << "Lame::history() " << this << " returned " << ret.size() << " chunks";
	return ret;
}
  
void
Lame::next_song(){
	++current_pos_;
	if ( current_pos_ >= pl_->size() ){
		current_pos_ = 0;
	}
	this->select_song( current_pos_ );
}

void
Lame::song_order_changed( sqlite::id_t song_id, unsigned int new_position ){
	BOOST_LOGL( strm, debug ) << " encoder on song " << current_song_id_ << " / " << song_id << " moved to " << new_position;

	if ( current_song_id_ == song_id ){
		current_pos_ = new_position;
	}
}

Spinny::Song::ptr
Lame::current_song(){
	return pl_->at( current_pos_ );
}

bool
Lame::select_song( Spinny::Song::ptr song ){
	BOOST_LOGL( strm, err ) << "Attempting to stream song " << song->title() << " ( " << song->db_id() << " )";

	Spinny::Song::result_set songs = pl_->songs();
	unsigned int pos = 0;
	for ( Spinny::Song::result_set::iterator s = songs.begin(); s != songs.end(); ++s ){
		if ( s->db_id() == song->db_id() ){
			BOOST_LOGL( strm, debug ) << "Selecting song at position: " << pos;
			{
				boost::recursive_mutex::scoped_lock lock(buffer_mutex);
				this->set_song( song );
				current_pos_=pos;
			}
			for ( int i = 0 ; i <= NUM_BUFFERS; ++i ){

				boost::recursive_mutex::scoped_lock lock(buffer_mutex);

				while ( AwaitingRead != action_ ){
					buffer_condition.wait(lock);
				}

				action_= FillBuffer;
				buffer_condition.notify_one();
			}

			BOOST_LOGL( strm, debug ) << "Done";
			return true;
		}
		++pos;
	}

	BOOST_LOGL( strm, err ) << "Tried to stream song " << song->title() << " ( " << song->db_id() 
				<< " ) on playlist " << pl_->db_id() << ", but it wasn't contained therein";
	return false;
}

void
Lame::select_song( unsigned int index ){
	Spinny::Song::ptr song = pl_->at( index );
	current_pos_=index;
	set_song( song );
}

void
Lame::set_song( Spinny::Song::ptr song ){
	current_song_id_ = song->db_id();
	std::string in_path = song->path().string();

	BOOST_LOGL( strm, debug ) << __PRETTY_FUNCTION__ << " " << song->title();
	if ( this->musicin && fclose( this->musicin ) ) {
		BOOST_LOGL( strm, err ) << "Could not close audio input file";
		throw std::runtime_error("Could not close audio input file");
	}
	
	BOOST_LOGL( strm, debug ) << __FILE__ << " : " << __LINE__;

	if ( ( musicin = fopen(in_path.c_str(), "rb" ) ) == NULL ) {
		BOOST_LOGL( strm, err ) << "Could not open " << in_path;
		throw std::runtime_error( "file open for read failed" );
	}

	BOOST_LOGL( strm, debug ) << "Opened input file " << musicin;
	
	if ( -1 == init_decode_file() ) {
		BOOST_LOGL( strm, err ) << "Error reading headers in mp3 input file " << in_path;
		throw std::runtime_error( "Error reading headers in mp3 input file" );
	}

// 	id3tag_init( lgf );
// 	id3tag_add_v2(lgf);
// 	id3tag_v2_only(lgf);
// 	id3tag_set_title( lgf, song->title().c_str() );
// 	id3tag_set_artist( lgf, song->artist()->name().c_str() );
// 	id3tag_set_album( lgf, song->album()->name().c_str() );
// 	id3tag_set_year( lgf, boost::lexical_cast<std::string>( song->year() ).c_str()  );

//	(void) lame_set_in_samplerate( lgf, mp3input_data.samplerate );
//	(void) lame_set_num_samples( lgf, mp3input_data.nsamp );

//	lame_init_bitstream(lgf);

// 	if ( lame_get_num_samples( lgf ) == MAX_U_32_NUM ) {

// 		double  flen = lame_get_file_size(in_path); /* try to figure out num_samples */
// 		if (flen >= 0) {

// 			/* try file size, assume 2 bytes per sample */
// 			if ( mp3input_data.bitrate > 0) {
// 				double  totalseconds =
// 					(flen * 8.0 / (1000.0 * mp3input_data.bitrate));
// 				unsigned long tmp_num_samples =
// 					(unsigned long) (totalseconds * lame_get_in_samplerate( lgf ));
				
// 				(void) lame_set_num_samples( lgf, tmp_num_samples );
// //				mp3input_data.nsamp = tmp_num_samples;
// 			} else {
// 				(void) lame_set_num_samples( lgf,
// 							     (unsigned long)(flen / (2 * lame_get_num_channels( lgf ))) );
// 			}

// 		}
// 	}
		

	BOOST_LOGL( strm, info ) << this<< " encoding "  <<  lame_get_brate(lgf)
				 << "kbs " << mode_names[lame_get_force_ms(lgf)][lame_get_mode(lgf)] 
				 << " (qval=" << lame_get_quality(lgf) << ")"
				 << "\n    " << in_path
				 << "\n    Title:   " << song->title() 
				 << "\n    Artist:  " << song->artist()->name()
				 << "\n    Album:   " << song->album()->name();


}


Lame::Buffer::Buffer() :
	prev( NULL ),
	next( NULL )
{
	BOOST_LOGL( strm, info ) << "new Lame::Buffer " <<  this;
}

void
Lame::Buffer::create(){
	Buffer *last = this;
	for ( int buf_num = 0 ; buf_num < NUM_BUFFERS; ++buf_num ){
		last->next = new Buffer;
		last->next->prev = last;
		last = last->next;
	}
	this->prev = last;
	last->next = this;
}

void
Lame::Buffer::destroy( int buf_num ){
	BOOST_LOGL( strm, info ) << "Destroying Lame::Buffer " <<  this << " buffer num = " << buf_num;
	if ( buf_num < NUM_BUFFERS ){
		this->next->destroy( buf_num+1 );
	}
	delete this;
}

Chunk::Chunk( Lame::Buffer *b,
	      unsigned short int br  ) : 
	data( asio::buffer( b->data, b->data_length ) ),
	bitrate( br ) 
{ }

unsigned int
Chunk::milliseconds() const {
	return ( this->size() * 1000 ) / bitrate / 125;
}

std::size_t
Chunk::size() const {
	return asio::buffer_size( data );
}
