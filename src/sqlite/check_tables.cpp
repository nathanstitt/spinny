/* @(#)check_tables.cpp
 */

#ifndef _POPULATE_DB_H
#define _POPULATE_DB_H 1

#include "sqlite.hpp"
#include <algorithm>

typedef std::list<sqlite::table_desc*> tables_list;

tables_list*
sqlite::register_db_table_check( sqlite::table_desc *table ) {

	static tables_list *tables = new tables_list;

	if ( table ){
		tables->push_back( table );
	}

	return tables;
}


void
sqlite::check_and_create_tables( connection &con ){

	tables_list* tables = register_db_table_check( NULL );

	command cmd( con, "SELECT name FROM sqlite_master WHERE type='table'" );

	for( tables_list::iterator table = tables->begin(); table != tables->end(); ++table ){
		const char **fields=(*table)->fields();
		const char **field_types=(*table)->field_types();

		if (  std::find( cmd.begin(), cmd.end(), (*table)->table_name() ) == cmd.end() ){
			con << "create table " << (*table)->table_name() << "(";
			for ( int i=0; i<(*table)->num_fields(); ++i ){
				if ( i > 0 ){
					con << ", ";
				}
				con << fields[i] << " " << field_types[i];
			}
			con << ")";
			con.exec<none>();
		}
	}
}



#endif /* _POPULATE_DB_H */

