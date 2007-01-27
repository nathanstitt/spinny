
#include "testing.hpp"
#include "boost/filesystem/fstream.hpp"
#include <iostream>
#include "spinny/playlist.hpp"
#include "spinny/music_dir.hpp"
#include "streaming/lame.hpp"
using namespace std;

extern "C" {
#include "lame/lame.h"
}


#include <boost/thread/xtime.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <iostream>

const int BUF_SIZE = 2;
const int ITERS = 20;

boost::mutex io_mutex;

typedef int buff_t;


struct buffer_link {

	buff_t data;
	int buf_pos;
	buffer_link *next;

};
buffer_link *cur_link;


class buffer
{
public:
	typedef boost::mutex::scoped_lock
	scoped_lock;
    
	buffer()
		: write_buffer(0), p(0), c(0), full(0){


	cur_link=new buffer_link;
	cur_link->next=new buffer_link;
	cur_link->next->next = cur_link;
	write_buffer = cur_link;

	}
    
	void write(int m)	{
		scoped_lock lock(mutex);
	
		if ( ! write_buffer )
		{

			{	boost::mutex::scoped_lock lock(io_mutex);
				std::cout << "still writing. Waiting..." << std::endl; }

			while (  ! write_buffer )
				cond.wait(lock);
		}

			{	boost::mutex::scoped_lock lock(io_mutex);
				std::cout << "wrote to " << (int)write_buffer << std::endl; }

		write_buffer->data = m;
//		sleep( cur_link ->data );
		write_buffer = NULL;
		cond.notify_one();
	}

	int read(){
		scoped_lock lk(mutex);
		if ( write_buffer )
		{
			
			{	boost::mutex::scoped_lock lock(io_mutex);
				std::cout << "still reading. Waiting..." << std::endl; }

			while ( write_buffer ){
				boost::xtime xt;
				boost::xtime_get( &xt, TIME_UTC );
				xt.sec+=1;
				if ( ! cond.timed_wait(lk,xt) ){
					{	boost::mutex::scoped_lock lock(io_mutex);
						std::cout << "Timed Out!" << std::endl; }

				}
			}
		}

		write_buffer = cur_link;
 		cur_link=cur_link->next;
		cond.notify_one();

			{	boost::mutex::scoped_lock lock(io_mutex);
				std::cout << "read from " << (int)cur_link << std::endl; }

 		return cur_link->data;
	}
    
private:
	buffer_link *write_buffer;
	buffer_link *read_buffer;
	boost::mutex mutex;
	boost::condition cond;
	unsigned int p, c, full;
	int buf[BUF_SIZE];
};

buffer buf;

void writer()
{
	for (int n = 0; n < ITERS; ++n)
	{
		{       boost::mutex::scoped_lock lock(io_mutex);
			std::cout << "writing " << n << std::endl; }

		buf.write(n);
	}
}

void reader()
{
	for (int x = 0; x < ITERS; ++x)
	{
		int n = buf.read();
		{
		{       boost::mutex::scoped_lock lock(io_mutex);
			std::cout << "read " << n << std::endl; }
		}
	}
}
    


int
lame( int, char ** ) 
{

// 	boost::thread thrd1(&reader);
// 	boost::thread thrd2(&writer);
// 	thrd1.join();
// 	thrd2.join();
// 	return 0;

 	DummyApp da;
	EnableLogging el("lame");
//	da.populate_music_fixtures();
	
	Spinny::MusicDir::ptr md = Spinny::MusicDir::create_root( "/tmp/music" );
	md->sync();

	Spinny::Song::result_set songs = md->songs();
 	Spinny::PlayList::ptr pl = Spinny::PlayList::create( 128,  std::string("128 Kbs"), std::string("BooYah") );

	pl->save();

	for ( Spinny::Song::result_set::iterator s=songs.begin(); songs.end() != s; ++s ){
		pl->insert( s.shared_ptr(), 0 );
	}
	
	
   	boost::filesystem::ofstream os("/tmp/out.mp3", std::ios::out | std::ios::binary);
    	
// //   	std::cout  << "Encoded " << wav_buf_len;

	BOOST_LOGL( lame, info ) << "Test";
	{
		Lame l(pl);
		int read=0;
		while ( read < 1024*1024*2 ) {
			asio::const_buffer buf = l.get_chunk();
			read+= asio::buffer_size( buf );
			os.write( asio::buffer_cast<const char*>(buf), asio::buffer_size( buf ) );
			//	cout << "Encoded " << read << endl;
		}
		BOOST_LOGL(lame,info)<< "Out Loop";
	}
	BOOST_LOGL(lame,info)<< "shutdown";

	os.close();

}
