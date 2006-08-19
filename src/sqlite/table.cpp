/* @(#)table.cpp
 */


#include "sqlite.hpp"
#include <typeinfo>

using namespace sqlite;

table::~table(){}

table::table() : _id( 0 ){ }

void
table::set_db_id( sqlite::id_t id ) {
	_id=id;
};

sqlite::id_t
table::db_id() const {
	return _id;
};


bool
table::operator == ( const table &other ) const {
	return  ( this->db_id() == other.db_id() );
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
table::description::insert_fields( std::ostream &str ) const {
	for( int i=0; i < this->num_fields(); ++i ){
		if ( i != 0 ) {
			str << ',';
		}
		str << this->fields()[i];
	}
}
