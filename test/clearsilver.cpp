#include "testing.hpp"

#include "cs/cs.h"

#include "boost/filesystem/fstream.hpp"

SUITE(ClearSilver) {

static NEOERR *output (void *ctx, char *s)
{
  printf ("%s", s);
  return STATUS_OK;
}


TEST( Start ){
	CSPARSE *p=0;
	HDF *hdf=0;
	NEOERR* err;

	err = hdf_init ( &hdf );
	err = hdf_set_value ( hdf, "Foo", "bar" );

	err = cs_init ( &p, hdf );

	char *s=new char[1000];

	strcpy(s,"Blas <?cs var:Foo ?>");
	err=cs_parse_string ( p, s, strlen(s) );


	err = cs_render(p, NULL, output);

//	cout << s << endl;

	
}


} // SUITE(EwsRun)

int
clearsilver( int argc, char * argv[] ) 
{
	return UnitTest::RunAllTests("ClearSilver");

}
