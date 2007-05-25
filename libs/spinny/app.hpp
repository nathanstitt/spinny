/* @(#)app.hpp
 */

#ifndef _SPINNY_APP_H
#define _SPINNY_APP_H 1


#include "sqlite/sqlite.hpp"
#include "spinny/config.h"
#include "boost/program_options.hpp"
#include "boost/log/log.hpp"
#include "boost/asio.hpp"
#include "streaming/server.hpp"


BOOST_DECLARE_LOG(app)

namespace ews{
	class server;
}

namespace Spinny {

class App {
	App( int argc, char **argv );
	int _argc;
	char **_argv;
	boost::program_options::variables_map _vm;
	void startup_threads();
	void stop_threads();
	~App();

	void
	rescan();
public:
	class CmdLineEx {};

	static int run( int argc, char **argv );

	static void stop();

	static App* instance();

	template<typename T>
 	const T& config( const std::string &name ) const {
		return _vm[name].as<T>(); 
	}

	Streaming::Server *streaming;

	::ews::server *www;

// 	static
// 	bool
// 	add_streaming_client( Spinny::PlayList::ptr, boost::shared_ptr<asio::ip::tcp::socket> );

	
};

}

#endif /* _SPINNY_APP_H */


