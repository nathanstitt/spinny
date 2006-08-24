/* @(#)config.h.cmake
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_ 1

using namespace std;

#define GRANT_NEEDED_FRIENDSHIP(classnm) \
	friend class DummyApp; \
	friend class sqlite::connection; \
	friend class sqlite::command; \
        friend class sqlite::reader; \
	friend class sqlite::command::iterator<classnm>


#cmakedefine TESTING_FIXTURES_PATH "@TESTING_FIXTURES_PATH@"
#cmakedefine WINDOWS
#cmakedefine UNIX_VARIANT


#endif /* _CONFIG_H_ */

