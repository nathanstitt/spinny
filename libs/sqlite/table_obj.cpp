/* @(#)table_obj.cpp
 */


#include "sqlite/sqlite.hpp"
#include "sqlite/comma.hpp"

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

void
table::destroy(){
	if ( ! _id ){
		return;
	}
	connection *con = sqlite::db();
	*con << "delete from " << this->m_table_description()->table_name() << " where rowid=" << this->db_id();
	con->exec<none>();
	_id=0;
}

table::description::~description(){

}


table::description::description(){
	sqlite::register_db_table_check( this );
}

void
table::description::insert_fields( std::ostream &str ) const {
	sqlite::commas comma;
	for( int i=0; i < this->num_fields(); ++i ){
		str << comma << this->fields()[i];
	}
}


