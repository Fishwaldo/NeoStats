include ../../Makefile.conf

INCLUDES = -I../..

ifdef PTHREADIDENT
# okie, pthreads is used there, we HAVE to use pthreads here, otherwise nothing works.
CFLAGS := -DUSEPTHREADS
endif


SOURCES= services.c
OBJECTS= services.lo

TARGET= services.so

all:
	gcc $(CFLAGS) $(INCLUDES) services.lo -c services.c
	gcc -o services.so $(LIBS) services.lo -shared


