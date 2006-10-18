#include <iostream>
#include <string>
#include "boost/asio.hpp"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "spinny/spinny.hpp"
#include "handlers/instantiate.hpp"

#if defined(_WIN32)

boost::function0<void> console_ctrl_function;

BOOL WINAPI console_ctrl_handler(DWORD ctrl_type)
{
	switch (ctrl_type)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		Spinny::stop();
		return TRUE;
	default:
		return FALSE;
	}
}

int main(int argc, char* argv[])
{
	handlers::link_up();


	try
	{
		SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
		Spinny::run(argc,argv);
	}
	catch (asio::error& e)
	{
		std::cerr << "asio error: " << e << "\n";
	}
	catch (std::exception& e)
	{
		std::cerr << "exception: " << e.what() << "\n";
	}

	return 0;
}

#endif // defined(_WIN32)
