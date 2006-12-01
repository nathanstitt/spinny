/* @(#)user.hpp
 */

#ifndef _USER_H
#define _USER_H 1

#include "spinny/spinny.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

class User : public sqlite::table {
public:
	enum Role_t {
		GuestRole,
		TrustedRole,
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

	Role_t role_;
	std::string password_;
	
public:
	typedef boost::shared_ptr<User> ptr;

	static ptr with_ticket( std::string &ticket );
	static ptr create( const std::string &login, const std::string &password );

	std::string login;
	std::string ticket;
	boost::posix_time::ptime last_visit;

	bool
	is_guest() const;

	bool
	is_trusted() const;

	bool
	is_admin() const;

	Role_t
	set_role( Role_t role );

	std::string
	set_password( const std::string &pass );

	bool
	authen( const std::string &password );

	std::string
	generate_new_ticket();

	bool
	save() const;
};

#endif /* _USER_H */

