#ifndef __SQLITE_HPP__
#define __SQLITE_HPP__

#include <sqlite/sqlite3.h>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include "boost/algorithm/string.hpp"
#include <boost/iterator/iterator_facade.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <stdexcept>
#include <list>
#include <iostream>
#include <boost/log/log.hpp>
#include <boost/call_traits.hpp>
using namespace std;

BOOST_DECLARE_LOG(sql)

namespace sqlite {

	class reader;


	// template functors and other utilities used internally
	namespace detail{
			
		// determine the "Best" way to return objects
		// if it's a base type, return by value,
		// if a class, allocate a new object, safegaurd it
		// with boost::shared_ptr, and return as needed
		template <typename T, bool b>
		struct best_type {
			typedef T stored_type;
			typedef T& ref_type;
			static ref_type as_ref_type( stored_type &val ){ return val; }

			static stored_type create(){
				T v;
				return v;
			};
			static void initialize( const stored_type obj, const reader *r ) {}
		};

		template <typename T>
		struct best_type<T, true>{
			typedef boost::shared_ptr<T> stored_type;
			typedef T& ref_type;
			static ref_type as_ref_type( stored_type &val ){ return *(val.get()); }
			static stored_type create(){ return stored_type(new T()); }
			static void initialize( const stored_type obj, const reader *r );
		};
	}

	// sqlite uses long long for the rowid
	typedef long long id_t;

	// quoteing functions
	const id_t&	q( const id_t &arg );
	const long long&	q( const long long &arg );
	int		q( const int arg );
	std::string	q( const std::string &val );

	// forward declarations
	class reader;
	class command;
	class connection;

	// a special type, which is specialized
	// to mean, don't bother returning anything usefull
	// from queries such as delete
	struct none{};

	// a base class which can be inherited from
	// for easy reading objects from sqlite
	class table {
		mutable id_t _id;
	protected:
		table();
	public:
		virtual bool operator == ( const table &other ) const;
		// a struct that describes a table
		// it should be inhereited from ONLY ONCE per table,
		// as a static member,  when it's created, it registers
		// itself for later checking and creation
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
		virtual bool needs_saved() const;
		virtual bool save_if_needed() const;
		virtual id_t db_id() const;
		virtual void set_db_id( id_t ) const;
		virtual void destroy();
		
		// the below are pure virtual
		virtual bool save() const = 0;
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

	// is thrown in various locations
	class database_error : public std::runtime_error {
	public:
		database_error(const char *msg);
		database_error(connection &con);
	};

	// a simple utility 
	// which is usefull with the std::find_if
	// to search for equality
	template< typename T >
	struct dref_eq {
		boost::shared_ptr<T> obj;
		dref_eq( boost::shared_ptr<T> &o) : obj(o) { }
		bool operator()( T &other ){
			return ( other == *obj );
		}
	};

	// responsible for reading values from
	// a command
	class reader {
	private:
		friend class command;
		command *_cmd;
		reader(command *cmd);
		void validate( int index ) const;
		reader();
		reader(const reader &copy);
		~reader();
	public:
		template<typename T>
		void initialize_obj( boost::shared_ptr<T> obj ) const;
		bool operator==(const std::string &str) const;
		bool operator==(id_t num) const;
		bool read();
		void reset();

		template<typename T>
		T get(int index) const;

		template<typename T>
		typename detail::best_type< T, boost::is_class<T>::value >::stored_type
		get() const;

		std::string colname(int index) const;
	};


	// executes a command on the given connection
	class command : boost::noncopyable {
	private:
		friend class reader;
		friend class transaction;
		friend class connection;
		connection &_con;
		struct sqlite3_stmt *_stmt;
		reader _reader;
	public:
		command(connection *con );
		command(connection *con, const std::string &sql );
		command(connection &con );
		command(connection &con, const std::string &sql );
		~command();

		int num_rows();

		int num_columns();

		template<typename T>
		void bind( const T& data );

		template<typename T>
		void bind( int index, T data );

		void bind( int index, const void *data, int datalen );

		template<typename T>
		T exec();

		void close();

