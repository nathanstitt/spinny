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

#ifndef __SQLITE3X_HPP__
#define __SQLITE3X_HPP__

#include <string>
#include <boost/utility.hpp>
#include <stdexcept>

namespace sqlite {
	class reader;

	class connection : boost::noncopyable {
	private:
		friend class command;
		friend class database_error;
		struct sqlite3 *db;
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

		template<class T>
		T exec(const char *sql);

		template<class T>
		T exec(const wchar_t *sql);

		template<class T>
		T exec(const std::string &sql);

		template<class T>
		T exec(const std::wstring &sql);
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
		struct stmt *stmt;
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

		reader executereader();
		void executenonquery();
		int executeint();
		long long executeint64();
		double executedouble();
		std::string executestring();
		std::wstring executestring16();
		std::string executeblob();
	};

	class reader {
	private:
		friend class command;

		command *cmd;

		reader(command *cmd);

	public:
		reader();
		reader(const reader &copy);
		~reader();

		reader& operator=(const reader &copy);

		bool read();
		void reset();
		void close();

		int getint(int index);
		long long getint64(int index);
		double getdouble(int index);
		std::string getstring(int index);
		std::wstring getstring16(int index);
		std::string getblob(int index);

		std::string getcolname(int index);
		std::wstring getcolname16(int index);
	};

	class database_error : public std::runtime_error {
	public:
		database_error(const char *msg);
		database_error(connection &con);
	};

}

#endif
