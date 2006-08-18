/* @(#)sqlite_testing.hpp
 */

#ifndef _SQLITE_TESTING_H
#define _SQLITE_TESTING_H 1

#include "sqlite.hpp"
#include "boost/filesystem/operations.hpp"

using namespace sqlite;
using namespace boost;
#define SQLITE_TEST_FILE "test.db"

struct FooDoggy {
	int age;
	std::string name;

	static inline
	const char* fields(){
		return "age, name";
	}

	static inline
	const char* table(){
		return "people_test";
	}
};

namespace sqlite{
	template <> inline
	FooDoggy 
	reader::get( int index ) const {
		FooDoggy f;
		f.age=this->get<int>(0);
		f.name=this->get<std::string>(1);
		return f;
	}
	template <> inline
	void
	command::bind( FooDoggy fd ){
		this->bind( 1, fd.age );
		this->bind<const std::string&>( 2, fd.name );
	};
}

struct Conn 
{
	Conn() : con( SQLITE_TEST_FILE ) {
		con << "create table foo( col1 int, col2 string )";
		con.exec<none>();
	}
	~Conn() {
		con.close();
		filesystem::remove( SQLITE_TEST_FILE );
	}
	connection con;
};



#endif /* _SQLITE_TESTING_H */

