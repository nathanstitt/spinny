include(CheckIncludeFiles)

PROJECT( Spinny )

IF(WIN32)
  SET( WINDOWS 1 )
ELSE(WIN32)
  SET( UNIX_VARIANT 1 )
ENDIF(WIN32)

SET( TESTING_FIXTURES_PATH 1 )
SET( SRC_PATH 1 )


INCLUDE(Dart)
MARK_AS_ADVANCED(BUILD_TESTING DART_ROOT TCL_TCLSH)
ENABLE_TESTING()
SET(BUILDNAME "OpenJPEG-${CMAKE_SYSTEM}-${CMAKE_C_COMPILER}" CACHE STRING "Name of build on the dashboard")
MARK_AS_ADVANCED(BUILDNAME)

CHECK_INCLUDE_FILES(pthread.h   HAVE_PTHREAD_H)

SUBDIRS( libs src test )