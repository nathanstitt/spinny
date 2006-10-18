#include "ews/server.hpp"
#include "handlers/index.hpp"
#include "spinny/music_dir.hpp"
#include "boost/lambda/lambda.hpp"
#include "boost/lambda/bind.hpp"
#include <algorithm>


using namespace handlers;


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
	for ( MusicDir::result_set::iterator md = rs.begin(); rs.end() != md; ++md ){
		rep.content << "    " << md->db_id() << "=" << md->path().leaf() << "\n";
	}
	rep.content << "}\n";

	rep.add_header( "X-HANDLED-BY", "IndexHandler" );
	rep.set_basic_headers( "html" );
	
	return stop;
}
std::string
Index::name() const { return "Index"; }


