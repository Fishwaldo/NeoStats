# Makefile for GeoStats
# GeoStats CVS Identification
# $Id: Makefile,v 1.4 2000/02/22 03:32:32 fishwaldo Exp $
# makefile originally created by Andy Church.
 
CC=gcc
CFLAGS = -O2 -Wall -ggdb
#CFLAGS	= -O3 -Wall
LDFLAGS= -rdynamic -ldl

OBJS =	dotconf.o services.o main.o sock.o conf.o ircd.o timer.o users.o \
		ns_help.o dl.o 
SRCS =	dotconf.c services.c main.c sock.c conf.c ircd.c timer.c users.c \
		ns_help.c dl.c

.c.o:
	$(CC) $(CFLAGS) -c $<


all: 	stats
	(cd dl; $(MAKE) $@)

clean:
	/bin/rm -rf *.o stats
	(cd dl; $(MAKE) $@)


stats: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS)  -o $@

# Catch any changes in compilation options at the top of this file
$(OBJS): Makefile

main.o:		main.c		stats.h
sock.o:		sock.c		stats.h
conf.o:		conf.c		stats.h
ircd.o:		ircd.c		stats.h
timer.o:	timer.c		stats.h
statserv.o:	statserv.c	stats.h
users.o:	users.c		stats.h
stats.o:	stats.c		stats.h
help.o:		help.c		stats.h
