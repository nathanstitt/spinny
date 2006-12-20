/* @(#)config.h.cmake
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_ 1

#include <algorithm>
using namespace std;

#define GRANT_NEEDED_FRIENDSHIP(classnm) \
	friend class sqlite::connection; \
	friend class sqlite::command; \
        friend class sqlite::reader; \
	friend class sqlite::command::iterator<Spinny::classnm>; \
	friend struct sqlite::detail::best_type<Spinny::classnm,true>


#cmakedefine SRC_PATH "@CMAKE_CURRENT_SOURCE_DIR@/../../"
#cmakedefine TESTING_FIXTURES_PATH "@CMAKE_CURRENT_BINARY_DIR@/../../test/fixtures"
#cmakedefine WINDOWS
#cmakedefine UNIX_VARIANT


#endif /* _CONFIG_H_ */

