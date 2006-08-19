#include "sqlite.hpp"


namespace sqlite {

	connection::connection() : std::ostream( &_cmd ), db(NULL) {}

	connection::connection( const std::string &db ) : std::ostream( &_cmd ), db(NULL) { this->open(db); }

	connection::~connection() { if(this->db) sqlite3_close(this->db); }

	void connection::open( const std::string &db ) {
		if(sqlite3_open(db.c_str(), &this->db)!=SQLITE_OK)
			throw database_error("unable to open database");
	}

	void
	connection::validate_db(){
		if( ! this->db ) {
			throw database_error("database is not open");
		}
	}

	void
	connection::close() {
		if(this->db) {
			if(sqlite3_close(this->db)!=SQLITE_OK)
				throw database_error(*this);
			this->db=NULL;
		}
	}

	id_t
	connection::insertid() {
		if(!this->db) throw database_error("database is not open");
		return sqlite3_last_insert_rowid(this->db);
	}

	void
	connection::setbusytimeout(int ms) {
		if(!this->db) throw database_error("database is not open");

		if(sqlite3_busy_timeout(this->db, ms)!=SQLITE_OK)
			throw database_error(*this);
	}

	void
	connection::clear_cmd(){
		_cmd.abandon();
	}

	std::string
	connection::current_statement(){
		return _cmd.curval();
	}

	std::string
	connection::buffer::curval(){
		return _buffer;
	}

	void
	connection::buffer::abandon(){
		_buffer.clear();
	}

	int
	connection::buffer::overflow(int c) {
		_buffer += static_cast<char>(c);
		return c;
	}



} // sqlite namespace
