/* @(#)user.hpp
 */

#ifndef _USER_H
#define _USER_H 1

#include "spinny/app.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

namespace Spinny {

class User : public sqlite::table {
public:
	enum Role {
		ReadOnlyRole,
		ModifyRole,
		AdminRole
	};
private:
	GRANT_NEEDED_FRIENDSHIP(User);

	User();

	virtual void table_insert_values( std::ostream &str ) const;
	virtual void table_update_values( std::ostream &str ) const;
	void initialize_from_db( const sqlite::reader *reader );
 	static const sqlite::table::description* table_description();
 	virtual const description* m_table_description() const;

	Role role_;
	std::string password_;
	
public:
	typedef boost::shared_ptr<User> ptr;
	typedef ::sqlite::result_set<User> result_set;

	User::ptr
	static load( sqlite::id_t db_id );

	static ptr
	with_ticket( const std::string &ticket );

	static ptr
	with_login( const std::string &login );

	static ptr
	create( const std::string &login, const std::string &password );

	static result_set
	list( int first=0, int count=0, const std::string &sort="", bool descending=true );

	static unsigned int
	count();

	void
	update_access_time();

	std::string login;
	std::string ticket;
	boost::posix_time::ptime last_visit;

	bool
	has_modify_role() const;

	bool
	is_admin() const;

	Role role() const;

	Role
	set_role( Role role );

	bool
	has_at_least( Role role ) const;

	std::string
	set_password( const std::string &pass );

	bool
	authen( const std::string &password );

	std::string
	generate_new_ticket();

	bool
	save() const;
};

} // namespace Spinny

#endif /* _USER_H */

