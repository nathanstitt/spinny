
#include "testing.hpp"

#include "boost/filesystem/fstream.hpp"
#include <boost/algorithm/string.hpp>

#include <json/json.h>
#include <algorithm> // sort
#include <stdio.h>
#include "boost/algorithm/string/erase.hpp"

#if defined(_MSC_VER)  &&  _MSC_VER >= 1310
# pragma warning( disable: 4996 )     // disable fopen deprecation warning
#endif


SUITE(JSON){



static void
getValueTree( std::string &results, Json::Value &value, const std::string &path = "." )
{
	char fout[1000];
	switch ( value.type() ) {
	case Json::nullValue:
		sprintf( fout, "%s=null\n", path.c_str() );
		results+=fout;
		break;
	case Json::intValue:
		sprintf( fout, "%s=%d\n", path.c_str(), value.asInt() );
		results+=fout;
		break;
	case Json::uintValue:
		sprintf( fout, "%s=%u\n", path.c_str(), value.asUInt() );
		results+=fout;
		break;
	case Json::realValue:
		sprintf( fout, "%s=%.16g\n", path.c_str(), value.asDouble() );
		results+=fout;
		break;
	case Json::stringValue:
		sprintf( fout, "%s=\"%s\"\n", path.c_str(), value.asString().c_str() );
		results+=fout;
		break;
	case Json::booleanValue:
		sprintf( fout, "%s=%s\n", path.c_str(), value.asBool() ? "true" : "false" );
		results+=fout;
		break;
	case Json::arrayValue: {
		sprintf( fout, "%s=[]\n", path.c_str() );
		results+=fout;
		int size = value.size();
		for ( int index =0; index < size; ++index ) {
			static char buffer[16];
			sprintf( buffer, "[%d]", index );
			getValueTree( results, value[index], path + buffer );
		}
	}
		break;
	case Json::objectValue: {
		sprintf( fout, "%s={}\n", path.c_str() );
		results+=fout;
		Json::Value::Members members( value.getMemberNames() );
		std::sort( members.begin(), members.end() );
		std::string suffix = *(path.end()-1) == '.' ? "" : ".";
		for ( Json::Value::Members::iterator it = members.begin(); 
		      it != members.end(); 
		      ++it )
		{
			const std::string &name = *it;
			getValueTree( results, value[name], path + suffix + name );
		}
	}
		break;
	default:
		break;
	}
}




static int
rewriteValueTree( const std::string &rewritePath, 
                  const Json::Value &root, 
                  std::string &rewrite )
{
//   Json::FastWriter writer;
   Json::StyledWriter writer;
   rewrite = writer.write( root );
   FILE *fout = fopen( rewritePath.c_str(), "wt" );
   if ( !fout )
   {
      printf( "Failed to create rewrite file: %s\n", rewritePath.c_str() );
      return 2;
   }
   fprintf( fout, "%s\n", rewrite.c_str() );
   fclose( fout );
   return 0;
}



TEST( All ){
	DummyApp da;
	da.populate_test_files("json");
	
	boost::filesystem::path input_path(  da.fixtures_path / "json" );
	boost::filesystem::directory_iterator file( input_path );
	filesystem::directory_iterator end_itr;
	for ( file ;file != end_itr; ++file ){
		
		if ( ! boost::ends_with( file->leaf(), ".json") ){
			continue;
		}
		std::string input_file( file->leaf() );
		std::string base_name( input_file, 0, input_file.size()-5 );
// 		if ( base_name != "test_real_05" ){
// 			continue;
// 		}
//		cout << base_name<< endl;
		std::string input;
 		char buf[512];
 		boost::filesystem::ifstream is( input_path / file->leaf() );
 		while (is.read(buf, sizeof(buf)).gcount() > 0){
 			input.append( buf, is.gcount() );
 		}
		
		Json::Reader reader;
		Json::Value root;
		bool ok = reader.parse( input, root );
		if ( ! ok ){
			cerr << reader.getFormatedErrorMessages() << endl;
		}
		CHECK( ok );
		std::string results( base_name+" : " );

		getValueTree( results,root );


		std::string expected( base_name+" : " );
 		boost::filesystem::ifstream ex( input_path / (base_name + ".expected") );
 		while (ex.read(buf, sizeof(buf)).gcount() > 0){
 			expected.append( buf, ex.gcount() );
 		}
		boost::algorithm::erase_all(expected, "\r");
		boost::algorithm::erase_all(expected, "\n");
		boost::algorithm::erase_all(results, "\r");
		boost::algorithm::erase_all(results, "\n");

		if ( expected != results ){
			for ( int x=0; x<expected.size(); x++ ){
				if ( expected[x] != results[x] ){
					cout << "Char " << x << " differs: " <<  (int)expected[x] << "!=" << (int)results[x] << endl;
				}
			}
		}
		CHECK_EQUAL( expected,results );
	}


}


} // Suite(JSON)

int json( int argc, char * argv[] )
{
	return UnitTest::RunAllTests("JSON");

 //   std::string input = readInputTestFile( argv[1] );
//    if ( input.empty() )
//    {
//       printf( "Failed to read input or empty input: %s\n", argv[1] );
//       return 3;
//    }

//    std::string basePath = removeSuffix( argv[1], ".json" );
//    if ( basePath.empty() )
//    {
//       printf( "Bad input path. Path does not end with '.expected':\n%s\n", argv[1] );
//       return 3;
//    }

//    std::string actualPath = basePath + ".actual";
//    std::string rewritePath = basePath + ".rewrite";
//    std::string rewriteActualPath = basePath + ".actual-rewrite";

//    Json::Value root;
//    int exitCode = parseAndSaveValueTree( input, actualPath, "input", root );
//    if ( exitCode == 0 )
//    {
//       std::string rewrite;
//       exitCode = rewriteValueTree( rewritePath, root, rewrite );
//       if ( exitCode == 0 )
//       {
//          Json::Value rewriteRoot;
//          exitCode = parseAndSaveValueTree( rewrite, rewriteActualPath, "rewrite", rewriteRoot );
//       }
//    }

//    return exitCode;
}

