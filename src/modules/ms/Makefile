SRCS = ms.c ms_help.c
TARGET = moraleserv.so
DISTFILES = $(SRCS) ms.h Makefile ChangeLog

include ../Makefile.inc

install: $(TARGET)
	$(INSTALL_MOD) $(TARGET) $(MODDIRECTORY)
