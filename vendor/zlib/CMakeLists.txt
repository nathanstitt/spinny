
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )

SET(ZLIB_SRCS
  adler32.c   gzio.c      inftrees.c  uncompr.c
  compress.c  infblock.c  infutil.c   zutil.c
  crc32.c     infcodes.c 
  deflate.c   inffast.c  
  inflate.c   trees.c
  )


ADD_LIBRARY( zlib ${ZLIB_SRCS})

