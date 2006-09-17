/* @(#)sqlite.cpp
 */


#include "sqlite.hpp"
#include "boost/thread/tss.hpp"
#include "spinny.hpp"

BOOST_DEFINE_LOG(sql, "sql")

static std::string _default_db;

void
close_n_delete_db( sqlite::connection *s ){
	s->close();
	delete s;
}


boost::thread_specific_ptr<sqlite::connection> _db( &close_n_delete_db );



namespace sqlite {

	void startup( const std::string &location ){
		_default_db=location;
		check_and_create_tables();
	}



	connection*
	db(){
		connection *conn=_db.get();
		if ( ! conn ){
			conn=new sqlite::connection( _default_db );
			_db.reset( conn );
		}
		return conn;
	}


	int
	q( const int arg ){
		return arg;
	}
	
	const long long&
	q( const long long &arg ){
		return arg; 
	}

	//id_t&
	//q( id_t &arg ){
	//	return arg; 
	//}

	std::string
	q( const std::string &val ){
		return std::string("'") + boost::replace_all_copy(val, "'","''" ) + std::string("'");
	}


	void
	stop_db(){
		sqlite::connection *con = _db.get();
		if ( con ){
			con->close();
			delete con;
			_db.release();
		}
	}

}
