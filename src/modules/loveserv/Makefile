SRCS = loveserv.c ls_help.c 
TARGET = loveserv.so
DISTFILES = $(SRCS) loveserv.h Makefile

include ../Makefile.inc

install: $(TARGET)
	$(INSTALL_MOD) $(TARGET) $(MODDIRECTORY)
