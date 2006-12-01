
#include "spinny/user.hpp"
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include "boost/nondet_random.hpp"
#include <boost/generator_iterator.hpp>



class user_desc : public sqlite::table::description {
public:
	virtual const char* table_name() const {
		return "users";
	};
	virtual int num_fields() const {
		return 5;
	}
	virtual const char** fields() const {
		static const char *fields[] = {
			"login",
			"password",
			"ticket",
			"last_visit",
			"role",
		};
		return fields;
	}
	virtual const char** field_types() const {
		static const char *field_types[] = {
			"string",
			"string",
			"string",
			"string",
			"int"
		};
		return field_types;
	};
};

static user_desc table_desc;

User::User() : role_(GuestRole)
{ }

const sqlite::table::description*
User::table_description(){
	return &table_desc;
}


const sqlite::table::description*
User::m_table_description() const {
	return &table_desc;
}


void
User::table_insert_values( std::ostream &str ) const {
	str << sqlite::q(login) << ',' << sqlite::q(password_) << ',' << sqlite::q(ticket)
	    <<  ',' << sqlite::q( boost::posix_time::to_iso_string( last_visit ) ) << ',' << role_;
}

void
User::table_update_values( std::ostream &str ) const {
	str << "login=" << sqlite::q(login) << ",password=" << sqlite::q(password_) 
	    << ",ticket=" << sqlite::q(ticket) << ",last_visit=" << sqlite::q( boost::posix_time::to_iso_string( last_visit ) )
	    << ",role=" << role_;
}


void
User::initialize_from_db( const sqlite::reader *reader ) {
	login	   = reader->get<std::string>(0);
	password_   = reader->get<std::string>(1);
	ticket	   = reader->get<std::string>(2);
	last_visit = boost::posix_time::from_iso_string( reader->get<std::string>(3) );
	role_	   = static_cast<Role_t>( reader->get<int>(4) );
}


User::ptr
User::with_ticket( std::string &ticket ){
	return sqlite::db()->load_one<User>( "ticket", ticket );
}


User::ptr
User::create( const std::string &login, const std::string &password ){
	User::ptr user( new User );
	user->login = login;
	user->password_ = password;
	user->generate_new_ticket();
	user->last_visit = boost::posix_time::second_clock::local_time();
	return user;
}



bool
User::is_admin() const {
	return AdminRole == role_;
}

bool
User::is_guest() const {
	return GuestRole == role_;
}

bool
User::is_trusted() const {
	return TrustedRole == role_;
}

User::Role_t
User::set_role( Role_t role ){
	return role_=role;
}

std::string
User::set_password( const std::string &pass ){
	return password_=pass;
}

bool
User::authen( const std::string &pass ){
	if ( pass == password_ ){
		this->generate_new_ticket();
		return true;
	} else {
		return false;
	}
}

std::string
User::generate_new_ticket(){
	this->ticket = "";

	typedef boost::minstd_rand base_generator_type;

	typedef boost::uniform_int<> distribution_type;
	typedef boost::variate_generator<base_generator_type&, distribution_type> gen_type;
	boost::random_device dev;

	base_generator_type generator( dev() );

	gen_type die_gen(generator, distribution_type(97,122));

	boost::generator_iterator<gen_type> die(&die_gen);
	for(int i = 0; i < 25; i++)
		this->ticket += static_cast<char>( *die++ );

	return this->ticket;
}





bool
User::save() const {
	return sqlite::db()->save<User>(*this);
}
