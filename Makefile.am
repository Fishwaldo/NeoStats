AUTOMAKE_OPTIONS 	= foreign
SUBDIRS 		= include lib src modules
ACLOCAL_AMFLAGS 	= -I autotools
AM_MAKEFLAGS		= -s
bin_SCRIPTS		= makeconf cronchk
data_DATA		= neostats.motd makeconf

docdir			= @prefix@/doc
doc_DATA		= doc/* doc/old/*

all-local:
	@touch include/version.sh

include $(top_srcdir)/autotools/rules.mk