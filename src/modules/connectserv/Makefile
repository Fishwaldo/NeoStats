SRCS = cs.c cs_help.c
TARGET = connectserv.so
DISTFILES = $(SRCS) cs.h Makefile

include ../Makefile.inc

install: $(TARGET)
	$(INSTALL_MOD) $(TARGET) $(MODDIRECTORY)