		// iterates over the command results
		// returning the reader instance, allowing 
		// picking values from columns
		class reader_iterator : public boost::iterator_facade<reader_iterator, reader, boost::forward_traversal_tag>
		{
			friend class ::sqlite::command;
			friend class boost::iterator_core_access;
			explicit reader_iterator( reader *r ) :  _r( r ) { }
			void increment(){ if ( ! _r->read() ) _r=0; }
			bool equal( reader_iterator const& other ) const { return other._r == this->_r; }
			reader& dereference() const { return *_r; }
			reader *_r;
		public:
			reader_iterator() : _r(0) {}
		};

		// connection::reader_begin & reader_end
		reader_iterator reader_begin();
		reader_iterator reader_end();



		// iterates over the command results,
		// returning the entire column as an object.
		template<class T>
		class iterator :
			public boost::iterator_facade< ::sqlite::command::iterator<T>, 
						       T,
						       boost::forward_traversal_tag>
		{
			friend class ::sqlite::command;
			friend class boost::iterator_core_access;
			explicit iterator( reader *r ) :  _r( r ), obj( _r->get<T>() ) { }
			void increment(){ 
				if ( ! _r->read() ) {
					_r=0; 
				} else {
					obj=_r->get<T>();
				}
			}

			bool equal( ::sqlite::command::iterator<T> const& other ) const { 
				return other._r == this->_r;
			}

			// note that although this returns the value by reference.. it is a weak
			// one, and the value will be destroyed once the iterator moves on to
			// the next row.  Hopefully before completion, I will rewrite to use boost::shared_container_iterator
			typename ::sqlite::detail::best_type< T, boost::is_class<T>::value >::ref_type
			dereference() const {
				return ::sqlite::detail::best_type< T, boost::is_class<T>::value >::as_ref_type( obj );
			}
			reader *_r;

			mutable 
			typename detail::best_type< T, boost::is_class<T>::value >::stored_type
			obj;
		public:

			// this method allows retrieval of the object's
			// boost::safe_ptr wrapper, therefore allowing the object to persist
			// past the iterator moving on to the next row
			typename detail::best_type< T, boost::is_class<T>::value >::stored_type
			shared_ptr(){
				return obj;
			}

			iterator() : _r(0) {}
		};

		// return an iterator
		template<class T>
		iterator<T> begin(){
			_reader.reset();
			if ( _reader.read() ) {
				return iterator<T>( &_reader );
			} else {
				return this->end<T>();
			}
		}
		// a end iterator
		template<class T>
		iterator<T> end(){
			return iterator<T>();
		}
	};


	// how connection::load_many returns it results
	// really just a wrapper for sqlite::command
	template<typename T>
	struct result_set {
		typedef command::iterator<T> iterator;

		result_set( command *c ) : cmd(c){ }

		iterator
		begin(){ return cmd->begin<T>(); }

		iterator
		end(){ return cmd->end<T>(); }

		boost::shared_ptr<command> cmd;
	};


	// the main connection class.
	// inherits from ostream, so can commands can be streamed
	// into it
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
		void log_table_contents( const std::string &name, int limit=10 );
		connection();
		connection(const std::string &db );
		~connection();
		
		template<class T>
		result_set<T>
		load_many(){
			command *cmd = new command( *this,  _cmd.curval() );
			result_set<T> ret( cmd );
			BOOST_LOG(sql) << "load_many of " << typeid(T).name();
			this->clear_cmd();
			return ret;
		}

		// load many sqlite::table objects
		template<class T1,class T2>
		result_set<T1>
		load_many(  const std::string &field, const T2 &value, int limit=0,const std::string op="=" ){
			*this << "select ";
			const ::sqlite::table::description *td=T1::table_description();
			td->insert_fields( *this );
			*this << ",rowid from " << td->table_name();
			if ( ! field.empty() ){
				*this << " where " << field << op << q(value);
			}
			if ( limit ){
				*this << " limit " << limit;
			}
			return this->load_many<T1>();
		}


		// load one sqlite::table object
		template<class T,class T2>
		typename ::sqlite::detail::best_type< T, boost::is_class<T>::value >::stored_type
		load_one( const std::string &field, const T2 &value ){
			result_set<T> rs=this->load_many<T,T2>( field, value, 1 );
			return rs.begin().shared_ptr();
		}

		// load a sqlite::table object where rowid = value
		template<class T>
		typename ::sqlite::detail::best_type< T, boost::is_class<T>::value >::stored_type
		load( sqlite::id_t value ){
			return this->load_one<T,sqlite::id_t>( "rowid", value );
		}

