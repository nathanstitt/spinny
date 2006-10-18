#include "ews/server.hpp"
#include "ews/handlers/index.hpp"
#include "spinny/music_dir.hpp"
#include "boost/lambda/lambda.hpp"
#include "boost/lambda/bind.hpp"
#include <algorithm>
using namespace ews::handlers;




ews::request_handler::result
Index::handle( const ews::request& req, ews::reply& rep ){
	BOOST_LOGL( ewslog, info ) << "Index H: " << req.url;

	if ( req.url != "/" ){
		return cont;
	}

	rep.set_template( "index.html" );

	MusicDir::result_set rs = MusicDir::roots();
//      using namespace boost::lambda;
	rep.content << "MusicDirRoots {\n";
	using namespace boost::lambda;
 	std::for_each( rs.begin(), rs.end(),
		       rep.content << bind(&MusicDir::filesystem_name, _1 )
 		);
	rep.content << "}\n";


	
	rep.add_header( "X-HANDLED-BY", "IndexHandler" );
	rep.set_basic_headers( "html" );
	
	return stop;
}
std::string
Index::name() const { return "CustomHandler"; }


