/* @(#)table_desc.cpp
 */


#include "sqlite.hpp"

using namespace sqlite;


table_desc::table_desc(){
	sqlite::register_db_table_check( this );
}

