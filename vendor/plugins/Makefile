# Makefile for Solaris/Linux builds

CC=gcc
CFLAGS=-g -O2 -fexceptions
LIBS=-ldl -lstdc++

SOURCE=PITest.cpp ClassFactory.cpp DynamicLibrary.cpp SimpleMath.cpp
OBJS=$(SOURCE:.c=.o)
TARGETS=PITest COMLikeMath.so TimeTest.so GoodMath.so BadMath.so

all:	$(TARGETS)

PITest:	$(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

COMLikeMath.so:	COMLikeMath.cpp
	$(CC) -o $@ -fpic -shared $?

TimeTest.so:	TimeTest.cpp
	$(CC) -o $@ -fpic -shared $?

GoodMath.so:	GoodMath.cpp
	$(CC) -o $@ -fpic -shared $?

BadMath.so:		BadMath.cpp
	$(CC) -o $@ -fpic -shared $?

clean:
	rm -f *.o
	rm -f $(TARGETS)
