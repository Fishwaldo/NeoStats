# Makefile for GeoStats
# GeoStats CVS Identification
# $Id: Makefile,v 1.2 2000/02/18 00:42:24 fishwaldo Exp $
# makefile originally created by Andy Church.
 
CC=gcc
CFLAGS = -O2 -Wall -ggdb
#CFLAGS	= -O3 -Wall
LDFLAGS= -rdynamic -ldl -lefence

OBJS =	dotconf.o services.o main.o sock.o conf.o ircd.o timer.o users.o \
		ns_help.o dl.o 
SRCS =	dotconf.c services.c main.c sock.c conf.c ircd.c timer.c users.c \
		ns_help.c dl.c

.c.o:
	$(CC) $(CFLAGS) -c $<


all: stats

clean:
	/bin/rm -rf *.o stats
########


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
