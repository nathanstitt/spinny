/* @(#)check_tables.cpp
 */

#ifndef _POPULATE_DB_H
#define _POPULATE_DB_H 1

#include "sqlite.hpp"
#include <algorithm>


typedef std::list<sqlite::table::description*> tables_list;

tables_list*
sqlite::register_db_table_check( sqlite::table::description *table ) {

	static tables_list *tables = new tables_list;

	if ( table ){
		tables->push_back( table );
	}

	return tables;
}


void
sqlite::check_and_create_tables(){
	connection *con = sqlite::db();

	tables_list* tables = register_db_table_check( NULL );

	for( tables_list::iterator table = tables->begin(); table != tables->end(); ++table ){

		*con << "SELECT 1 FROM sqlite_master WHERE type='table' and name='"
		    << (*table)->table_name()
		    << "'";


		int res= con->exec<int>();

		if ( ! res ){
			const char **fields=(*table)->fields();
			const char **field_types=(*table)->field_types();

			*con << "create table " << (*table)->table_name() << "(";
			for ( int i=0; i<(*table)->num_fields(); ++i ){
				if ( i > 0 ){
					*con << ", ";
				}
				*con << fields[i] << " " << field_types[i];
			}
			*con << ")";

			con->exec<none>();
		}
		BOOST_LOGL(sql,info) << "Checking for table " << (*table)->table_name() << ( ( res ) ? " exists" : " doesn't exist, created" ); 
	}
}



#endif /* _POPULATE_DB_H */

