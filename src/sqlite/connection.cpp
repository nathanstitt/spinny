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

	bool
	connection::save( table &obj ){
		const ::sqlite::table::description *td=obj.table_description();
		if ( ! obj.db_id() ){
			*this << "insert into ( " << td->table_name();
			td->insert_fields( *this, true );
			*this << ") values (";
			obj.table_insert_values( *this );
			*this << ")";
			this->exec<none>();
			obj.set_db_id( this->insertid() );
		} else {
			*this << "update " << td->table_name() << " set ";
			obj.table_update_values( *this );
			* this << " where rowid=" << obj.db_id();
			this->exec<none>();
		}
		return true;
	}

} // sqlite namespace
