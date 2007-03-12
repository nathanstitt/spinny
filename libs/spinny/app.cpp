/* @(#)app.cpp
 */


#include "spinny/app.hpp"

#include <vector>
#include <algorithm>
#include "boost/bind.hpp"
#include "boost/log/functions.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/asio.hpp"
#include "spinny/settings.hpp"
#include "boost/bind.hpp"
#include "ews/server.hpp"
#include "streaming/server.hpp"
#include "spinny/music_dir.hpp"

using namespace std;
using namespace boost;
using namespace Spinny;

BOOST_DEFINE_LOG(app, "app")

boost::program_options::variables_map
parse_program_options(int ac, char* av[]);

namespace Spinny {


static App *_instance=0;
//static ews::server *_ews=0;
//static Streaming::Server *_strm=0;
static asio::thread *_web_thread=0;
//static asio::thread *_strm_thread=0;

void
init_music_dir( const std::string &dir ){
	BOOST_LOGL( app, debug ) << "Scanning directory " << dir;
	MusicDir::ptr md=MusicDir::create_root(dir);
	md->sync();
}


App::App(int argc, char **argv) :
	_argc( argc ),
	_argv( argv )
{
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

	_vm=parse_program_options(argc,argv);

	
	sqlite::startup( this->config<string>("db") );

	BOOST_LOGL( app, debug ) << "Starting Streaming";

 	streaming = new Streaming::Server(  this->config<string>( "streaming_listen_address" ),
					    this->config<unsigned int>( "streaming_listen_port" )
 		);


	BOOST_LOGL( app, debug ) << "Streaming started successfully, now starting www";

	www = new ews::server( this->config<string>( "web_listen_address" ),
				this->config<string>( "web_listen_port" ),
				this->config<string>( "web_root" ),
				this->config<string>( "template_root" )
		);

  	_web_thread =  new asio::thread( boost::bind( &ews::server::run, this->www ) );

	BOOST_LOGL( app, debug ) << "WWW started successfully";
}

void
App::rescan(){
	BOOST_LOGL( app, info ) << "re-scanning directories";

	vector<string> mds = this->config<vector<string> >( "music_dir" );

 	for_each( mds.begin(), mds.end(), boost::bind( init_music_dir, _1 ) );
}

App*
App::instance(){
	return _instance;
}




int
App::run(int argc, char **argv)
{
	Settings::link_up();

	boost::logging::manipulate_logs("*")
		.add_modifier( boost::logging::prepend_time("$yy$MM$dd $hh:$mm:$ss "), "time" );

	if ( boost::filesystem::path::default_name_check_writable() ) {
		boost::filesystem::path::default_name_check( boost::filesystem::native );
	}

	BOOST_LOGL(app, warn) << "App starting up";

	if ( _instance != 0 ){
		throw( "App::run called twice!" );
	}

	try {
		_instance=new App( argc, argv );
		BOOST_LOGL(app,info) << "Started spinny @ " << _instance;
		_instance->rescan();
	}

	catch( App::CmdLineEx & ){
		return 0;
	}


	return 0;
}

// bool
// App::add_streaming_client( Spinny::PlayList::ptr pl, boost::shared_ptr<asio::ip::tcp::socket> socket ){

// 	if ( ! pl->size() ){
// 		BOOST_LOGL(app,info)<< "NOT adding streaming client "
// 				    << socket->remote_endpoint().address().to_string()
// 				    << " as playlist is empty";
// 		return false;
// 	}

// 	BOOST_LOGL(app,info)<< "Adding streaming client "
// 			    << socket->remote_endpoint().address().to_string();

// 	Streaming::Connection *ptr = new Streaming::Connection( socket );

// 	boost::shared_ptr<Streaming::Connection> ref( ptr );

// 	return _strm->add_client( pl, ref );
// }


App::~App(){
	BOOST_LOGL(app, info ) << "Stopping app";
	www->stop();
	_web_thread->join();

	BOOST_LOGL(app, info ) << "Stopping app";

	delete _web_thread;

	BOOST_LOGL(app, info ) << "Stopping app";
	delete www;
	BOOST_LOGL(app, info ) << "Stopping app";
	delete streaming;
	BOOST_LOGL(app, info ) << "Stopping app";

	BOOST_LOGL(app, warn) << "App stopped";
}
void
App::stop(){
	sqlite::stop_db();

	delete _instance;
	_instance=0;

//	_ews->stop();
//	_strm->stop();


//	_strm_thread->join();

//	delete _web_thread;
//	delete _strm_thread;
//	_web_thread=0;
//	_strm_thread=0;

}

} // namespace Spinny
