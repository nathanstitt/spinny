#include "testing.hpp"
#include "spinny/user.hpp"


SUITE(SpinnyUser) {


TEST( Create ){
	DummyApp da;
	User::ptr u = User::create( "test-login", "test" );

	CHECK( ! u->ticket.empty() );
}

TEST( Roles ){
	DummyApp da;
	User::ptr u = User::create( "test-login", "test" );

	CHECK( u->is_guest() );
	u->set_role( User::TrustedRole );
	CHECK( u->is_trusted() );

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

