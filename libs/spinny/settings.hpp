/* @(#)settings.hpp
 */

#ifndef _SPINNY_SETTINGS_H
#define _SPINNY_SETTINGS_H 1

#include <string>
#include "sqlite/sqlite.hpp"

namespace Spinny {

namespace Settings {

	void link_up();

	template<typename T> inline
	T
	set( const std::string &name, T value ){
		sqlite::connection *con=sqlite::db();
		*con << "update settings set value = " << sqlite::q(value) << " where name = " << sqlite::q(name);
		con->exec<sqlite::none>();
		return value;
	}


	template<typename T> inline
	T
	get( const std::string &name ){
		sqlite::connection *con=sqlite::db();
		*con << "select value from settings where name = " << sqlite::q(name);
		return con->exec<T>();
	}


};


} // Spinny

#endif /* _SPINNY_SETTINGS_H */

