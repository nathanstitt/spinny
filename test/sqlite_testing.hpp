/* @(#)sqlite_testing.hpp
 */

#ifndef _SQLITE_TESTING_H
#define _SQLITE_TESTING_H 1
struct FooDoggy {
	sqlite::id_t id;
	int age;
	int num_bones;
	std::string name;
	void bind_to_db_stmt( sqlite::command *cmd ) const {
		cmd->bind( 1, age );
		cmd->bind<const std::string&>( 2, name );
		cmd->bind( 3, num_bones );
	}

	bool operator==( const FooDoggy &fd ) const {
		return id==fd.id;
	}
	void initialize_from_db( const sqlite::reader *r ){
		age=r->get<int>(0);
		name=r->get<std::string>(1);
		num_bones=r->get<int>(2);
	}

	void set_db_id( sqlite::id_t val ){ id = val; }
};



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
	GRANT_NEEDED_FRIENDSHIP(FooTable);

	sqlite::id_t parent_id;
	string name;	

	bool save() const {
		return sqlite::db()->save<FooTable>(*this);
	}

	static const sqlite::table::description* table_description() {
		return &foo_desc;
	}

	const sqlite::table::description* m_table_description() const {
		return &foo_desc;
	}

	virtual void table_insert_values( std::ostream &str ) const {
		str << parent_id << "," << sqlite::q(name);
	}
	virtual void table_update_values( std::ostream &str ) const {
		str << "parent_id=" << parent_id << "," << "name=" << sqlite::q(name);
	}

	void initialize_from_db( const sqlite::reader *reader ){
		parent_id = reader->get<int>(0);
		name	  = reader->get<std::string>(1);
	}
};



#endif /* _SQLITE_TESTING_H */

