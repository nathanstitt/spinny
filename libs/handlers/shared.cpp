

/* @(#)shared.cpp
 */


#include "handlers/shared.hpp"
#include "handlers/index.hpp"
#include "handlers/tree.hpp"
#include "handlers/pl.hpp"
#include "handlers/file_system.hpp"

void
handlers::link_up(){

	static handlers::PL pl_;
	static handlers::Index index_;
	static handlers::Tree tree_;
	static handlers::Files files_;

	BOOST_LOGL(www,info) << "LINKING UP HANDLERS";
};

