
#include "sqlite.hpp"

namespace sqlite {

	database_error::database_error(const char *msg) : runtime_error(msg) {}
	database_error::database_error(connection &con) : runtime_error(sqlite3_errmsg(con.db)) {}

} // sqlite namespace
