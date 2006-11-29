#include "testing.hpp"
#include "spinny/user.hpp"


SUITE(SpinnyUser) {


TEST( Create ){
	DummyApp da;
	User::ptr u = User::create( "test-login", "test" );

	cout << u->ticket << endl;
}


}



int
spinny_user( int argc, char * argv[] )  
{
	return UnitTest::RunAllTests("SpinnyUser");
}

