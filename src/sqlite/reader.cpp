#include "sqlite.hpp"


namespace sqlite {

	void
	reader::validate( int index ) const {
		if(!this->_cmd) throw database_error("reader is closed");
		if((index)>(this->_cmd->num_columns()-1)) throw std::out_of_range("index out of range");
	}

	reader::reader() : _cmd(NULL) {}

	reader::reader(command *cmd) : _cmd(cmd) { }

	reader::~reader() {
//		sqlite3_reset(this->_cmd->_stmt);
	}

	bool
	reader::operator==(id_t num) const {
		return ( this->get<id_t>(0) == num );
	}

	bool
	reader::operator==(const std::string &str) const {
		return ( this->get<std::string>(0) == str );
	}

	bool
	reader::read() {
		if(!this->_cmd) throw database_error("reader is closed");
		switch( sqlite3_step( this->_cmd->_stmt ) ) {
		case SQLITE_ROW:
			return true;
		case SQLITE_DONE:
			return false;
		default:
			throw database_error(this->_cmd->_con);
		}
	}

	void reader::reset() {
		if(!this->_cmd) throw database_error("reader is closed");

		if(sqlite3_reset(this->_cmd->_stmt)!=SQLITE_OK)
			throw database_error(this->_cmd->_con);
	}



	std::string reader::colname(int index) const {
		if(!this->_cmd) throw database_error("reader is closed");
		if((index)>(this->_cmd->num_columns()-1)) throw std::out_of_range("index out of range");
		return sqlite3_column_name(this->_cmd->_stmt, index);
	}



}
