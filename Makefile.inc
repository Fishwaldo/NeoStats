CC=gcc
CFLAGS= -O2 -Wall
LDFLAGS= -rdynamic -ldl 
INSTALL = /usr/bin/install -c                                                                                                                 
INSTALL_PROGRAM = ${INSTALL}                                                                                                 
INSTALL_DATA = ${INSTALL} -m 644
MODULES=statserv spam
DIRECTORY=/home/fish/NeoStats