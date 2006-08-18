#ifndef __SQLITE_HPP__
#define __SQLITE_HPP__

#include <sqlite/sqlite3.h>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <stdexcept>
#include <vector>
#include <iostream>
using namespace std;

#define SQLITE_GRANT_FRIENDSHIP \
	friend class sqlite::reader; \
	friend class sqlite::command; \
	friend class sqlite::connection; \

namespace sqlite {

	class reader;
	class command;
	class connection;

	struct none{};
	typedef long long auto_id_t;

						
	class table_desc {
	public:
		virtual ~table_desc(){}
		virtual const char* table_name() const = 0;
		virtual int num_fields() const = 0;
		virtual const char** fields() const = 0;
		virtual const char** field_types() const = 0;
	};

	class table { // functionality may be needed later
	public:

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
		void validate( int index ) const;

		reader();
		reader(const reader &copy);

		~reader();
	public:
		bool operator==(const std::string &str) const;

		bool operator==(auto_id_t num) const;

		bool read();

		void reset();

		template<typename T>
		T get(int index) const;

		template<typename T>
		T get() const;

		std::string colname(int index) const;
	};



	class command : boost::noncopyable {
	private:
		friend class reader;
		friend class transaction;
		friend class connection;
		connection &con;

		struct sqlite3_stmt *stmt;

		int argc;

		reader _reader;
	public:
		command(connection &con );
		command(connection &con, const char *sql );
		command(connection &con, const std::string &sql );
		~command();

		template<typename T>
		void bind( T data );

		template<typename T>
		void bind( int index, T data );

		void bind( int index, const void *data, int datalen );

 		template<typename T>
 		T exec();

		void close();

		class iterator
			: public boost::iterator_facade<iterator, reader,boost::forward_traversal_tag>
		{
		public:
			iterator() : _r(0) {}
			explicit iterator( reader *r ) :  _r( r ) {}
		private:
			friend class boost::iterator_core_access;

			void increment(){
				if ( ! _r->read() ){
					_r=0;
				}
			}
			
			bool equal( iterator const& other ) const {
				return other._r == this->_r;
			}
			
			reader& dereference() const {
				return *_r;
			}

			reader *_r;
		};

		iterator begin();
		iterator end();

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
			*this << "select " << T1::fields() << " from " << T1::table() << " where " << field << " = " << value;
			return this->exec<T1>();
		}

		void open( const std::string &db );

		void close();

		auto_id_t insertid();

		void setbusytimeout(int ms);

		template<class T1>
		T1 exec( const std::string &sql );

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



	// specializations


	// connection::exec
	template<class T1> inline
	T1 connection::exec(){
		this->validate_db();
		command c( *this, _cmd.curval() );
		this->clear_cmd();
		return c.exec<T1>();
	}

 	template<class T1> inline
	T1 connection::exec( const std::string &sql){
		this->validate_db();
		return command(*this, sql ).exec<T1>();
	}

	// command::exec
	template<typename T> inline
	T command::exec(){
		_reader.read();
		return _reader.get<T>();
	}

	template<> inline
	none command::exec(){
		_reader.read();
		return none();
	}


	// command::bind
	template<> inline
	void command::bind(int index, const char *data) {
		this->bind( index,data, strnlen( data, 1024 ) );
	}


	template<> inline
	void command::bind(int index, int data) {
		_reader.reset();
		if(sqlite3_bind_int(this->stmt, index, data)!=SQLITE_OK)
			throw database_error(this->con);
	}
	
	template<> inline
	void command::bind( int index, const std::string &data){
		_reader.reset();
		if(sqlite3_bind_text(this->stmt, index, data.data(), (int)data.length(), SQLITE_TRANSIENT)!=SQLITE_OK)
			throw database_error(this->con);
	}


	template<> inline
	void command::bind(int index, const std::wstring &data){
		_reader.reset();
		if(sqlite3_bind_text16(this->stmt, index, data.data(), (int)data.length()*2, SQLITE_TRANSIENT)!=SQLITE_OK)
			throw database_error(this->con);

	}

	template<> inline
	void command::bind(int index, long long data) {
		_reader.reset();
		if(sqlite3_bind_int64(this->stmt, index, data)!=SQLITE_OK)
			throw database_error(this->con);
	}

	template<> inline
	void command::bind(int index, double data) {
		_reader.reset();
		if(sqlite3_bind_double(this->stmt, index, data)!=SQLITE_OK)
			throw database_error(this->con);
	}

	// reader::get
	template <class T>
	T reader::get() const {
		return this->get<T>( 0 );
	}


	template <class T>
	T reader::get( int index ) const {
		validate(index);
		return boost::lexical_cast<T>( sqlite3_column_text(this->cmd->stmt, index) );
	}

	template <> inline
	int reader::get( int index ) const {
		validate(index);
		return sqlite3_column_int(this->cmd->stmt, index);
	}

	template <> inline
	long long reader::get( int index ) const {
		validate(index);
		return sqlite3_column_int64(this->cmd->stmt, index);
	}
		
	template <> inline
	double reader::get( int index ) const {
		validate(index);
		return sqlite3_column_double(this->cmd->stmt, index);
	}

	template <> inline
	std::string reader::get( int index ) const {
		validate(index);
		return std::string((const char*)sqlite3_column_text(this->cmd->stmt, index), sqlite3_column_bytes(this->cmd->stmt, index));
	}

	template<> inline
	std::wstring reader::get(int index) const {
		validate(index);
		return std::wstring((const wchar_t*)sqlite3_column_text16(this->cmd->stmt, index), sqlite3_column_bytes16(this->cmd->stmt, index)/2);
	}



}

#endif // __SQLITE_HPP__
