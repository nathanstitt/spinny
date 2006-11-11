#include "sqlite.hpp"

namespace sqlite {
#if defined(_MSC_VER)
   #pragma warning ( disable : 4355 ) /* unreferenced variable */
#endif
	command::command(connection *c) : _con(*c), _reader(this) {
		BOOST_LOG(sql) << " : " <<  _con._cmd.curval();
		if( sqlite3_prepare( _con.db,_con._cmd.curval().c_str(),_con._cmd.curval().length() , &this->_stmt, 0)!=SQLITE_OK )
			throw database_error(_con);
		_con.clear_cmd();
	}

	command::command(connection *c, const std::string &sqlstmt) : _con(*c),_reader(this) {
		BOOST_LOG(sql) <<  sqlstmt;
		if(sqlite3_prepare(_con.db, sqlstmt.data(), (int)sqlstmt.length(), &this->_stmt, 0)!=SQLITE_OK)
			throw database_error(_con);
	}


	command::command(connection &c) : _con(c), _reader(this) {
		BOOST_LOG(sql) <<  _con._cmd.curval();
		if( sqlite3_prepare( _con.db,_con._cmd.curval().c_str(),_con._cmd.curval().length() , &this->_stmt, 0)!=SQLITE_OK )
			throw database_error(_con);
		_con.clear_cmd();
	}

	command::command(connection &c, const std::string &sqlstmt) : _con(c),_reader(this) {
		BOOST_LOG(sql) <<  sqlstmt;
		if(sqlite3_prepare(_con.db, sqlstmt.data(), (int)sqlstmt.length(), &this->_stmt, 0)!=SQLITE_OK)
			throw database_error(_con);
	}


	command::~command() {
		if ( _stmt )
			sqlite3_finalize(this->_stmt);
	}


	int
	command::num_columns() const {
		return sqlite3_column_count(this->_stmt);
	}

	void
	command::close(){
		sqlite3_finalize(this->_stmt);
		_reader._cmd=0;
		this->_stmt=0;
	}
	
	void command::bind( int index, const void *data, int datalen ) {
		int res;
		if ( datalen == 0 ){
			res=sqlite3_bind_null(this->_stmt, index);
		} else {
			res=sqlite3_bind_blob(this->_stmt, index, data, datalen, SQLITE_TRANSIENT);
		}
		if (res!=SQLITE_OK)
			throw database_error(this->_con);
	}

	command::reader_iterator
	command::reader_begin() { 
		_reader.reset();
		if ( _reader.read() ){
			return reader_iterator( &_reader );
		} else {
			return this->reader_end();
		}
	}

	command::reader_iterator
	command::reader_end(){
		return reader_iterator();
	}

}
