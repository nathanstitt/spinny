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

	connection::connection() : db(NULL) {}

	connection::connection(const char *db) : db(NULL) { this->open(db); }

	connection::connection(const wchar_t *db) : db(NULL) { this->open(db); }

	connection::~connection() { if(this->db) sqlite3_close(this->db); }

	void connection::open(const char *db) {
		if(sqlite3_open(db, &this->db)!=SQLITE_OK)
			throw database_error("unable to open database");
	}

	void connection::open(const wchar_t *db) {
		if(sqlite3_open16(db, &this->db)!=SQLITE_OK)
			throw database_error("unable to open database");
	}

	void
	connection::validate_db(){
		if( ! this->db ) {
			throw database_error("database is not open");
		}
	}

	void connection::close() {
		if(this->db) {
			if(sqlite3_close(this->db)!=SQLITE_OK)
				throw database_error(*this);
			this->db=NULL;
		}
	}

	long long connection::insertid() {
		if(!this->db) throw database_error("database is not open");
		return sqlite3_last_insert_rowid(this->db);
	}

	void connection::setbusytimeout(int ms) {
		if(!this->db) throw database_error("database is not open");

		if(sqlite3_busy_timeout(this->db, ms)!=SQLITE_OK)
			throw database_error(*this);
	}





} // sqlite namespace
