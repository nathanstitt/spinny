/* @(#)spinny.cpp
 */


#include "spinny.hpp"
#include <vector>

#include "boost/log/functions.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "ews/server.hpp"

using namespace boost;
using namespace std;

BOOST_DEFINE_LOG(app, "app")

static Spinny *_instance=0;
static ews::server *_ews=0;
static asio::thread *_web_thread=0;


boost::program_options::variables_map
parse_program_options(int ac, char* av[]);



Spinny::Spinny(int argc, char **argv) :
	_argc( argc ),
	_argv( argv )
{
	_vm=parse_program_options(argc,argv);
}

Spinny*
Spinny::instance(){
	return _instance;
}




int
Spinny::run(int argc, char **argv)
{
	boost::logging::manipulate_logs("*")
		.add_modifier( boost::logging::prepend_time("$yy$MM$dd $hh:$mm:$ss "), "time" );

	if ( boost::filesystem::path::default_name_check_writable() ) {
		boost::filesystem::path::default_name_check( boost::filesystem::native );
	}

	BOOST_LOGL(app, warn) << "Spinny starting up";

	if ( _instance != 0 ){
		throw( "Spinny::run called twice!" );
	}

	try {
		_instance=new Spinny( argc, argv );
	}

	catch( Spinny::CmdLineEx & ){
		return 0;
	}

	sqlite::startup( _instance->config<string>("db") );


	_ews = new ews::server( _instance->config<string>( "web_listen_address" ),
				_instance->config<string>( "web_listen_port" ),
				_instance->config<string>( "web_root" ) );

  	_web_thread = new asio::thread( boost::bind( &ews::server::run, _ews ) );


	return 0;
}


void
Spinny::stop(){
	sqlite::stop_db();

	delete _instance;
	_instance=0;

	_ews->stop();
	_web_thread->join();

	delete _ews;
	_ews=0;

	delete _web_thread;
	_web_thread=0;
}
