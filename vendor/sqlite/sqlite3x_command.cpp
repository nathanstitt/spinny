/*
	Copyright (C) 2004-2005 Cory Nelson

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
	
	CVS Info :
		$Author: phrostbyte $
		$Date: 2005/06/16 20:46:40 $
		$Revision: 1.1 $
*/

#include <sqlite3.h>
#include "sqlite.hpp"

namespace sqlite {

	command::command(connection &con) : con(con), _reader(this) {
		if( sqlite3_prepare( con.db,con._cmd.curval().c_str(),con._cmd.curval().length() , &this->stmt, 0)!=SQLITE_OK )
			throw database_error(con);
		con.clear_cmd();
		this->argc=sqlite3_column_count(this->stmt);
	}

        command::command(connection &con, const char *sql) : con(con),_reader(this) {
		if( sqlite3_prepare( con.db, sql, -1, &this->stmt, 0)!=SQLITE_OK )
			throw database_error(con);
		this->argc=sqlite3_column_count(this->stmt);
	}

	command::command(connection &con, const std::string &sql) : con(con),_reader(this) {
		if(sqlite3_prepare(con.db, sql.data(), (int)sql.length(), &this->stmt, 0)!=SQLITE_OK)
			throw database_error(con);
		this->argc=sqlite3_column_count(this->stmt);
	}


	command::~command() {
		if ( stmt )
			sqlite3_finalize(this->stmt);
	}

	void
	command::close(){
		sqlite3_finalize(this->stmt);
		_reader.cmd=0;
		this->stmt=0;
	}
	
	void command::bind( int index, const void *data, int datalen ) {
		int res;
		if ( datalen == 0 ){
			res=sqlite3_bind_null(this->stmt, index);
		} else {
			res=sqlite3_bind_blob(this->stmt, index, data, datalen, SQLITE_TRANSIENT);
		}
		if (res!=SQLITE_OK)
			throw database_error(this->con);
	}

	command::iterator
	command::begin() { 
		_reader.reset();
		_reader.read();
		return iterator( &_reader );
	}

	command::iterator
	command::end(){
		return iterator();
	}
}
