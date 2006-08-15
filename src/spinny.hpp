/* @(#)spinny.hpp
 */

#ifndef _SPINNY_H
#define _SPINNY_H 1

#include "sqlite/sqlite.hpp"
#include "config.h"
#include <boost/program_options.hpp>

class Spinny {
	int _argc;
	char **_argv;
	boost::program_options::variables_map _vm;
	Spinny( int argc, char **argv );
public:
	class CmdLineEx {};

	static int run( int argc, char **argv );

	static Spinny* instance();

	static sqlite::connection* Spinny::db();

	template<typename T>
	void config( const std::string &name ){
		_vm[name].as<T>();
	}

};


#endif /* _SPINNY_H */

