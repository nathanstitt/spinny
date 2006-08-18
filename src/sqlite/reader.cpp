#include "sqlite.hpp"


namespace sqlite {

	void
	reader::validate( int index ) const {
		if(!this->cmd) throw database_error("reader is closed");
		if((index)>(this->cmd->argc-1)) throw std::out_of_range("index out of range");
	}

	reader::reader() : cmd(NULL) {}

	reader::reader(command *cmd) : cmd(cmd) { }

	reader::~reader() {
//		sqlite3_reset(this->cmd->stmt);
	}

	bool
	reader::operator==(id_t num) const {
		return ( this->get<id_t>() == num );
	}

	bool
	reader::operator==(const std::string &str) const {
		return ( this->get<std::string>() == str );
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



	std::string reader::colname(int index) const {
		if(!this->cmd) throw database_error("reader is closed");
		if((index)>(this->cmd->argc-1)) throw std::out_of_range("index out of range");
		return sqlite3_column_name(this->cmd->stmt, index);
	}



}
