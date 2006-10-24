/* @(#)sqlite.cpp
 */


#include "sqlite.hpp"
#include "boost/thread/tss.hpp"
#include <algorithm>
#include <list>
#include "boost/bind.hpp"
#include "boost/thread/mutex.hpp"

BOOST_DEFINE_LOG(sql, "sql")

static std::string _default_db;

static std::list<sqlite::connection*> connections_;
static boost::mutex connections_mutex_;

static
void
close_n_delete_db( sqlite::connection *s ){
  	s->close();
  	delete s;
}

// static
// void
// close_delete_n_remove_db( sqlite::connection  ){
// 	boost::mutex::scoped_lock lock( connections_mutex_ );
//  	std::list<sqlite::connection*>::iterator con = std::find( connections_.begin(), connections_.end(), s );
//  	if ( connections_.end() != con ){
//  		connections_.erase( con );
//  	}
//  	close_n_delete_db( s );
// }

static boost::thread_specific_ptr<sqlite::connection> _db( &close_n_delete_db );


namespace sqlite {


	void startup( const std::string &location ){
		_default_db=location;
		check_and_create_tables();
	}



	connection*
	db(){
		connection *conn=_db.get();
		if ( ! conn ){
			boost::mutex::scoped_lock lock( connections_mutex_ );
			conn=new sqlite::connection( _default_db );
			_db.reset( conn );
			connections_.push_back( conn );
		}
		return conn;
	}

	int
	q( const int arg, bool ){
		return arg;
	}
	
 	const long long&
 	q( const long long &arg, bool ){
 		return arg; 
 	}

// 	id_t&
// 	q( id_t &arg ){
// 		return arg; 
// 	}

	std::string
	q( const std::string &val, bool enclose ){
		if ( enclose ){
			return std::string("'") + boost::replace_all_copy(val, "'","''" ) + std::string("'");
		} else {
			return boost::replace_all_copy(val, "'","''" );
		}
	}

	void
	stop_db(){
		sqlite::connection *con = _db.get();
		if ( con ){
			
			con->close();
			delete con;
			_db.release();
		}

// 		boost::mutex::scoped_lock lock( connections_mutex_ );
// 		std::for_each( connections_.begin(), connections_.end(),
// 			boost::bind( close_n_delete_db, _1 ) );
// 			connections_.clear();
	}

}
