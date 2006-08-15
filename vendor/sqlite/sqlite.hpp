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

#ifndef __SQLITE_HPP__
#define __SQLITE_HPP__

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <sqlite/sqlite3.h>
#include <iostream>

namespace sqlite {

	class reader;
	struct none{};
	typedef long long auto_id_t;

									
	class table { // functionality may be needed later
		friend class reader;
		friend class command;
		friend class connection;

	};

	class connection : boost::noncopyable, public std::ostream {
	private:
		class buffer : public std::streambuf {
		public:
			//! return contents of buffer
			std::string curval();
			//! abandon contents of buffer
			void abandon();
		private:
			int overflow(int c);
			std::string _buffer;
		};

		buffer _cmd;

		friend class command;
		friend class database_error;
		struct sqlite3 *db;
		void validate_db();
	public:
		connection();
		connection(const std::string &db );
 		~connection();

		template<class T1,class T2>
		T1
		find_by_field( const std::string &field, T2 value ){
			*this << "select " << T1::fields() << " from " << T1::table() << " where field = " << value;
			return this->exec<T1>();
		}

		void open( const std::string &db );

		void close();

		auto_id_t insertid();

		void setbusytimeout(int ms);

		template<class T1, class T2>
		T1 exec( T2 sql);

		template<class T1>
		T1 exec();

		void clear_cmd();
		std::string current_statement();
	};



	class transaction : boost::noncopyable {
	private:
		connection &con;
		bool intrans;

	public:
		transaction(connection &con, bool start=true);
		~transaction();

		void begin();
		void commit();
		void rollback();
	};

	class command : boost::noncopyable {
	private:
		friend class reader;
		friend class transaction;
		friend class connection;
		connection &con;
		struct sqlite3_stmt *stmt;
		unsigned int refs;
		int argc;

	public:
		command(connection &con );
		command(connection &con, const char *sql);
		command(connection &con, const wchar_t *sql);
		command(connection &con, const std::string &sql);
		command(connection &con, const std::wstring &sql);
		~command();

		template<typename T>
		void bind( T data );

		template<typename T>
		void bind( int index, T data );

		void command::bind( int index, const void *data, int datalen );

		template<typename T>
		T exec();

		void close();
	};


	class database_error : public std::runtime_error {
	public:
		database_error(const char *msg);
		database_error(connection &con);
	};


	class reader {
	private:
		friend class command;

		command *cmd;

		reader(command *cmd);
		void validate( int index );
	public:
		reader();
		reader(const reader &copy);
		~reader();

		reader& operator=(const reader &copy);

		bool read();
		void reset();
		void close();

		template<typename T>
		T get(int index);

		template<typename T>
		T get();

		std::string getcolname(int index);
		std::wstring getcolname16(int index);
	};




	// specializations
	template<class T1> inline
	T1 connection::exec(){
		this->validate_db();
		command c( *this, _cmd.curval() );
		this->clear_cmd();
		return c.exec<T1>();
	}

 	template<class T1, class T2> inline
	T1 connection::exec(T2 sql){
		this->validate_db();
		return command(*this, sql ).exec<T1>();
	}

	template<typename T> inline
	T command::exec(){
		reader r(this);
		r.read();
		return r.get<T>();
	}

	template<> inline
	none command::exec(){ 
		reader(this).read();
		return none();
	}

	template<> inline
	reader command::exec(){ 
		return reader(this);
	}

	template<> inline
	void command::bind(int index, const char *data) {
		this->bind( index,data, strnlen( data, 1024 ) );
	}


	template<> inline
	void command::bind(int index, int data) {
		if(sqlite3_bind_int(this->stmt, index, data)!=SQLITE_OK)
			throw database_error(this->con);
	}
	
	template<> inline
	void command::bind( int index, const std::string &data){
		if(sqlite3_bind_text(this->stmt, index, data.data(), (int)data.length(), SQLITE_TRANSIENT)!=SQLITE_OK)
			throw database_error(this->con);
	}


	template<> inline
	void command::bind(int index, const std::wstring &data){
		if(sqlite3_bind_text16(this->stmt, index, data.data(), (int)data.length()*2, SQLITE_TRANSIENT)!=SQLITE_OK)
			throw database_error(this->con);

	}

	template<> inline
	void command::bind(int index, long long data) {
		if(sqlite3_bind_int64(this->stmt, index, data)!=SQLITE_OK)
			throw database_error(this->con);
	}

	template<> inline
	void command::bind(int index, double data) {
		if(sqlite3_bind_double(this->stmt, index, data)!=SQLITE_OK)
			throw database_error(this->con);
	}

	template <class T>
	T reader::get(){
		return this->get<T>( 0 );
	}


	template <class T>
	T reader::get( int index ){
		validate(index);
		return boost::lexical_cast<T>( sqlite3_column_text(this->cmd->stmt, index) );
	}

	template <> inline
	int reader::get( int index ){
		validate(index);
		return sqlite3_column_int(this->cmd->stmt, index);
	}

	template <> inline
	long long reader::get( int index ){
		validate(index);
		return sqlite3_column_int64(this->cmd->stmt, index);
	}
		
	template <> inline
	double reader::get( int index ){
		validate(index);
		return sqlite3_column_double(this->cmd->stmt, index);
	}

	template <> inline
	std::string reader::get( int index ){
		validate(index);
		return std::string((const char*)sqlite3_column_text(this->cmd->stmt, index), sqlite3_column_bytes(this->cmd->stmt, index));
	}

	template<> inline
	std::wstring reader::get(int index) {
		validate(index);
		return std::wstring((const wchar_t*)sqlite3_column_text16(this->cmd->stmt, index), sqlite3_column_bytes16(this->cmd->stmt, index)/2);
	}



}

#endif // __SQLITE_HPP__
