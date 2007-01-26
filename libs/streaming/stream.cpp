/* @(#)stream.cpp
 */


#include "stream.hpp"
#include "lame/lame.h"
#include "boost/filesystem/fstream.hpp"
//
#include <iostream>


//#include "hip/hip.h"


BOOST_DEFINE_LOG( strm, "strm" )

using namespace Streaming;

#define INPUT_BUFSIZ 8192000

//void decodeMP3(mpstr*, char*, int, char*, int, int*){}
void
Stop(){

}

void
error(const char*msg ){
	std::cerr << msg << std::endl;
}

Stream::Stream( Spinny::PlayList::ptr pl ) :
	mp3_data(""),
	pl_(pl)
{ 
//	lame_decode_init();
}

void
Stream::decode_song( Spinny::Song::ptr song ) {
	song->title();

// 	int eof=0;
// 	HIP_File hf;
// 	char pcmout[8000];
// 	int current_section;

// 	FILE *fp = fopen( "/mnt/snoopy/music/REM/Losing_My_Religion.mp3","r" );
// 	FILE *out = fopen( "/tmp/out.wav", "w" );

// 	if( hip_open(fp, &hf, NULL, 0) < 0) {
// 		fprintf(stderr,"Input does not appear to be an mpeg bitstream.\n");
// 		exit(1);
// 	}
// 	BOOST_LOGL(strm,info) << "Encoding " << song->path().string() << " - " << hf.header_parsed << " | " << hf.stereo << " " << hf.bitrate;
// 	while(!eof){
// 		long ret = hip_read( &hf, pcmout, sizeof(pcmout), 0, 2, 1, &current_section );

// 		if (ret == 0) {
// 			/* EOF */
// 			eof=1;
// 		} else if (ret < 0) {
// 			/* error in the stream.  Not a problem, just reporting it in
// 			   case we (the app) cares.  In this case, we notify since this
// 			   is a test application.
// 			*/
// 			fprintf(stderr,"Error in the stream %ld\n", ret);
// 		} else {
// 			fwrite(pcmout,1,ret,out);
// 		}
// 	}

// 	fclose( fp );
// 	fclose( out );

// 	hip_clear(&hf);


// 	struct mpstr mp;
// 	static char input_buf[INPUT_BUFSIZ];

//	int size;
//	char out[8192];
//	int ret;
// 	mp3_data = new char[INPUT_BUFSIZ];

// 	InitMP3(&mp);

//  	boost::filesystem::ifstream is;
//  	is.open( song->path(), std::ios::in | std::ios::binary);

//  	std::streamsize len = is.read( (char*)input_buf, sizeof(input_buf) ).gcount();

// 	BOOST_LOGL(strm,info) << "Encoded " << song->path().string() << " len " << len << " / " << sizeof(input_buf);

// 	mp3_data_len = decodeMP3(&mp,input_buf,len,mp3_data,8192,0);

// 	BOOST_LOGL(strm,info) << "Encoded " << song->path().string() << " len " << len << " / " << mp3_data_len;

	
	
// 	static char input_buf[INPUT_BUFSIZ];
// 	static int16_t left[INPUT_BUFSIZ * 45];
// 	static int16_t right[INPUT_BUFSIZ * 45];



// 	lame_global_flags *gfp;

// 	static char *out_buf;
// 	unsigned int out_buf_len=0;

// 	gfp = lame_init();

// 	lame_set_num_channels(gfp,2);
// 	lame_set_in_samplerate(gfp,44100);
// 	lame_set_brate(gfp,128);
// 	lame_set_mode(gfp,JOINT_STEREO);
// 	lame_set_quality(gfp,2);   /* 2=high  5 = medium  7=low */ 

// 	if ( lame_init_params(gfp) == -1 ){
// 		cerr << "Failed to set params" << endl;
// 	}
   

// 	boost::filesystem::ifstream is;
// 	is.open( song->path(), std::ios::in | std::ios::binary);

// 	std::streamsize len = is.read( (char*)input_buf, sizeof(input_buf)).gcount();

// 	unsigned int num_samples = lame_decode( (unsigned char*)input_buf, len, left, right);

// 	if ( out_buf_len < 1.25*num_samples + 7200 ){
// 		char *tmpbuf;
// 		if (!(tmpbuf = (char*)realloc( (void*)out_buf, out_buf_len ) ) ) {
// 			// FIXME - handle ERROR
// 		}
// 		out_buf = tmpbuf;
// 	}

// 	unsigned int olen = lame_encode_buffer ( gfp, left, right, num_samples, (unsigned char*)out_buf, out_buf_len );

// 	BOOST_LOGL(strm,info) << "Encoded " << song->path().string() << " len " << sizeof(input_buf) << "/" << len << " / " << num_samples << " to " << olen;
	
// 	mp3_data = out_buf;
// 	mp3_data_len=olen;
	
//	return asio::buffer( mpeg.rawdata, );

}



