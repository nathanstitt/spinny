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

	void
	reader::validate( int index ){
		if(!this->cmd) throw database_error("reader is closed");
		if((index)>(this->cmd->argc-1)) throw std::out_of_range("index out of range");
	}

	reader::reader() : cmd(NULL) {}

	reader::reader(const reader &copy) : cmd(copy.cmd) {
		if(this->cmd) ++this->cmd->refs;
	}

	reader::reader(command *cmd) : cmd(cmd) {
		++cmd->refs;
	}

	reader::~reader() {
		this->close();
	}

	reader& reader::operator=(const reader &copy) {
		this->close();

		this->cmd=copy.cmd;
		if(this->cmd) ++this->cmd->refs;

		return *this;
	}

	bool
	reader::read() {
		if(!this->cmd) throw database_error("reader is closed");
		switch( sqlite3_step( this->cmd->stmt ) ) {
		case SQLITE_ROW:
			return true;
		case SQLITE_DONE:
			return false;
		default:
			throw database_error(this->cmd->con);
		}
	}

	void reader::reset() {
		if(!this->cmd) throw database_error("reader is closed");

		if(sqlite3_reset(this->cmd->stmt)!=SQLITE_OK)
			throw database_error(this->cmd->con);
	}

	void reader::close() {
		if(this->cmd) {
			if(--this->cmd->refs==0) sqlite3_reset(this->cmd->stmt);
			this->cmd=NULL;
		}
	}


	std::string reader::getcolname(int index) {
		if(!this->cmd) throw database_error("reader is closed");
		if((index)>(this->cmd->argc-1)) throw std::out_of_range("index out of range");
		return sqlite3_column_name(this->cmd->stmt, index);
	}

	std::wstring reader::getcolname16(int index) {
		if(!this->cmd) throw database_error("reader is closed");
		if((index)>(this->cmd->argc-1)) throw std::out_of_range("index out of range");
		return (const wchar_t*)sqlite3_column_name16(this->cmd->stmt, index);
	}


}
