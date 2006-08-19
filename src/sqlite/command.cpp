#include "sqlite.hpp"

namespace sqlite {

	command::command(connection &con) : con(con), _reader(this) {
		if( sqlite3_prepare( con.db,con._cmd.curval().c_str(),con._cmd.curval().length() , &this->stmt, 0)!=SQLITE_OK )
			throw database_error(con);
		BOOST_LOG(sql) << sqlite3_data_count(this->stmt) << " : " <<  con._cmd.curval();
		con.clear_cmd();
		this->num_columns=sqlite3_column_count(this->stmt);
	}

        command::command(connection &con, const char *sqlstmt) : con(con),_reader(this) {
		if( sqlite3_prepare( con.db, sqlstmt, -1, &this->stmt, 0)!=SQLITE_OK )
			throw database_error(con);
		this->num_columns=sqlite3_column_count(this->stmt);
		BOOST_LOG(sql) << sqlite3_data_count(this->stmt) << " : " <<  sqlstmt;
	}

	command::command(connection &con, const std::string &sqlstmt) : con(con),_reader(this) {
		if(sqlite3_prepare(con.db, sqlstmt.data(), (int)sqlstmt.length(), &this->stmt, 0)!=SQLITE_OK)
			throw database_error(con);
		BOOST_LOG(sql) << sqlite3_data_count(this->stmt) << " : " <<  sqlstmt;
		this->num_columns=sqlite3_column_count(this->stmt);
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
