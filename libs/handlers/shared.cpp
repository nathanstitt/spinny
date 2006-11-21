

/* @(#)shared.cpp
 */


#include "handlers/shared.hpp"
#include "handlers/index.hpp"
#include "handlers/tree.hpp"
#include "handlers/pl.hpp"


void
handlers::link_up(){

	static handlers::PL pl_;
	static handlers::Index index_;
	static handlers::Tree tree_;

	BOOST_LOGL(www,info) << "LINKING UP HANDLERS";
};

