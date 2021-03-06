#include <iostream>
#include "spinny/app.hpp"
#include "sqlite/sqlite.hpp"
#include "boost/log/functions.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/xtime.hpp"
#include <boost/thread/tss.hpp>
#include "handlers/shared.hpp"
#include <cassert>


class thread_adapter {
public:
	thread_adapter( int argv, char **argc )
		: argv_(argv), argc_(argc )
		{ }
	void operator()() const {
		Spinny::App::run( argv_, argc_ );
	}

private:
	int argv_;
	char **argc_;
};


#if !defined(_WIN32)

#include <pthread.h>
#include <signal.h>

#include "handlers/pl.hpp"

#endif

int main(int argv , char** argc ) {
	handlers::link_up();
	
// 	handlers::PlayList p;
	

	boost::logging::manipulate_logs("www")
		.del_modifier("time")
		.enable( boost::logging::level::info )
		.del_modifier("prefix")
		.del_modifier("enter")
		.add_appender(&boost::logging::write_to_cout)    // all messages are written to cout
		.add_modifier(&boost::logging::prepend_prefix,"prefix" )
		.add_modifier( boost::logging::prepend_time("$yy$MM$dd $hh:$mm:$ss "), "time" )
		.add_modifier(&boost::logging::append_enter,"enter");

	boost::logging::manipulate_logs("sql")
		.del_modifier("time")
		.enable( boost::logging::level::warn )
		.del_modifier("prefix")
		.del_modifier("enter")
		.add_appender(&boost::logging::write_to_cout)    // all messages are written to cout
		.add_modifier(&boost::logging::prepend_prefix,"prefix" )
		.add_modifier( boost::logging::prepend_time("$yy$MM$dd $hh:$mm:$ss "), "time" )
		.add_modifier(&boost::logging::append_enter,"enter");

	boost::logging::flush_log_cache();

	// Block all signals for background thread.
	sigset_t new_mask;
	sigfillset(&new_mask);
	sigset_t old_mask;
	pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);

	boost::thread thread(thread_adapter(argv,argc));

	// Restore previous signals.

	pthread_sigmask(SIG_SETMASK, &old_mask, 0);


	// Wait for signal indicating time to shut down.
	sigset_t wait_mask;
	sigemptyset(&wait_mask);
	sigaddset(&wait_mask, SIGINT);
	sigaddset(&wait_mask, SIGQUIT);
	sigaddset(&wait_mask, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
	int sig = 0;
	sigwait(&wait_mask, &sig);


	Spinny::App::stop();
	
	thread.join();
}
