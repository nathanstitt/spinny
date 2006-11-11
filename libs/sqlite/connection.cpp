#include "sqlite.hpp"

#include <sstream>
#include <algorithm>

#include "boost/thread.hpp"

static
int
busy_handler( void*, int num_waiting ){
	if ( num_waiting > 3 ){
		return 0;
	} else {
		boost::xtime xt;
		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.sec += num_waiting;
		boost::thread::sleep( xt );
		return 1;
	}
}

namespace sqlite {

	connection::connection() : std::ostream( &_cmd ), db(NULL) {}

	connection::connection( const std::string &db ) : std::ostream( &_cmd ), db(NULL) {
		this->open(db); 
	}

	connection::~connection() { if(this->db) sqlite3_close(this->db); }


	void
	connection::log_table_contents( const std::string &name, int limit ){
		std::stringstream str;
		*this << "select rowid,* from " << name << " limit " << limit;
		command cmd( *this,  _cmd.curval() );
		this->clear_cmd();
		BOOST_LOG(sql) << "  Dumping first " << limit << " rows from " << name;
		int cols=cmd.num_columns();
		std::string v;
		command::reader_iterator row = cmd.reader_begin();
		if ( row == cmd.reader_end() ){
			BOOST_LOG(sql) << "Table doesn't exist" << endl;
			return;
		}
		str << " | ";
		for ( int x=0; x<cols; ++x ){
			str.width( max( v.size(), static_cast<size_t>(20) ) );
			str << row->colname( x ) + " | ";
		}
		BOOST_LOG(sql) << str.str();
		str.str("");
		for ( ; row != cmd.reader_end(); ++row ){
			str << " | ";
			for ( int x=0; x<cols; ++x ){
				v=row->get<string>( x );
				str.width( max( v.size(), static_cast<size_t>(20) ) );
				str << v + " | ";
			}
			BOOST_LOG(sql) << str.str();
			str.str("");
		}
		return;
	}

	void connection::open( const std::string &db ) {
		if(sqlite3_open(db.c_str(), &this->db)!=SQLITE_OK)
			throw database_error("unable to open database");

		sqlite3_busy_handler( this->db, busy_handler, NULL );

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
