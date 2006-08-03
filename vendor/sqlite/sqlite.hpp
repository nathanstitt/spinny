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
#include <boost/utility.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <sqlite/sqlite3.h>

namespace sqlite {
	class reader;
	struct none{};

	class connection : boost::noncopyable {
	private:
		friend class command;
		friend class database_error;
		struct sqlite3 *db;
		void validate_db();
	public:
		connection();
		connection(const char *db);
		connection(const wchar_t *db);
		~connection();

		void open(const char *db);
		void open(const wchar_t *db);
		void close();

		long long insertid();

		void setbusytimeout(int ms);

		template<class T1, class T2>
		T1 exec( T2 sql);
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

		connection &con;
		struct sqlite3_stmt *stmt;
		unsigned int refs;
		int argc;

	public:
		command(connection &con, const char *sql);
		command(connection &con, const wchar_t *sql);
		command(connection &con, const std::string &sql);
		command(connection &con, const std::wstring &sql);
		~command();

		void bind(int index);
		void bind(int index, int data);
		void bind(int index, long long data);
		void bind(int index, double data);
		void bind(int index, const char *data, int datalen);
		void bind(int index, const wchar_t *data, int datalen);
		void bind(int index, const void *data, int datalen);
		void bind(int index, const std::string &data);
		void bind(int index, const std::wstring &data);

		template<typename T>
		T exec();
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
		T get(int index=0);

		std::string getcolname(int index);
		std::wstring getcolname16(int index);
	};

	// specializations
	template<class T1, class T2>
	T1 connection::exec(T2 sql){
		this->validate_db();
		return command(*this, sql ).exec<T1>();
	}

	template<typename T>
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
