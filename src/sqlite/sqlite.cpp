/* @(#)sqlite.cpp
 */


#include "sqlite.hpp"

BOOST_DEFINE_LOG(sql, "sql")


namespace sqlite {
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


}
