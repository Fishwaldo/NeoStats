SRCS = hostserv.c hs_help.c
TARGET = hostserv.so
DISTFILES = $(SRCS) hostserv.h Makefile

include ../Makefile.inc

install: $(TARGET)
	$(INSTALL_MOD) $(TARGET) $(MODDIRECTORY)
