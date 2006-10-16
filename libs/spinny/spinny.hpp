/* @(#)spinny.hpp
 */

#ifndef _SPINNY_H
#define _SPINNY_H 1

#include "sqlite/sqlite.hpp"
#include "config.h"
#include <boost/program_options.hpp>
#include <boost/log/log.hpp>

BOOST_DECLARE_LOG(app)


class Spinny {
	int _argc;
	char **_argv;
	boost::program_options::variables_map _vm;
	Spinny( int argc, char **argv );
	void startup_threads();
	void stop_threads();
public:
	class CmdLineEx {};

	static int run( int argc, char **argv );

	static void stop();

	static Spinny* instance();

	template<typename T>
 	const T& config( const std::string &name ) const {
		return _vm[name].as<T>(); 
	}

};


#endif /* _SPINNY_H */


