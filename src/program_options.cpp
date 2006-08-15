// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

/* Shows how to use both command line and config file. */

#include <boost/program_options.hpp>
#include "spinny.hpp"

namespace po = boost::program_options;


#include <iostream>
#include <fstream>
#include <iterator>
using namespace std;


#ifdef WINDOWS
static const char *default_file="spinny.cfg";
#else
static const char *default_file="/etc/spinny.conf";
#endif // WINDOWS

// A helper function to simplify the main part.
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
	copy(v.begin(), v.end(), ostream_iterator<T>(cout, " ")); 
	return os;
}


boost::program_options::variables_map
parse_program_options(int ac, char* av[])
{
       
        po::variables_map vm;
    
	po::options_description generic("Other options");
	generic.add_options()
		("db", "Directory to store music database in")
		;

        // Declare a group of options that will be 
        // allowed only on command line
        po::options_description cmd_line_opts("Command line options");
        cmd_line_opts.add_options()
		("version,v", "print version string")
		("help,h", "produce help message")
		("config,c","config file location")
		;
	cmd_line_opts.add( generic );
	po::store(po::parse_command_line(ac, av, cmd_line_opts), vm);

	if ( vm.count("help") ){
		std::cout << cmd_line_opts << "\n";
		throw Spinny::CmdLineEx();
	}


        po::options_description cfg_file_opts("Config file options");
	cfg_file_opts.add( generic );


	ifstream config_file;
        if ( vm.count("config")) {
		config_file.open( vm[ "config" ].as<const char*>() );
        } else {
		config_file.open( default_file );
	}

	po::store( parse_config_file( config_file, cfg_file_opts), vm);
    
	return vm;
}