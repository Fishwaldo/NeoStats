noinst_LIBRARIES	= libevent.a
libevent_a_SOURCES 	= buffer.c devpoll.c epoll.c epoll_sub.c evbuffer.c event.c kqueue.c poll.c rtsig.c select.c signal.c
include_HEADERS 	= event.h
INCLUDES 		= -Icompat

EXTRA_DIST = err.c event.h event-internal.h evsignal.h \
	kqueue.c epoll_sub.c epoll.c select.c rtsig.c poll.c signal.c \
	devpoll.c \
	compat/err.h compat/sys/queue.h compat/sys/tree.h compat/sys/_time.h \
	WIN32-Code WIN32-Code/config.h WIN32-Code/misc.c \
	WIN32-Code/win32.c WIN32-Code/misc.h \
	WIN32-Prj WIN32-Prj/event_test WIN32-Prj/event_test/event_test.dsp \
	WIN32-Prj/event_test/test.txt WIN32-Prj/libevent.dsp \
	WIN32-Prj/libevent.dsw WIN32-Prj/signal_test \
	WIN32-Prj/signal_test/signal_test.dsp WIN32-Prj/time_test \
	WIN32-Prj/time_test/time_test.dsp



include $(top_srcdir)/autotools/rules.mk


select.o:	select.c
devpoll.o:	devpoll.c
epoll.o:	epoll.c
epoll_sub.o:	epoll_sub.c
err.o:		err.c
kqueue.o:	kqueue.c
poll.o:		poll.c
rtsig.o:	rtsig.c
signal.o:	signal.c