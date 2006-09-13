/* @(#)config.h.cmake
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_ 1

#include <algorithm>
using namespace std;

#define GRANT_NEEDED_FRIENDSHIP(classnm) \
	friend class DummyApp; \
	friend class sqlite::connection; \
	friend class sqlite::command; \
        friend class sqlite::reader; \
	friend class sqlite::command::iterator<classnm>; \
	friend struct sqlite::detail::best_type<classnm,true>


#cmakedefine TESTING_FIXTURES_PATH "@CMAKE_CURRENT_BINARY_DIR@/../test/fixtures"
#cmakedefine WINDOWS
#cmakedefine UNIX_VARIANT


#endif /* _CONFIG_H_ */

