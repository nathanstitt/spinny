/* @(#)spinny.cpp
 */


#include "spinny.hpp"
#include <vector>
#include <algorithm>
#include "boost/bind.hpp"
#include "boost/log/functions.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/asio.hpp"
#include "spinny/settings.hpp"
#include "boost/bind.hpp"
#include "ews/server.hpp"
#include "spinny/music_dir.hpp"

using namespace std;
using namespace boost;
using namespace Spinny;

BOOST_DEFINE_LOG(app, "app")

boost::program_options::variables_map
parse_program_options(int ac, char* av[]);

namespace Spinny {


static App *_instance=0;
static ews::server *_ews=0;
static asio::thread *_web_thread=0;





App::App(int argc, char **argv) :
	_argc( argc ),
	_argv( argv )
{
	_vm=parse_program_options(argc,argv);
}

App*
App::instance(){
	return _instance;
}


void
init_music_dir( const std::string &dir ){
	MusicDir::ptr md=MusicDir::create_root(dir);
	md->sync();
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
	}

	catch( App::CmdLineEx & ){
		return 0;
	}

	sqlite::startup( _instance->config<string>("db") );

	_ews = new ews::server( _instance->config<string>( "web_listen_address" ),
				_instance->config<string>( "web_listen_port" ),
				_instance->config<string>( "web_root" ),
				_instance->config<string>( "template_root" )
		);


  	_web_thread = new asio::thread( boost::bind( &ews::server::run, _ews ) );

	vector<string> mds = _instance->config<vector<string> >( "music_dir" );
 	for_each( mds.begin(), mds.end(), boost::bind( init_music_dir, _1 ) );

	return 0;
}


void
App::stop(){
	sqlite::stop_db();

	delete _instance;
	_instance=0;

	_ews->stop();
	_web_thread->join();

	delete _ews;
	_ews=0;

	delete _web_thread;
	_web_thread=0;

	BOOST_LOGL(app, warn) << "App stopped";
}

} // namespace Spinny
