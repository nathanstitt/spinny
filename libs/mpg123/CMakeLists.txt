IF(NOT WIN32)
  SET(CMAKE_C_FLAGS " -g")
  SET(CMAKE_CXX_FLAGS " -g")
ENDIF(NOT WIN32)


INCLUDE (CheckIncludeFiles)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckTypeSize)
INCLUDE (CheckLibraryExists)

CHECK_FUNCTION_EXISTS( getpagesize HAVE_GETPAGESIZE )
CHECK_INCLUDE_FILES( inttypes.h HAVE_INTTYPES_H )
CHECK_LIBRARY_EXISTS( m cos "" HAVE_LIBM )
CHECK_LIBRARY_EXISTS( mx powf "" HAVE_LIBMX )
CHECK_INCLUDE_FILES( limits.h HAVE_LIMITS_H )
CHECK_INCLUDE_FILES( linux/soundcard.h HAVE_LINUX_SOUNDCARD_H )
CHECK_INCLUDE_FILES( machine/soundcard.h HAVE_MACHINE_SOUNDCARD_H )
CHECK_INCLUDE_FILES( memory.h HAVE_MEMORY_H )
CHECK_FUNCTION_EXISTS( mmap HAVE_MMAP )
CHECK_INCLUDE_FILES( sched.h HAVE_SCHED_H )
CHECK_FUNCTION_EXISTS( sched_setscheduler HAVE_SCHED_SETSCHEDULER )
CHECK_FUNCTION_EXISTS( setpriority HAVE_SETPRIORITY )
CHECK_INCLUDE_FILES( stdint.h HAVE_STDINT_H )
CHECK_INCLUDE_FILES( stdlib.h HAVE_STDLIB_H )
CHECK_INCLUDE_FILES( strings.h HAVE_STRINGS_H )
CHECK_INCLUDE_FILES( string.h HAVE_STRING_H )
CHECK_INCLUDE_FILES( sun/audioio.h HAVE_SUN_AUDIOIO_H )
CHECK_INCLUDE_FILES( sys/audio.h HAVE_SYS_AUDIO_H )
CHECK_INCLUDE_FILES( sys/ioctl.h HAVE_SYS_IOCTL_H )
CHECK_INCLUDE_FILES( sys/soundcard.h HAVE_SYS_SOUNDCARD_H )
CHECK_INCLUDE_FILES( sys/stat.h HAVE_SYS_STAT_H )
CHECK_INCLUDE_FILES( sys/types.h HAVE_SYS_TYPES_H )
CHECK_INCLUDE_FILES( termios.h HAVE_TERMIOS )
CHECK_INCLUDE_FILES( unistd.h HAVE_UNISTD_H )


SET(CMAKE_C_FLAGS "-DHAVE_CONFIG_H ${CMAKE_C_FLAGS}")

# SET(CMAKE_EXTRA_INCLUDE_FILES types.h)
#CHECK_TYPE_SIZE( 
#SET(CMAKE_EXTRA_INCLUDE_FILES)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

#  ${CMAKE_CURRENT_SOURCE_DIR}/mpglib 
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

SET ( C_SRC audio.c 
	audio.h 
	audio_dummy.c
	buffer.c 
	buffer.h 
	common.c 
	common.h 
	control_generic.c 
	debug.h 
	decode_2to1.c 
	decode_4to1.c 
	decode_ntom.c 
	equalizer.c 
	getbits.c 
	getbits.h 
	getlopt.c 
	getlopt.h 
	httpget.c 
	huffman.h 
	icy.c 
	icy.h 
	id3.c 
	id3.h 
	genre.h 
	l2tables.h 
	layer1.c 
	layer2.c 
	layer3.c 
	layer3.h 
	mpg123.c 
	mpg123.h 
	playlist.c 
	playlist.h 
	readers.c 
	tabinit.c 
	sfifo.c 
	sfifo.h 
	stringbuf.c 
	stringbuf.h 
	term.c 
	term.h 
	wav.c 
	xfermem.c 
	xfermem.h )

SET ( C_SRC ${C_SRC}
  decode_i386.c 
  dct64_i386.c
  decode_i486.c
  dct64_i486.c )

ADD_LIBRARY( mpg123 ${C_SRC} )