		// save an object to the database.
		// performs either an insert or update, depending on whether or
		// not the object has it's db_id set. Note
		// it does NOT pay attention to the needs_saved() method
		template<class T>
		bool
		save( const T &obj ){
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
			BOOST_LOG(sql) << "ID: " << obj.db_id();
			return true;
		}


		// open the connection to the db file
		void open( const std::string &file );

		// close the connction
		void close();

		// return the rowid of the last row that was created
		id_t insertid();

		// how long to wait on locked table's before giving up and throwing database_error
		void setbusytimeout(int ms);

		// exec the statement, returning either sqlite::none, a command class,
		// or the value from the first column & row, converted to T
		template<class T>
		T exec( const std::string &sql );

		// exec whatever statement has been streamed in, returning values as above.
		template<class T>
		T exec();

		// clear the ostream buffer
		void clear_cmd();

		//  return the contents of the ostream buffer
		std::string current_statement();
	};


	// a sql transaction.. if start is true (the default)
	// begins on creation, dtor calls rollback, unless
	// commit is called prior
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
		if(sqlite3_bind_int(this->_stmt, index, data)!=SQLITE_OK)
			throw database_error(this->_con);
	}
		
	template<> inline
	void command::bind( int index, const std::string &data){
		_reader.reset();
		if(sqlite3_bind_text(this->_stmt, index, data.data(), (int)data.length(), SQLITE_TRANSIENT)!=SQLITE_OK)
			throw database_error(this->_con);
	}


	template<> inline
	void command::bind(int index, const std::wstring &data){
		_reader.reset();
		if(sqlite3_bind_text16(this->_stmt, index, data.data(), (int)data.length()*2, SQLITE_TRANSIENT)!=SQLITE_OK)
			throw database_error(this->_con);

	}

	template<> inline
	void command::bind(int index, long long data) {
		_reader.reset();
		if(sqlite3_bind_int64(this->_stmt, index, data)!=SQLITE_OK)
			throw database_error(this->_con);
	}

	template<> inline
	void command::bind(int index, double data) {
		_reader.reset();
		if(sqlite3_bind_double(this->_stmt, index, data)!=SQLITE_OK)
			throw database_error(this->_con);
	}

	namespace detail {
		template <typename T>
		void best_type<T,true>::initialize( const stored_type obj, const reader *r ) {
			r->initialize_obj( obj );
		}
	}



	template<typename T>
	void
	reader::initialize_obj( const boost::shared_ptr<T> obj ) const {
		obj->initialize_from_db( this );
		obj->set_db_id( this->get<id_t>( this->_cmd->num_columns()-1 ) );
	}

 // reader::get

	template <class T>
	typename detail::best_type< T, boost::is_class<T>::value >::stored_type
	reader::get() const {
		typename detail::best_type< T, boost::is_class<T>::value >::stored_type obj = 
			detail::best_type< T, boost::is_class<T>::value >::create();
		detail::best_type< T, boost::is_class<T>::value >::initialize( obj,this );
		return obj;
	}


	template <class T>
	T reader::get( int index ) const {
		validate(index);
		return boost::lexical_cast<T>( sqlite3_column_text(this->_cmd->_stmt, index) );
	}

	template <> inline
	int reader::get( int index ) const {
		validate(index);
		return sqlite3_column_int(this->_cmd->_stmt, index);
	}

	template <> inline
	long long reader::get( int index ) const {
		validate(index);
		return sqlite3_column_int64(this->_cmd->_stmt, index);
	}
			
	template <> inline
	double reader::get( int index ) const {
		validate(index);
		return sqlite3_column_double(this->_cmd->_stmt, index);
	}

	template <> inline
	std::string reader::get( int index ) const {
		validate(index);
		return std::string((const char*)sqlite3_column_text(this->_cmd->_stmt, index), sqlite3_column_bytes(this->_cmd->_stmt, index));
	}

	template<> inline
	std::wstring reader::get(int index) const {
		validate(index);
		return std::wstring((const wchar_t*)sqlite3_column_text16(this->_cmd->_stmt, index), sqlite3_column_bytes16(this->_cmd->_stmt, index)/2);
	}



} // namespace sqlite

#endif // __SQLITE_HPP__
