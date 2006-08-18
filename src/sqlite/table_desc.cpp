/* @(#)table_desc.cpp
 */


#include "sqlite.hpp"

using namespace sqlite;

table::~table(){}

table::table() : _id( 0 ){ }

void
table::set_db_id( sqlite::id_t id ) {
	id=_id;
};

sqlite::id_t
table::db_id() const {
	return _id;
};

bool
table::save( sqlite::connection &con ){
	return con.save( *this );
}

bool
table::needs_saved(){
	return ( _id==0 );
}



table::description::~description(){

}


table::description::description(){
	sqlite::register_db_table_check( this );
}

void
table::description::insert_fields( std::ostream &str, bool ignore_first ) const {
	for( int i=ignore_first; i < this->num_fields(); ++i ){
		if ( i != 0 ) {
			str << ',';
		}
		str << this->fields()[i];
	}
}
