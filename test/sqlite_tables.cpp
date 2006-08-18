
#include "testing.hpp"
#include "sqlite_testing.hpp"


SUITE(SqliteTables) {



struct foo_desc_table : public sqlite::table_desc {
	virtual const char* table_name() const {
		return "footab";
	};
	virtual int num_fields() const {
		return 3;
	}
	virtual const char** fields() const {
		static const char *fields[] = {
			"id",
			"parent_id",
			"name",
		};
		return fields;
	}
	virtual const char** field_types() const {
		static const char *field_types[] = {
			"int",
			"int",
			"string",
		};
		return field_types;
	};
};
static foo_desc_table foo_desc;

TEST( Register ) {
	Conn c;

	typedef std::list<table_desc*> tl;
	tl *tables = sqlite::register_db_table_check( NULL );

	CHECK( ! tables->empty() );
	CHECK( tables->end() != std::find( tables->begin(), tables->end(), &foo_desc ) );
}


TEST( Check ){
	Conn c;
	sqlite::check_and_create_tables( c.con );
	c.con << "insert into footab(id,parent_id,name) values ( 2, 3, 'basd' )";
	c.con.exec<none>();

}

} // SUITE(SqliteTables)





int
sqlite_tables( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("SqliteTables");

}
