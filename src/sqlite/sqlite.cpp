/* @(#)sqlite.cpp
 */


#include "sqlite.hpp"

BOOST_DEFINE_LOG(sql, "sql")


namespace sqlite {
	int
	q( int arg ){
		return arg;
	}
	
	long long
	q( long long &arg ){
		return arg; 
	}

	std::string
	q( const std::string &val ){
		return std::string("'") + boost::replace_all_copy(val, "'","''" ) + std::string("'");
	}


}
