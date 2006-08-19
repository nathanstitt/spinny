
#include "testing.hpp"
#include "sqlite_testing.hpp"



struct foo_desc_table : public sqlite::table::description {
	
	virtual const char* table_name() const {
		return "foo_table";
	};
	virtual int num_fields() const {
		return 2;
	}
	virtual const char** fields() const {
		static const char *fields[] = {
			"parent_id",
			"name",
		};
		return fields;
	}
	virtual const char** field_types() const {
		static const char *field_types[] = {
			"int",
			"string",
		};
		return field_types;
	};
};

static foo_desc_table foo_desc;

struct FooTable : public sqlite::table {
	GRANT_SQLITE_FRIENDSHIP;

	sqlite::id_t parent_id;
	string name;	

	bool save() {
		Spinny::db()->save<FooTable>(*this);
	}

	static const sqlite::table::description* table_description() {
		return &foo_desc;
	}

	const sqlite::table::description* m_table_description() const {
		return &foo_desc;
	}

	virtual void table_insert_values( std::ostream &str ) const {
		str << parent_id << ",'" << sqlite::q(name) << "'";
	}
	virtual void table_update_values( std::ostream &str ) const {
		str << "parent_id=" << parent_id << "," << "name='" << sqlite::q(name) << "'";
	}

	void initialize_from_db( const sqlite::reader *reader ){
		parent_id = reader->get<int>(0);
		name	  = reader->get<std::string>(1);
	}
};


SUITE(SqliteTables) {


TEST( Register ) {
	DummyApp c;

	typedef std::list<sqlite::table::description*> tl;
	tl *tables = sqlite::register_db_table_check( NULL );

	CHECK( ! tables->empty() );
	CHECK( tables->end() != std::find( tables->begin(), tables->end(), &foo_desc ) );
}


TEST( Check ){
	DummyApp c;
	*c.con << "insert into foo_table (parent_id,name) values ( 3, 'basd' )";
	c.con->exec<sqlite::none>();
}

TEST( SaveNew ){
	DummyApp da;
	FooTable ft;
	ft.parent_id = 43;
	ft.name="testing foo table";
	CHECK( ft.save() );
	CHECK( ft.db_id() );

	CHECK_EQUAL( 43, sqlite::command( *da.con,"select parent_id from foo_table" ).exec<int>() );

	*da.con << "select parent_id,name from foo_table where rowid=" << ft.db_id();
	sqlite::command cmd(*da.con);
	sqlite::command::iterator row=cmd.begin();
	CHECK_EQUAL( 43, row->get<int>(0) );
	CHECK_EQUAL( "testing foo table", row->get<std::string>(1) );

	FooTable ft2=da.con->load<FooTable>( ft.db_id() );

	CHECK_EQUAL( ft.db_id(), ft2.db_id() );
	CHECK_EQUAL( ft.parent_id, ft2.parent_id );
	CHECK_EQUAL( ft.name, ft2.name );
}

TEST( SaveExisting ){



}

} // SUITE(SqliteTables)





int
sqlite_tables( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SqliteTables");

}
