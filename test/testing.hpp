
/* @(#)testing.hpp
 */

#ifndef _TESTING_H
#define _TESTING_H 1

#include <iostream>
#include <stdexcept>
#include "unit_test/test.h"
#include "boost/filesystem/operations.hpp"
#include "boost/noncopyable.hpp"
#include "sqlite/sqlite.hpp"
#include "spinny/spinny.hpp"
#include "boost/log/functions.hpp"

// evil I know... just couldn't help myself
using namespace std;
using namespace boost;

#define ARGS_SIZE 11
class DummyApp : boost::noncopyable {

public:
	boost::filesystem::path fixtures_path;
	boost::filesystem::path db_path;
	boost::filesystem::path music_path;
	boost::filesystem::path web_path;
	boost::filesystem::path template_path;

	DummyApp() :
		fixtures_path( TESTING_FIXTURES_PATH ),
		db_path( fixtures_path / "test.db" ),
		music_path( fixtures_path / "music" ),
		web_path( fixtures_path / "webroot" ),
		template_path( fixtures_path / "webtmpl" )
		{
			boost::filesystem::remove( db_path );
			boost::filesystem::create_directory( fixtures_path );
			boost::filesystem::create_directory( web_path );
			boost::filesystem::create_directory( template_path );
			boost::filesystem::create_directory( music_path );

			char const *args[ARGS_SIZE];
			args[0] = "program";
			args[1] = "--db";
			args[2] = db_path.string().c_str();
			args[3] = "--web_root";
			args[4] = web_path.string().c_str();
			args[5] = "--web_listen_address";
			args[6] = "127.0.0.1";
			args[7] = "--web_listen_port";
			args[8] = "3001";
			args[9] = "--template_root";
			args[10]= template_path.string().c_str();

			Spinny::run( ARGS_SIZE, const_cast<char**>(args) );
			con=sqlite::db();
			con->exec<sqlite::none>("create table testing( col1 int, col2 string, col3 int )");
		}

	void
	populate_web(){
		filesystem::directory_iterator end_itr;
		boost::filesystem::directory_iterator file(  filesystem::path( SRC_PATH ) / "libs" / "cs" / "test-files" );
		for (  file ;file != end_itr; ++file ){
			if ( file->leaf() != ".svn" ){
				try {
					boost::filesystem::copy_file( *file, template_path / file->leaf() );
				}
				catch ( std::exception &c ){}
			}
		}
	}

	void
	populate_music_fixtures(){
		filesystem::directory_iterator end_itr;
		boost::filesystem::directory_iterator file( filesystem::path( SRC_PATH ) / "libs" / "id3lib" / "test-files" );
		for (  file ;file != end_itr; ++file ){
			if ( file->leaf() != ".svn" ){
				boost::filesystem::copy_file( *file, music_path / file->leaf() );
			}
		}
	}

	~DummyApp() {
		Spinny::stop();
//		filesystem::remove_all( fixtures_path );
	}

	template<typename T>
	bool
	test_table_obj();

	sqlite::connection *con;
};


struct EnableLogging {

	EnableLogging( const std::string logs="*" ){
		boost::logging::manipulate_logs(logs)
 			.del_modifier("time")
 			.del_modifier("prefix")
 			.del_modifier("enter")
			.add_appender(&boost::logging::write_to_cout)    // all messages are written to cout
			.add_modifier(&boost::logging::prepend_prefix,"prefix" )
			.add_modifier( boost::logging::prepend_time("$yy$MM$dd $hh:$mm:$ss "), "time" )
			.add_modifier(&boost::logging::append_enter,"enter");
	}
};

template<typename T> inline
bool
DummyApp::test_table_obj(){
	T table;

	const sqlite::table::description *desc = table.m_table_description();
	std::vector< std::string > vals;
	*con << "insert into " << desc->table_name() << " (";
	for ( int i=0; i< desc->num_fields(); ++i ){
		if ( i>0 ){
			*con << ",";
		}
		*con << desc->fields()[i];
	}
	*con << ") values (";
	for ( int i=0; i< desc->num_fields(); ++i ){
		if ( i>0 ){
			*con << ",";
		}

		std::string val = boost::lexical_cast<string>( rand() );
		vals.push_back( val );
		*con << sqlite::q(val);
	}
	*con << ")";

	con->exec<sqlite::none>();
	sqlite::id_t new_id = con->insertid();
	boost::shared_ptr<T> ft = con->load<T>( new_id );

	ft->set_db_id( 0 );

	*con << "delete from " << desc->table_name() << " where rowid=" << new_id;
	con->exec<sqlite::none>();

	ft->save();
	new_id = ft->db_id();

	for ( int i=0; i< desc->num_fields(); ++i ){
		*con << "select " << desc->fields()[i] << " from " << desc->table_name() << " where rowid=" << new_id;
		std::string ret=con->exec<std::string>();
		if  ( vals[i] != ret ){
			std::string msg="Table: ";
			msg+=desc->table_name();
			msg+=" / Field: ";
			msg+=desc->fields()[i];
			msg+=" ";
			msg+=vals[i];
			msg+=" != ";
			msg+=ret;
			throw std::runtime_error( msg );
		}
	}
	return true;
}

#endif /* _TESTING_H */

