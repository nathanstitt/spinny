#include "testing.hpp"
#include "spinny/user.hpp"

using namespace Spinny;

SUITE(SpinnyUser) {


TEST( Create ){
	DummyApp da;
	User::ptr u = User::create( "test-login", "test" );

	CHECK( ! u->ticket.empty() );
}

TEST( IdLoad ){
	DummyApp da;
	EnableLogging el;
	User::ptr u = User::create( "test-login", "test" );
	u->save();

	sqlite::id_t uid = sqlite::db()->exec<sqlite::id_t>( "select rowid from users order by rowid desc limit 1" );
	CHECK_EQUAL( u->db_id(), uid );
}

TEST( Roles ){
	DummyApp da;
	User::ptr u = User::create( "test-login", "test" );
	u->save();

	sqlite::id_t uid = sqlite::db()->exec<sqlite::id_t>( "select rowid from users" );
	CHECK_EQUAL( u->db_id(), uid );

	CHECK( u->has_at_least( User::ReadOnlyRole ) );
	CHECK( ! u->has_modify_role() );
	CHECK( ! u->has_at_least( User::ModifyRole ) );


	u->set_role( User::ModifyRole );
	CHECK( u->has_modify_role() );
	CHECK( u->has_at_least( User::ModifyRole ) );

	u->set_role( User::ReadOnlyRole );
	CHECK( ! u->has_modify_role() );


	u->set_role( User::AdminRole );
	CHECK( u->has_modify_role() );
	CHECK( u->is_admin() );
	CHECK( u->has_at_least( User::ReadOnlyRole ) );
}

TEST( Authen ){
	DummyApp da;
	User::ptr u = User::create( "test-login", "test" );
	std::string ticket = u->ticket;
	CHECK( ! u->authen( "bad-password" ) );
	CHECK_EQUAL( u->ticket, ticket );
	CHECK( u->authen( "test" ) );
	CHECK( u->ticket != ticket );
}


TEST(Ticket){
	DummyApp da;
	User::ptr u = User::create( "test-login", "test" );
	std::string ticket = u->ticket;
	u->generate_new_ticket();
	CHECK( u->ticket != ticket );
}

TEST( FromTicket ){
	DummyApp da;
	User::ptr u1 = User::create( "test-login", "test" );
	u1->save();
	
	User::ptr u2 = User::with_ticket( u1->ticket );

	CHECK_EQUAL( u1->login, u2->login );
	CHECK_EQUAL( u1->ticket, u2->ticket );
	CHECK_EQUAL( u1->last_visit, u2->last_visit );
}

}



int
spinny_user( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("SpinnyUser");
}

