#ifndef __SQLITE_HPP__
#define __SQLITE_HPP__

#include <sqlite/sqlite3.h>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/algorithm/string.hpp"
#include <boost/iterator/iterator_facade.hpp>
#include <stdexcept>
#include <list>
#include <iostream>
#include <boost/log/log.hpp>
using namespace std;

BOOST_DECLARE_LOG(sql)


#define GRANT_SQLITE_FRIENDSHIP \
	friend class sqlite::reader; \
	friend class sqlite::command; \
	friend class sqlite::connection; \

namespace sqlite {

	template<class T> inline
	T q( const T &val ){
		return val;
	}

	template<> inline
	std::string q( const std::string &val ){
		return boost::replace_all_copy(val, "'","''" );
	}


	class reader;
	class command;
	class connection;

	struct none{};
	typedef long long id_t;
						
	class table {
		id_t _id;
	protected:
		table();
	public:
		virtual bool operator == ( const table &other ) const;
		struct description {
			virtual ~description();
			description();
			virtual void insert_fields( std::ostream &str ) const;
			virtual const char* table_name() const = 0;
			virtual int num_fields() const = 0;
			virtual const char** fields() const = 0;
			virtual const char** field_types() const = 0;
		};
		virtual ~table();
		bool needs_saved();
		virtual id_t db_id() const;
		virtual void set_db_id( id_t );

		virtual bool save() = 0;
		virtual void initialize_from_db( const reader* ) = 0;
		virtual const description* m_table_description() const = 0;
		virtual void table_insert_values( std::ostream &str ) const = 0;
		virtual void table_update_values( std::ostream &str ) const = 0;
	};

	// the constructor for table desc will register with this 
	// function.
	std::list< table::description* > *
	register_db_table_check( sqlite::table::description *table );

	// this will check the tables registered
	// above to ensure they exist and have the 
	// proper column names
	// since sqlite doesn't enforce column types
	// no attempt is made to check that columns
	// are of the proper type
	void
	check_and_create_tables( connection &conn );


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

		bool operator==(id_t num) const;

		bool read();

		void reset();

		template<typename T>
		T get(int index) const;

		template<typename T>
		T get() const;

		std::string colname(int index) const;
	};


	template<class T>
	struct slurp{
		slurp() : ii( container, container.end() ) { }
		void operator()( sqlite::reader &r ){
			*ii = r.get<typename T::value_type>();
		}
		T container;
		std::insert_iterator<T> ii;
	};


	class command : boost::noncopyable {
	private:
		friend class reader;
		friend class transaction;
		friend class connection;
		connection &con;

		struct sqlite3_stmt *stmt;
		int num_columns;

		reader _reader;
	public:
		command(connection &con );
		command(connection &con, const char *sql );
		command(connection &con, const std::string &sql );
		~command();

		template<typename T>
		void bind( const T& data );

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
		boost::shared_ptr<command>
		load_many(  const std::string &field, const T2 &value, int limit=0 ){
			*this << "select ";
			const ::sqlite::table::description *td=T1::table_description();
			td->insert_fields( *this );
			*this << ",rowid from " << td->table_name() << " where " << field << " = " << q(value);
			if ( limit ){
				*this << " limit " << limit;
			}
			command *cmd = new command( *this,  _cmd.curval() );
			boost::shared_ptr<command> ret( cmd );
			BOOST_LOG(sql) << "load_many of " << typeid(T1).name();
			this->clear_cmd();
			return ret;
		}


		template<class T1,class T2>
		T1
		load_one( const std::string &field, const T2 &value ){
			boost::shared_ptr<command> cmd = this->load_many<T1,T2>( field, value, 1 );
			command::iterator iter = cmd->begin();
			T1 obj = iter->get<T1>();
			return obj;
		}

		template<class T1,class T2>
		T1
		load( T2 value ){
			return this->load_one<T1,T2>( "rowid", value );
		}


		template<class T>
		bool
		connection::save( T &obj ){
			const ::sqlite::table::description *td=T::table_description();
			if ( ! obj.db_id() ){
				*this << "insert into " << td->table_name() << "(";
				td->insert_fields( *this );
				*this << ") values (";
				obj.table_insert_values( *this );
				*this << ")";
				BOOST_LOG(sql) << "save (insert) of " << typeid(T).name();
				this->exec<none>();
				obj.set_db_id( this->insertid() );
			} else {
				*this << "update " << td->table_name() << " set ";
				obj.table_update_values( *this );
				* this << " where rowid=" << obj.db_id();
				BOOST_LOG(sql) << "save (update) of " << typeid(T).name();
				this->exec<none>();
			}
			return true;
		}


		void open( const std::string &db );

		void close();

		id_t insertid();

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
		return _reader.get<T>(0);
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

	template<typename T> inline
	void command::bind( const T &obj ){
		obj.bind_to_db_stmt( this );
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
		T obj;
		obj.initialize_from_db( this );
		obj.set_db_id( this->get<id_t>( this->cmd->num_columns-1 ) );
		return obj;
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
