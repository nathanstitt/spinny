/* @(#)table.cpp
 */


#include "sqlite.hpp"


using namespace sqlite;

table::~table(){}

table::table() : _id( 0 ){ }

void
table::set_db_id( sqlite::id_t id ) const {
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
table::needs_saved() const {
	return ( _id==0 );
}

bool
table::save_if_needed() const {
	if ( this->needs_saved() ){
		return this->save();
	} else {
		return true;
	}
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
