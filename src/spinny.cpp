/* @(#)spinny.cpp
 */


#include "spinny.hpp"
#include <vector>
#include <boost/thread/tss.hpp>
#include <boost/log/functions.hpp>
#include <boost/filesystem/path.hpp>

using namespace boost;
using namespace std;

BOOST_DEFINE_LOG(app, "app")



boost::program_options::variables_map
parse_program_options(int ac, char* av[]);

void
close_n_delete_db( sqlite::connection *s ){
	s->close();
	delete s;
}

boost::thread_specific_ptr<sqlite::connection> _db( &close_n_delete_db );
static Spinny *_instance=0;


Spinny::Spinny(int argc, char **argv) :
	_argc( argc ),
	_argv( argv )
{
	_vm=parse_program_options(argc,argv);
	_db.reset( new sqlite::connection( _vm["db"].as<string>() ) );
}

Spinny*
Spinny::instance(){
	return _instance;
}

sqlite::connection*
Spinny::db(){
	sqlite::connection *conn=_db.get();
	if ( ! conn ){
		conn=new sqlite::connection( Spinny::instance()->_vm["db"].as<string>() );
		_db.reset( conn );
	}
	return conn;
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



	sqlite::check_and_create_tables( *Spinny::db() );

	return 0;
}


void
Spinny::stop(){
	sqlite::connection *con = _db.get();
	if ( con ){
		con->close();
 		delete con;
		_db.release();
	}
	delete _instance;
	_instance=0;
}
