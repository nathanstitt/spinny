/* @(#)sqlite_testing.hpp
 */

#ifndef _SQLITE_TESTING_H
#define _SQLITE_TESTING_H 1
struct FooDoggy {
	int age;
	int num_bones;
	std::string name;
	void bind_to_db_stmt( sqlite::command *cmd ) const {
		cmd->bind( 1, age );
		cmd->bind<const std::string&>( 2, name );
		cmd->bind( 3, num_bones );
	}

	void initialize_from_db( const sqlite::reader *r ){
		age=r->get<int>(0);
		name=r->get<std::string>(1);
		num_bones=r->get<int>(2);
	}

	void set_db_id( sqlite::id_t val ){	}
};


#endif /* _SQLITE_TESTING_H */

