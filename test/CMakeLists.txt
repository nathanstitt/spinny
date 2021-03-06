IF(CMAKE_COMPILER_IS_GNUCXX)
#  SET(CMAKE_CXX_FLAGS "-Wall -Wextra -g")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)


ENABLE_TESTING()

FILE(GLOB PATHED_TESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
SET(${TESTS_SRC} "")
FOREACH( FILE ${PATHED_TESTS_SRC} )
  GET_FILENAME_COMPONENT( NAME ${FILE} NAME )
  SET( TESTS_SRC ${TESTS_SRC} ${NAME} )
ENDFOREACH(FILE)

SET(KIT All)

CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx ${TESTS_SRC} )

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})

TARGET_LINK_LIBRARIES(${KIT}CxxTests handlers libspinny taglib zlib sqlite unit_test ews cs json boost )

IF( HAVE_PTHREAD_H )
  TARGET_LINK_LIBRARIES( ${KIT}CxxTests pthread )
ENDIF( HAVE_PTHREAD_H )
SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

FOREACH (test ${TestsToRun})
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  ADD_TEST(${TName} ${CXX_TEST_PATH}/${KIT}CxxTests ${TName})
ENDFOREACH (test)

INCLUDE_DIRECTORIES( 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../libs/
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/
  ${CMAKE_CURRENT_BINARY_DIR}/../src/
  ${CMAKE_CURRENT_BINARY_DIR}/../libs/
)


IF(CMAKE_COMPILER_IS_GNUCXX)
#  SET(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
